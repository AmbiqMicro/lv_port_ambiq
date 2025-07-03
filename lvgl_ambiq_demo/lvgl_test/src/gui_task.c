//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//!
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
#include "lv_draw_ambiq_private.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "gui_task.h"
#include "lvgl_test.h"
#include "demos/lv_demos.h"

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
    g_pfnCharPrint(buf);
}

void lv_example_style_5(void)
{
    static lv_style_t style;
    lv_style_init(&style);

    lv_color_t color_new = lv_color_make(0xff, 0x20, 0x30);

    /*Set a background color and a radius*/
    //lv_style_set_radius(&style, 40);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_bg_color(&style, color_new);

    // /*Add a shadow*/
    // lv_style_set_shadow_width(&style, 55);
    // lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_shadow_opa(&style, 25);
    // lv_style_set_shadow_offset_x(&style, 10);
    // lv_style_set_shadow_offset_y(&style, -20);

    /*Create an object with the new style*/
    lv_obj_t * obj = lv_obj_create(lv_screen_active());
    lv_obj_set_size(obj, 500, 500);
    lv_obj_add_style(obj, &style, 0);
    lv_obj_center(obj);
}

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    int ret;

    am_util_stdio_printf("Gui task start!\n");

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

    // lv_demo_benchmark();
    lv_demo_music();
    //lv_demo_scroll();
    //lv_demo_vector_graphic_not_buffered();    
    //lv_example_style_5();

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