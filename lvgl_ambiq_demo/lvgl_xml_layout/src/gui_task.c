//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//! The whole screen is described in xml/grid_view.xml and xml/grid_cell.xml
//! and built by the LVGL XML loader: a title label above a scrollable 3 x 6
//! grid whose cells read 00, 01, 02, 10, ... (row, column).
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
#include "lvgl_xml_layout.h"
#include "xml_assets.h"

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
// Build the screen from XML.
//
//*****************************************************************************
static void
xml_layout_screen_create(void)
{
    //
    // XML refers to fonts by name; only the default font is registered
    // out of the box.
    //
    lv_xml_register_font(NULL, "title", &lv_font_montserrat_26);
    lv_xml_register_font(NULL, "cell", &lv_font_montserrat_20);

    //
    // Register the components from the .xml files (see xml_assets.c). A
    // component must be registered before a view that uses it is created.
    //
    if (lv_xml_register_component_from_data("grid_cell", g_pcGridCellXml) != LV_RESULT_OK ||
        lv_xml_register_component_from_data("grid_view", g_pcGridViewXml) != LV_RESULT_OK)
    {
        LV_LOG_ERROR("XML component registration failed\n");
        return;
    }

    lv_obj_t * view = (lv_obj_t *)lv_xml_create(lv_screen_active(), "grid_view", NULL);
    if (view == NULL)
    {
        LV_LOG_ERROR("XML view creation failed\n");
        return;
    }

    //
    // Named widgets can be looked up from C, e.g. to attach event handlers.
    //
    lv_obj_t * grid = lv_obj_find_by_name(view, "grid");
    am_util_stdio_printf("XML layout: grid %s, %d cells\n",
                         grid ? "found" : "missing",
                         grid ? (int)lv_obj_get_child_count(grid) : 0);
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

    xml_layout_screen_create();

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
