# lvgl_frames_capture

Record what an LVGL demo shows on the Apollo510 EVB display to the eMMC, then
copy it to the PC and turn it into an MP4.

```
tools/lvgl_frames_capture/
├── lv_frames_capture.h      API
├── lv_frames_capture.c      grab + writer tasks
└── scripts/
    ├── pull.sh              eMMC -> PC over USB, then encode.sh
    └── encode.sh            frames.raw + meta.txt -> MP4 (ffmpeg)
```

## How it works

```
GUI task renders ──> display_buffer (SSRAM, 464x464 RGB565)
                          │  grab task, every 1/fps s, under display_buffer_lock
                          v
                     PSRAM ring (8 slots, ~3.4 MB)
                          │  writer task, below the GUI task
                          v
                     eMMC  /lvcap/NNNN/frames.raw  +  meta.txt
```

- The grab task copies the port's full-screen `display_buffer` while holding
  `display_buffer_lock`, the same lock the flush and the panel transfer use,
  so a recorded frame is never torn.
- Frames are grabbed on a fixed clock (30 fps by default), so the video plays
  back in real time whether LVGL redraws or not.
- If the writer falls behind and no slot is free, that frame is counted in
  `dropped` instead of delaying the next one.
- Each recording gets the next free `/lvcap/NNNN/` folder: `frames.raw` holds
  headerless RGB565 frames, and `meta.txt` holds size, pixel format, fps and
  the counters printed at the end.

## 1. Build it in

The tool is not part of a normal build. Add it with `LV_FRAMES_CAPTURE=1`;
switching it on or off needs a clean build, as make does not track defines:

```sh
cd lvgl_ambiq_demo/lvgl_timeline/apollo510_evb
make clean && make LV_FRAMES_CAPTURE=1
```

## 2. Start it from the demo

In the demo's `gui_task.c`, after `lv_ambiq_fs_init()` and
`lv_ambiq_display_init()`:

```c
#if LV_FRAMES_CAPTURE
#include "lv_frames_capture.h"
#endif

    ...
    lv_ambiq_display_init();

#if LV_FRAMES_CAPTURE
    lv_frames_capture_cfg_t capture_cfg;
    lv_frames_capture_cfg_init(&capture_cfg);   // "/lvcap", 30 fps, 8 slots
    capture_cfg.seconds = 10;                   // 0 = until lv_frames_capture_stop()
    lv_frames_capture_start(&capture_cfg);
#endif
```

| field | default | |
|---|---|---|
| `root` | `"/lvcap"` | eMMC folder for the sessions |
| `fps` | `30` | grab rate |
| `seconds` | `0` | stop on its own after this long; 0 = until `lv_frames_capture_stop()` |
| `slots` | `8` | PSRAM ring size, one frame each |

`lv_frames_capture_stop()` ends a recording early; the writer then drains the
ring, closes the file and writes `meta.txt`. `lv_frames_capture_get_stats()`
returns the counters, with `running` false once everything is on the eMMC.

Flash and run the demo. The SWO log shows the start and the end:

```
lvcap: recording 464x464 @ 30 fps to /lvcap/0001/frames.raw (10 s, 0 = until stop)
lvcap: /lvcap/0001 done, 300 frames 464x464 @ 30 fps, 0 dropped, queue max 2/8, write avg 19 ms max 60 ms, grab max 19 ms
```

## 3. Pull and encode

Connect the EVB's USB-C port (not the J-Link one) to the PC, then from WSL:

```sh
tools/lvgl_frames_capture/scripts/pull.sh lvgl_ambiq_demo/lvgl_timeline/apollo510_evb -rm
```

This flashes `tools/LoadTexture`, which makes the eMMC a USB drive, copies the
newest session to `<board dir>/lvcap/NNNN/` (ignored by git) and encodes
`capture.mp4` there. The board is left running LoadTexture; flash the demo
again afterwards.

| option | |
|---|---|
| `-session NNNN` | a specific session instead of the newest |
| `-o <dir>` | copy to `<dir>/NNNN` instead of `<board dir>/lvcap/NNNN` |
| `-rm` | delete the session from the eMMC after copying it |
| `-src <dir>` | the eMMC is already mounted at `<dir>`; skips LoadTexture |
| `-noencode` | copy only |

Flashing uses `$LVCAP_JLINK` when set, as `$LVCAP_JLINK run <dir>` (a J-Link
wrapper that flashes the `.bin` in `<dir>` and runs it, e.g. one that goes
through a J-Link Remote Server), otherwise `JLinkExe` with LoadTexture's own
`flash.jlink`. Finding the drive goes through Windows (`powershell.exe`), so
outside WSL mount the eMMC yourself and pass `-src`.

A session copied by other means is encoded on its own with

```sh
tools/lvgl_frames_capture/scripts/encode.sh <session dir> [-o out.mp4] [-crf 18]
```

which checks that `frames.raw` holds exactly `frames` frames of the size in
`meta.txt` and warns if any were dropped.

Session numbers come from what is on the eMMC, so after `-rm` the next
recording reuses the same number; pulling it into the same folder replaces the
earlier copy.

## Measured on the Apollo510 EVB

464x464 RGB565, 430592 B per frame; 30 fps needs 12.9 MB/s, 33.3 ms per frame.

| | |
|---|---|
| `f_write` of one frame, one sequential file | 17.6 ms avg, worst 32-60 ms (~24 MB/s) |
| one file per frame instead | 23.9 ms avg, worst 38.6 ms |
| `display_buffer` (SSRAM) to PSRAM copy | 9.4 ms |
| CPU read from PSRAM (uncached) | ~35 ms per frame, which is why the ring is filled from SSRAM |
| USB read of the eMMC through LoadTexture | ~15.5 MB/s |
| 10 s of `lvgl_timeline` (9.4, 9.5, 9.6) | 300 frames, 0 dropped, queue max 2/8, 129 MB raw, ~180 KB MP4 |

## Limits

- **CPU:** the eMMC driver busy-waits on writes, so recording costs CPU:
  `lvgl_timeline` went from ~1% to ~80% while recording, still at 60 fps. The
  writer runs below the GUI task, so rendering always comes first.
- **eMMC sharing:** FatFs is built without `FF_FS_REENTRANT`, so nothing else
  may use the eMMC while recording. The smartwatch (wallpapers) and
  `lvgl_ttf` (font) read it while running and are not safe to record.
- **Format:** RGB565 display buffers only.
- **Length:** FAT32 caps one file at 4 GB, about 5.5 minutes at 464x464, 30 fps.
