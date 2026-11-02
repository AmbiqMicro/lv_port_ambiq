# Record the LVGL display to the eMMC

`lv_frames_capture.{h,c}` grab the port's full-screen `display_buffer` at a
fixed rate (30 fps by default) into a PSRAM ring and append every frame to one
raw file on the EVB's eMMC. `scripts/pull.sh` copies a recording to the PC over
USB and `scripts/encode.sh` turns it into an MP4.

## Record

Build any demo with the recording length in seconds. Switching the flag on or
off needs a clean build (`make clean`), as make does not track defines:

```sh
cd lvgl_ambiq_demo/lvgl_timeline/apollo510_evb
make clean && make LV_FRAMES_CAPTURE_SECONDS=10
```

Flash and run it as usual. Recording starts when the display is set up and
stops on its own; the SWO log shows

```
lvcap: recording 464x464 @ 30 fps to /lvcap/0001/frames.raw (10 s, 0 = until stop)
lvcap: /lvcap/0001 done, 300 frames 464x464 @ 30 fps, 0 dropped, queue max 2/8, write avg 19 ms max 60 ms, grab max 19 ms
```

Each run creates the next `/lvcap/NNNN/` with `frames.raw` (headerless RGB565
frames) and `meta.txt` (size, pixel format, fps, and the counters above).

To start and stop from code instead, call `lv_frames_capture_start()` after
`lv_ambiq_display_init()` and `lv_frames_capture_stop()`, and add the folder to
the build as `common.mk` does for the flag.

## Pull and encode

Connect the EVB's USB-C port (not the J-Link one), then from WSL:

```sh
LVCAP_JLINK=<J-Link wrapper> tools/lvgl_frames_capture/scripts/pull.sh lvgl_ambiq_demo/lvgl_timeline/apollo510_evb
```

It flashes `tools/LoadTexture` (the eMMC as a USB drive), copies the newest
session to `<board dir>/lvcap/NNNN/` and encodes `capture.mp4` there. Without
`LVCAP_JLINK` it flashes with `JLinkExe` and LoadTexture's `flash.jlink`. The
board is left running LoadTexture; flash the demo again afterwards. `-rm`
deletes the session from the eMMC after copying, `-src <dir>` uses an already
mounted eMMC, and `encode.sh <session dir>` encodes on its own.

## Measured on the Apollo510 EVB (464x464 RGB565, 430592 B per frame)

| | |
|---|---|
| `f_write` of one frame, sequential file | 17.6 ms avg, 32-60 ms worst (~24 MB/s) |
| one file per frame instead | 23.9 ms avg, 38.6 ms worst |
| `display_buffer` (SSRAM) to PSRAM copy | 9.4 ms |
| 30 fps needs | 12.9 MB/s, 33.3 ms per frame |
| USB read of the eMMC (LoadTexture) | ~15.5 MB/s |
| 10 s recording, `lvgl_timeline` | 300 frames, 0 dropped, 129 MB raw, 176 KB MP4 |

A slow write is absorbed by the 8-slot ring (3.4 MB of PSRAM); a grab with no
free slot is counted in `dropped` rather than delaying the next one.

## Limits

- The eMMC driver busy-waits on writes, so recording costs CPU (the timeline
  demo went from ~1% to ~80% while recording, still at 60 fps). The writer runs
  below the GUI task so rendering always comes first.
- FatFs is not built reentrant: nothing else may use the eMMC while recording,
  so the smartwatch (wallpapers) and `lvgl_ttf` (font) are not safe to record.
- RGB565 display buffers only. FAT32 caps one recording at 4 GB (~5.5 min).
