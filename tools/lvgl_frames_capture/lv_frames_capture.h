//*****************************************************************************
//
//! @file lv_frames_capture.h
//!
//! @brief Record the LVGL display to the eMMC as raw frames.
//!
//! A grab task copies the port's full-screen display_buffer into a PSRAM ring
//! at a fixed rate (30 fps by default), under the display buffer lock so a
//! frame is never torn. A low-priority writer task appends every grabbed
//! frame to <root>/<NNNN>/frames.raw on the eMMC, one sequential file per
//! session, and writes meta.txt (size, pixel format, fps, timing) on stop.
//!
//! Requires the eMMC to be mounted (lv_ambiq_fs_init()) and the display to
//! be set up (lv_ambiq_display_init()), and an RGB565 display buffer.
//!
//! FatFs is built without FF_FS_REENTRANT, so nothing else may use the eMMC
//! while recording: demos that stream from it at run time (the smartwatch
//! wallpapers, lvgl_ttf's font) are not safe to record.
//!
//! Measured on the Apollo510 EVB, 464x464 RGB565 (430592 B per frame):
//! f_write of one frame takes ~17.6 ms (~24 MB/s, worst ~36 ms), and the
//! SSRAM -> PSRAM copy ~9.4 ms, so 30 fps (12.9 MB/s) fits with the ring
//! absorbing the slow writes. The eMMC driver busy-waits on writes, which is
//! why the writer runs below the GUI task.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef LV_FRAMES_CAPTURE_H
#define LV_FRAMES_CAPTURE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
// Configuration.
//
//*****************************************************************************
typedef struct
{
    const char * root;      // eMMC folder holding the sessions, "/lvcap"
    uint32_t fps;           // grab rate, 30
    uint32_t seconds;       // stop on its own after this long; 0 = until stop()
    uint32_t slots;         // PSRAM ring slots, 8 (~3.4 MB at 464x464 RGB565)
} lv_frames_capture_cfg_t;

//*****************************************************************************
//
// Telemetry, also written to meta.txt.
//
//*****************************************************************************
typedef struct
{
    bool running;           // between start() and the writer finishing
    char dir[32];           // session folder, e.g. "/lvcap/0003"
    uint32_t grabbed;       // frames copied into the ring
    uint32_t written;       // frames on the eMMC
    uint32_t dropped;       // grab ticks with no free slot (writer behind)
    uint32_t queue_max;     // most slots waiting for the writer at once
    uint32_t grab_ms_max;   // longest lock wait + copy
    uint32_t write_ms_max;  // longest f_write of one frame
    uint32_t write_ms_total;
} lv_frames_capture_stats_t;

//*****************************************************************************
//
// API.
//
//*****************************************************************************

//
// Fill a config with the defaults above.
//
extern void lv_frames_capture_cfg_init(lv_frames_capture_cfg_t * cfg);

//
// Create the session folder and file, the ring and the two tasks, and start
// grabbing. Returns false (and logs why) if anything is missing.
//
extern bool lv_frames_capture_start(const lv_frames_capture_cfg_t * cfg);

//
// Stop grabbing. The writer drains the ring, closes the file and writes
// meta.txt; poll lv_frames_capture_get_stats() for running == false.
//
extern void lv_frames_capture_stop(void);

extern void lv_frames_capture_get_stats(lv_frames_capture_stats_t * stats);

#ifdef __cplusplus
}
#endif

#endif // LV_FRAMES_CAPTURE_H
