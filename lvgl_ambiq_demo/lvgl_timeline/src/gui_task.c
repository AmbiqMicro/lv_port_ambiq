//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//! Three tab buttons (00, 01, 02) sit above three side-by-side containers in
//! light red, green and blue, each holding a lorem ipsum label. Pressing a
//! tab plays an animation timeline that grows its container to the full
//! width of the content area and collapses the other two to 0.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "gui_task.h"
#include "lvgl_timeline.h"

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Semaphores
//
//*****************************************************************************
SemaphoreHandle_t lvgl_mutex = NULL;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

void lv_ambiq_log_printf(lv_log_level_t level, const char * buf)
{
    g_pfnCharPrint((char *)buf);
}

//*****************************************************************************
//
// Layout and timing.
//
//*****************************************************************************
#define TAB_COUNT           3
#define AREA_W              340     // content area under the tabs
#define AREA_H              250
#define PANEL_PAD           16
#define ANIM_TIME_MS        400

static const uint32_t g_pui32Colors[TAB_COUNT] =
{
    0xff6666, 0x66ff66, 0x6666ff,
};

static const char * g_pcTabText[TAB_COUNT] =
{
    "00", "01", "02",
};

static const char * g_pcLorem[TAB_COUNT] =
{
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
    "eiusmod tempor incididunt ut labore et dolore magna aliqua.",
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
    "nisi ut aliquip ex ea commodo consequat.",
    "Duis aute irure dolor in reprehenderit in voluptate velit esse "
    "cillum dolore eu fugiat nulla pariatur.",
};

static lv_obj_t * g_psTabs[TAB_COUNT];
static lv_obj_t * g_psPanels[TAB_COUNT];

//
// The timeline of the transition in progress, NULL when none has run yet.
//
static lv_anim_timeline_t * g_psTimeline;

//*****************************************************************************
//
// Timeline construction.
//
// A timeline holds animations at fixed start/end values, so one is built per
// press from the panels' current widths: the selected panel goes to AREA_W,
// the others to 0. All three run over the same time with the same path, so
// the widths always add up to AREA_W and the row never overflows, and
// a press during a transition continues from wherever the panels are.
//
//*****************************************************************************
static void
panel_width_exec_cb(void * var, int32_t v)
{
    lv_obj_set_width((lv_obj_t *)var, v);
}

static void
select_tab(uint32_t ui32Sel)
{
    //
    // Stop and free the previous transition; the panels keep the widths it
    // reached.
    //
    if (g_psTimeline)
    {
        lv_anim_timeline_delete(g_psTimeline);
    }
    g_psTimeline = lv_anim_timeline_create();

    //
    // Widths set since the last refresh are only reflected after a layout
    // pass.
    //
    lv_obj_update_layout(g_psPanels[0]);

    for (uint32_t i = 0; i < TAB_COUNT; i++)
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_psPanels[i]);
        lv_anim_set_exec_cb(&a, panel_width_exec_cb);
        lv_anim_set_values(&a, lv_obj_get_width(g_psPanels[i]),
                           i == ui32Sel ? AREA_W : 0);
        lv_anim_set_duration(&a, ANIM_TIME_MS);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
        lv_anim_timeline_add(g_psTimeline, 0, &a);

        if (i == ui32Sel)
        {
            lv_obj_add_state(g_psTabs[i], LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_remove_state(g_psTabs[i], LV_STATE_CHECKED);
        }
    }

    lv_anim_timeline_start(g_psTimeline);
    am_util_stdio_printf("Tab %s: %u ms timeline\n", g_pcTabText[ui32Sel],
                         lv_anim_timeline_get_playtime(g_psTimeline));
}

static void
tab_clicked_cb(lv_event_t * e)
{
    select_tab((uint32_t)(uintptr_t)lv_event_get_user_data(e));
}

//*****************************************************************************
//
// Build the timeline screen.
//
//*****************************************************************************
static void
timeline_screen_create(void)
{
    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN);

    //
    // Tab buttons. The selected one takes its panel's colour.
    //
    lv_obj_t * tabs = lv_obj_create(scr);
    lv_obj_remove_style_all(tabs);
    lv_obj_set_size(tabs, AREA_W, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(tabs, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tabs, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_align(tabs, LV_ALIGN_TOP_MID, 0, 70);

    for (uint32_t i = 0; i < TAB_COUNT; i++)
    {
        lv_obj_t * btn = lv_button_create(tabs);
        lv_obj_set_size(btn, 100, 56);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x5a6b78), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(g_pui32Colors[i]), LV_STATE_CHECKED);
        lv_obj_set_style_text_color(btn, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_color(btn, lv_color_hex(0x202020), LV_STATE_CHECKED);
        lv_obj_add_event_cb(btn, tab_clicked_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text(label, g_pcTabText[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
        lv_obj_center(label);

        g_psTabs[i] = btn;
    }

    //
    // Content area: a row of the three panels. Each label keeps the full
    // panel width, so a collapsing panel clips its text instead of
    // re-wrapping it every frame.
    //
    lv_obj_t * area = lv_obj_create(scr);
    lv_obj_remove_style_all(area);
    lv_obj_set_size(area, AREA_W, AREA_H);
    lv_obj_set_flex_flow(area, LV_FLEX_FLOW_ROW);
    lv_obj_set_scrollbar_mode(area, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(area, tabs, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    for (uint32_t i = 0; i < TAB_COUNT; i++)
    {
        lv_obj_t * panel = lv_obj_create(area);
        lv_obj_set_size(panel, 0, lv_pct(100));
        lv_obj_set_style_bg_color(panel, lv_color_hex(g_pui32Colors[i]), 0);
        lv_obj_set_style_border_width(panel, 0, 0);
        lv_obj_set_style_radius(panel, 12, 0);
        lv_obj_set_style_pad_all(panel, PANEL_PAD, 0);
        lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t * label = lv_label_create(panel);
        lv_label_set_text(label, g_pcLorem[i]);
        lv_obj_set_width(label, AREA_W - 2 * PANEL_PAD);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x202020), 0);

        g_psPanels[i] = panel;
    }

    //
    // Start on 00, fully open.
    //
    lv_obj_set_width(g_psPanels[0], AREA_W);
    lv_obj_add_state(g_psTabs[0], LV_STATE_CHECKED);
}

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("Gui task start!\n");
    //
    // Init file system
    //
    LV_LOG_INFO("Init file system...\r\n");
    lv_ambiq_fs_init();
    //
    // Init LVGL.
    //
    lv_init();

    lv_tick_set_cb(xTaskGetTickCount);

#if LV_USE_LOG == 1
    lv_log_register_print_cb(lv_ambiq_log_printf);
#endif

    //
    // Iint display
    //
    LV_LOG_INFO("setup display...\r\n");
    lv_ambiq_display_init();

    //
    // Set up LVGL touch driver: init touch device and set it as the input device for lvgl.
    //
    LV_LOG_INFO("setup touch...\r\n");
    lv_ambiq_touch_init();

    /* Create a mutex to avoid the concurrent calling of LVGL functions. */
    lvgl_mutex = xSemaphoreCreateMutex();
    if( lvgl_mutex == NULL )
    {
        LV_LOG_ERROR("LVGL mutex create failed!\n");
        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    timeline_screen_create();

    while(1)
    {
        uint32_t time_till_next;

        // Run timer handler.
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
        time_till_next = lv_timer_handler();
        xSemaphoreGive(lvgl_mutex);

        // Delay
        vTaskDelay(time_till_next);

    }
}
