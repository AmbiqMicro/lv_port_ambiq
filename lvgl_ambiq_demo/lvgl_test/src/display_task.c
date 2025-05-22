//*****************************************************************************
//
//! @file display_task.c
//!
//! @brief Task to handle DISPLAY operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
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

#include "display_task.h"

#include "demos/lv_demos.h"

//*****************************************************************************
//
// DISPLAY task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;

//*****************************************************************************
//
// Semaphores
//
//*****************************************************************************
SemaphoreHandle_t lvgl_mutex = NULL;

//*****************************************************************************
//
// Private data
//
//*****************************************************************************

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
DisplayTask(void *pvParameters)
{
    int ret;

    LV_LOG_INFO("Display task start!\n");

    /* Create a mutex to avoid the concurrent calling of LVGL functions. */
    lvgl_mutex = xSemaphoreCreateMutex();
    if( lvgl_mutex == NULL )
    {
        LV_LOG_ERROR("LVGL mutex create failed!\n");
        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    lv_demo_benchmark();
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