//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//! The whole screen is described in XML and built by the LVGL XML loader:
//! a title label above a scrollable 3 x 6 grid whose cells read 00, 01, 02,
//! 10, ... (row, column).
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
// XML components.
//
// grid_cell is one rounded tile with a centred label; its "text" prop sets
// the label. grid_view is the screen: a title and a container laid out as a
// grid of 3 columns x 6 rows. Six 70 px rows do not fit in the 250 px tall
// container, so it scrolls vertically.
//
// Each cell picks its grid slot through style_grid_cell_column_pos and
// style_grid_cell_row_pos, which the instance passes to the lv_obj the
// component extends.
//
//*****************************************************************************
static const char g_pcGridCellXml[] =
    "<component>"
    "  <api>"
    "    <prop name=\"text\" type=\"string\" default=\"--\"/>"
    "  </api>"
    "  <view extends=\"lv_obj\" width=\"100\" height=\"70\" scrollable=\"false\""
    "        style_radius=\"12\" style_border_width=\"0\" style_pad_all=\"0\""
    "        style_bg_color=\"0x1f6f8b\">"
    "    <lv_label text=\"$text\" align=\"center\""
    "              style_text_font=\"cell\" style_text_color=\"0xffffff\"/>"
    "  </view>"
    "</component>";

static const char g_pcGridViewXml[] =
    "<component>"
    "  <view extends=\"lv_obj\" width=\"100%\" height=\"100%\" scrollable=\"false\""
    "        style_radius=\"0\" style_border_width=\"0\" style_bg_color=\"0x003a57\">"
    "    <lv_label text=\"XML layout\" align=\"top_mid\" y=\"60\""
    "              style_text_font=\"title\" style_text_color=\"0xffffff\"/>"
    "    <lv_obj name=\"grid\" width=\"340\" height=\"250\" align=\"center\" y=\"30\""
    "            scrollbar_mode=\"auto\""
    "            style_bg_opa=\"0\" style_border_width=\"0\""
    "            style_pad_all=\"10\" style_pad_row=\"10\" style_pad_column=\"10\""
    "            style_layout=\"grid\""
    "            style_grid_column_dsc_array=\"100 100 100\""
    "            style_grid_row_dsc_array=\"70 70 70 70 70 70\">"
    "      <grid_cell text=\"00\" style_grid_cell_row_pos=\"0\" style_grid_cell_column_pos=\"0\"/>"
    "      <grid_cell text=\"01\" style_grid_cell_row_pos=\"0\" style_grid_cell_column_pos=\"1\"/>"
    "      <grid_cell text=\"02\" style_grid_cell_row_pos=\"0\" style_grid_cell_column_pos=\"2\"/>"
    "      <grid_cell text=\"10\" style_grid_cell_row_pos=\"1\" style_grid_cell_column_pos=\"0\"/>"
    "      <grid_cell text=\"11\" style_grid_cell_row_pos=\"1\" style_grid_cell_column_pos=\"1\"/>"
    "      <grid_cell text=\"12\" style_grid_cell_row_pos=\"1\" style_grid_cell_column_pos=\"2\"/>"
    "      <grid_cell text=\"20\" style_grid_cell_row_pos=\"2\" style_grid_cell_column_pos=\"0\"/>"
    "      <grid_cell text=\"21\" style_grid_cell_row_pos=\"2\" style_grid_cell_column_pos=\"1\"/>"
    "      <grid_cell text=\"22\" style_grid_cell_row_pos=\"2\" style_grid_cell_column_pos=\"2\"/>"
    "      <grid_cell text=\"30\" style_grid_cell_row_pos=\"3\" style_grid_cell_column_pos=\"0\"/>"
    "      <grid_cell text=\"31\" style_grid_cell_row_pos=\"3\" style_grid_cell_column_pos=\"1\"/>"
    "      <grid_cell text=\"32\" style_grid_cell_row_pos=\"3\" style_grid_cell_column_pos=\"2\"/>"
    "      <grid_cell text=\"40\" style_grid_cell_row_pos=\"4\" style_grid_cell_column_pos=\"0\"/>"
    "      <grid_cell text=\"41\" style_grid_cell_row_pos=\"4\" style_grid_cell_column_pos=\"1\"/>"
    "      <grid_cell text=\"42\" style_grid_cell_row_pos=\"4\" style_grid_cell_column_pos=\"2\"/>"
    "      <grid_cell text=\"50\" style_grid_cell_row_pos=\"5\" style_grid_cell_column_pos=\"0\"/>"
    "      <grid_cell text=\"51\" style_grid_cell_row_pos=\"5\" style_grid_cell_column_pos=\"1\"/>"
    "      <grid_cell text=\"52\" style_grid_cell_row_pos=\"5\" style_grid_cell_column_pos=\"2\"/>"
    "    </lv_obj>"
    "  </view>"
    "</component>";

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
    // Components must be registered before a view that uses them is created.
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
