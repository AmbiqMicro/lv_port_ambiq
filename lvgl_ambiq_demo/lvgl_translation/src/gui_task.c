//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//! Two buttons pick the language (PT-BR or EN) and the label under them
//! switches between "Bom dia!" and "Good morning!" through the LVGL
//! translation module.
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
#include "lvgl_translation.h"

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
// Translations.
//
// The translations array has one row per tag, with one column per language
// in the order of the languages array.
//
//*****************************************************************************
static const char * g_pcLanguages[] = {"pt-BR", "en", NULL};
static const char * g_pcTags[] = {"greeting", NULL};
static const char * g_pcTranslations[] =
{
    "Bom dia!", "Good morning!",
};

static lv_obj_t * g_psBtnPtBr;
static lv_obj_t * g_psBtnEn;

//*****************************************************************************
//
// Language button handler: switch the language and mark the pressed button.
//
//*****************************************************************************
static void
language_btn_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target_obj(e);
    const char * lang = (const char *)lv_event_get_user_data(e);

    lv_obj_remove_state(g_psBtnPtBr, LV_STATE_CHECKED);
    lv_obj_remove_state(g_psBtnEn, LV_STATE_CHECKED);
    lv_obj_add_state(btn, LV_STATE_CHECKED);

    lv_translation_set_language(lang);
    am_util_stdio_printf("Language: %s -> %s\n", lang, lv_tr("greeting"));
}

static lv_obj_t *
language_btn_create(lv_obj_t * parent, const char * text, const char * lang)
{
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_size(btn, 140, 70);
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREEN), LV_STATE_CHECKED);
    lv_obj_add_event_cb(btn, language_btn_cb, LV_EVENT_CLICKED, (void *)lang);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_center(label);

    return btn;
}

//*****************************************************************************
//
// Build the translation screen.
//
//*****************************************************************************
static void
translation_screen_create(void)
{
    lv_translation_add_static(g_pcLanguages, g_pcTags, g_pcTranslations);
    lv_translation_set_language("pt-BR");

    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_set_style_text_color(scr, lv_color_hex(0xffffff), LV_PART_MAIN);

    //
    // Row with the two language buttons.
    //
    lv_obj_t * row = lv_obj_create(scr);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 30, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -50);

    g_psBtnPtBr = language_btn_create(row, "PT-BR", "pt-BR");
    g_psBtnEn = language_btn_create(row, "EN", "en");
    lv_obj_add_state(g_psBtnPtBr, LV_STATE_CHECKED);

    //
    // Greeting under the buttons. The translation tag makes the label
    // follow lv_translation_set_language() on its own. It spans the
    // display width and centres its text, so every translation stays
    // centred whatever its length.
    //
    lv_obj_t * greeting = lv_label_create(scr);
    lv_obj_set_style_text_font(greeting, &lv_font_montserrat_40, 0);
    lv_obj_set_width(greeting, lv_pct(100));
    lv_obj_set_style_text_align(greeting, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_translation_tag(greeting, "greeting");
    lv_obj_align(greeting, LV_ALIGN_CENTER, 0, 60);
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

    translation_screen_create();

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
