//*****************************************************************************
//
//! @file lv_frames_capture.c
//!
//! @brief Record the LVGL display to the eMMC as raw frames.
//!
//! See lv_frames_capture.h. Two tasks and two queues of ring-slot indices:
//!
//!   grab task (above the GUI)        writer task (below the GUI)
//!   every 1/fps s:                   forever:
//!     take a slot from free_q          take a slot from full_q
//!     lock display_buffer, copy        f_write it to frames.raw
//!     put the slot on full_q           put the slot back on free_q
//!
//! A grab tick with no free slot is counted as dropped rather than waited
//! for, so the grab timing never slips; meta.txt reports the count.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ff.h"
#include "am_mem.h"
#include "lv_frames_capture.h"

//*****************************************************************************
//
// The port's display buffer and its lock (lv_ambiq_display.c). The lock is
// held while the flush copies a frame in and while the panel reads it out,
// so holding it for the copy guarantees a whole frame.
//
//*****************************************************************************
extern lv_draw_buf_t * display_buffer;
extern SemaphoreHandle_t display_buffer_lock;

//*****************************************************************************
//
// Tasks.
//
//*****************************************************************************
#define GRAB_TASK_PRIORITY      (configMAX_PRIORITIES - 2)
#define WRITER_TASK_PRIORITY    (1)
#define GRAB_TASK_STACK         (512)
#define WRITER_TASK_STACK       (1024)
#define LOCK_TIMEOUT_MS         (100)
#define SLOT_STOP               (0xFFFFFFFFU)

//*****************************************************************************
//
// State.
//
//*****************************************************************************
static lv_frames_capture_cfg_t s_cfg;
static lv_frames_capture_stats_t s_stats;
static volatile bool s_stop;

static uint8_t * s_ring;
static uint32_t s_width;
static uint32_t s_height;
static uint32_t s_frame_bytes;

static QueueHandle_t s_free_q;
static QueueHandle_t s_full_q;
static FIL s_file;

//*****************************************************************************
//
// Session folder: the first unused <root>/NNNN.
//
//*****************************************************************************
static bool
session_dir_create(void)
{
    DIR dir;
    FILINFO info;
    uint32_t next = 1;

    FRESULT res = f_mkdir(s_cfg.root);
    if (res != FR_OK && res != FR_EXIST)
    {
        am_util_stdio_printf("lvcap: mkdir %s failed (%d), is the eMMC mounted?\n", s_cfg.root, res);
        return false;
    }

    if (f_opendir(&dir, s_cfg.root) == FR_OK)
    {
        while (f_readdir(&dir, &info) == FR_OK && info.fname[0] != '\0')
        {
            uint32_t n = 0;
            const char * c = info.fname;
            while (*c >= '0' && *c <= '9')
            {
                n = n * 10 + (uint32_t)(*c++ - '0');
            }
            if (*c == '\0' && c != info.fname && n >= next)
            {
                next = n + 1;
            }
        }
        f_closedir(&dir);
    }

    am_util_stdio_sprintf(s_stats.dir, "%s/%04u", s_cfg.root, next);
    res = f_mkdir(s_stats.dir);
    if (res != FR_OK)
    {
        am_util_stdio_printf("lvcap: mkdir %s failed (%d)\n", s_stats.dir, res);
        return false;
    }
    return true;
}

//*****************************************************************************
//
// meta.txt: what encode.sh needs to read frames.raw, plus the telemetry.
//
//*****************************************************************************
static void
meta_write(void)
{
    char path[48];
    char text[512];
    FIL f;
    UINT bw;

    am_util_stdio_sprintf(path, "%s/meta.txt", s_stats.dir);
    if (f_open(&f, path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        am_util_stdio_printf("lvcap: cannot write %s\n", path);
        return;
    }

    am_util_stdio_sprintf(text,
                          "width=%u\nheight=%u\npix_fmt=rgb565le\nfps=%u\n"
                          "frames=%u\ngrabbed=%u\ndropped=%u\nslots=%u\nqueue_max=%u\n"
                          "grab_ms_max=%u\nwrite_ms_avg=%u\nwrite_ms_max=%u\n",
                          s_width, s_height, s_cfg.fps,
                          s_stats.written, s_stats.grabbed, s_stats.dropped, s_cfg.slots,
                          s_stats.queue_max, s_stats.grab_ms_max,
                          s_stats.written ? s_stats.write_ms_total / s_stats.written : 0,
                          s_stats.write_ms_max);
    f_write(&f, text, strlen(text), &bw);
    f_close(&f);
}

//*****************************************************************************
//
// Writer task: drains full_q to the eMMC until the stop marker, then
// finishes the session and frees everything.
//
//*****************************************************************************
static void
writer_task(void * pvParameters)
{
    uint32_t slot;
    UINT bw;

    while (xQueueReceive(s_full_q, &slot, portMAX_DELAY) == pdTRUE && slot != SLOT_STOP)
    {
        TickType_t t0 = xTaskGetTickCount();
        FRESULT res = f_write(&s_file, s_ring + slot * s_frame_bytes, s_frame_bytes, &bw);
        uint32_t ms = (xTaskGetTickCount() - t0) * portTICK_PERIOD_MS;

        if (res != FR_OK || bw != s_frame_bytes)
        {
            am_util_stdio_printf("lvcap: write failed (%d), eMMC full? stopping\n", res);
            s_stop = true;
        }
        else
        {
            s_stats.written++;
            s_stats.write_ms_total += ms;
            if (ms > s_stats.write_ms_max)
            {
                s_stats.write_ms_max = ms;
            }
        }
        xQueueSend(s_free_q, &slot, 0);
    }

    f_close(&s_file);
    meta_write();

    am_util_stdio_printf("lvcap: %s done, %u frames %ux%u @ %u fps, %u dropped, "
                         "queue max %u/%u, write avg %u ms max %u ms, grab max %u ms\n",
                         s_stats.dir, s_stats.written, s_width, s_height, s_cfg.fps,
                         s_stats.dropped, s_stats.queue_max, s_cfg.slots,
                         s_stats.written ? s_stats.write_ms_total / s_stats.written : 0,
                         s_stats.write_ms_max, s_stats.grab_ms_max);

    am_mem_heap_free(&psram_heap, s_ring);
    vQueueDelete(s_free_q);
    vQueueDelete(s_full_q);
    s_ring = NULL;
    s_stats.running = false;
    vTaskDelete(NULL);
}

//*****************************************************************************
//
// Grab task: one frame per 1/fps s, timed from the start so it does not
// drift.
//
//*****************************************************************************
static void
grab_one(void)
{
    uint32_t slot;

    if (xQueueReceive(s_free_q, &slot, 0) != pdTRUE)
    {
        s_stats.dropped++;
        return;
    }

    TickType_t t0 = xTaskGetTickCount();
    if (xSemaphoreTake(display_buffer_lock, pdMS_TO_TICKS(LOCK_TIMEOUT_MS)) != pdTRUE)
    {
        s_stats.dropped++;
        xQueueSend(s_free_q, &slot, 0);
        return;
    }

    uint8_t * dst = s_ring + slot * s_frame_bytes;
    const uint8_t * src = display_buffer->data;
    uint32_t row_bytes = s_width * 2;
    uint32_t stride = display_buffer->header.stride;

    if (stride == row_bytes)
    {
        memcpy(dst, src, s_frame_bytes);
    }
    else
    {
        for (uint32_t y = 0; y < s_height; y++)
        {
            memcpy(dst + y * row_bytes, src + y * stride, row_bytes);
        }
    }
    xSemaphoreGive(display_buffer_lock);

    uint32_t ms = (xTaskGetTickCount() - t0) * portTICK_PERIOD_MS;
    if (ms > s_stats.grab_ms_max)
    {
        s_stats.grab_ms_max = ms;
    }

    s_stats.grabbed++;
    xQueueSend(s_full_q, &slot, 0);

    uint32_t waiting = uxQueueMessagesWaiting(s_full_q);
    if (waiting > s_stats.queue_max)
    {
        s_stats.queue_max = waiting;
    }
}

static void
grab_task(void * pvParameters)
{
    TickType_t start = xTaskGetTickCount();
    uint32_t total = s_cfg.seconds * s_cfg.fps;

    for (uint32_t n = 0; !s_stop && (total == 0 || n < total); n++)
    {
        TickType_t due = start + pdMS_TO_TICKS((uint64_t)n * 1000 / s_cfg.fps);
        TickType_t now = xTaskGetTickCount();
        if ((int32_t)(due - now) > 0)
        {
            vTaskDelay(due - now);
        }
        grab_one();
    }

    uint32_t stop = SLOT_STOP;
    xQueueSend(s_full_q, &stop, portMAX_DELAY);
    vTaskDelete(NULL);
}

//*****************************************************************************
//
// API.
//
//*****************************************************************************
void
lv_frames_capture_cfg_init(lv_frames_capture_cfg_t * cfg)
{
    cfg->root = "/lvcap";
    cfg->fps = 30;
    cfg->seconds = 0;
    cfg->slots = 8;
}

bool
lv_frames_capture_start(const lv_frames_capture_cfg_t * cfg)
{
    if (s_stats.running)
    {
        am_util_stdio_printf("lvcap: already running\n");
        return false;
    }
    if (display_buffer == NULL || display_buffer_lock == NULL)
    {
        am_util_stdio_printf("lvcap: call lv_ambiq_display_init() first\n");
        return false;
    }
    if (display_buffer->header.cf != LV_COLOR_FORMAT_RGB565)
    {
        am_util_stdio_printf("lvcap: only RGB565 display buffers are supported\n");
        return false;
    }

    s_cfg = *cfg;
    memset(&s_stats, 0, sizeof(s_stats));
    s_stop = false;
    s_width = display_buffer->header.w;
    s_height = display_buffer->header.h;
    s_frame_bytes = s_width * s_height * 2;

    if (!session_dir_create())
    {
        return false;
    }

    char path[48];
    am_util_stdio_sprintf(path, "%s/frames.raw", s_stats.dir);
    if (f_open(&s_file, path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        am_util_stdio_printf("lvcap: cannot create %s\n", path);
        return false;
    }

    s_ring = am_mem_heap_malloc(&psram_heap, s_cfg.slots * s_frame_bytes);
    s_free_q = xQueueCreate(s_cfg.slots, sizeof(uint32_t));
    s_full_q = xQueueCreate(s_cfg.slots + 1, sizeof(uint32_t));    // +1: stop marker
    if (s_ring == NULL || s_free_q == NULL || s_full_q == NULL)
    {
        am_util_stdio_printf("lvcap: out of memory for %u slots\n", s_cfg.slots);
        f_close(&s_file);
        return false;
    }
    for (uint32_t i = 0; i < s_cfg.slots; i++)
    {
        xQueueSend(s_free_q, &i, 0);
    }

    s_stats.running = true;
    xTaskCreate(writer_task, "lvcap_wr", WRITER_TASK_STACK, NULL, WRITER_TASK_PRIORITY, NULL);
    xTaskCreate(grab_task, "lvcap_grab", GRAB_TASK_STACK, NULL, GRAB_TASK_PRIORITY, NULL);

    am_util_stdio_printf("lvcap: recording %ux%u @ %u fps to %s/frames.raw (%u s, 0 = until stop)\n",
                         s_width, s_height, s_cfg.fps, s_stats.dir, s_cfg.seconds);
    return true;
}

void
lv_frames_capture_stop(void)
{
    s_stop = true;
}

void
lv_frames_capture_get_stats(lv_frames_capture_stats_t * stats)
{
    *stats = s_stats;
}
