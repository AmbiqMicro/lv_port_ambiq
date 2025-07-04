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
#include "lvgl_ttf.h"
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

/*
 * Load a vector font
 * ThorVG needs to be enabled, LV_USE_VECTOR_GRAPHICS=1
 */
void lv_example_freetype_2_vector_font(uint32_t font_size, uint32_t border_width)
{
    /*Chinese demo text, total 312 characters (including punctuation)*/
    const char * cntxt =
        "本示例基于 Ambiq 平台，展示中文字体在图形界面中的基本显示效果，"
        "用于验证字体文件加载是否正确，以及中文字符能否正常渲染和显示。"
        "本测试用例不涉及具体输入或交互逻辑，便于用户快速确认平台对中文字符集的支持情况。\n"
        "本示例基于 Ambiq 平台，展示中文字体在图形界面中的基本显示效果，"
        "用于验证字体文件加载是否正确，以及中文字符能否正常渲染和显示。"
        "本测试用例不涉及具体输入或交互逻辑，便于用户快速确认平台对中文字符集的支持情况。\n"
        "本示例基于 Ambiq 平台，展示中文字体在图形界面中的基本显示效果，"
        "用于验证字体文件加载是否正确，以及中文字符能否正常渲染和显示。"
        "本测试用例不涉及具体输入或交互逻辑，便于用户快速确认平台对中文字符集的支持情况。\n";

    /*English demo text, total 300 characters (including punctuation)*/
    const char * entxt =
        "ThisexampledemonstratesintegratingFreeTypewithLVGLtoenable"
        "high-qualityfontrenderinginembeddedUIs.ItshowcasesTrueType"
        "supportinlvgl.ThesolutionhandlesfontcachingandGPUacceleration"
        "whereavailable,ensuringoptimalperformance.Apracticalreferencefor"
        "developersenhancingLVGLwithprofessional-gradetextrendering.";

    lv_font_t * font = lv_freetype_font_create("E:SourceHanSansSC-Normal.ttf",
                                               LV_FREETYPE_FONT_RENDER_MODE_OUTLINE,
                                               font_size,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);


    if(!font) {
        LV_LOG_ERROR("Freetype font create failed.");
        return;
    }

    /*Create style with the new font*/
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_color(&style, lv_color_hex(0xFF0000));
    lv_style_set_text_opa(&style, LV_OPA_100);

    /* Simulate font glyph preloading by creating a label.
     * This step ensures that necessary glyphs are loaded into memory*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_obj_add_style(label, &style, 0);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, cntxt);
    lv_obj_center(label);

    /* Create a second label to test rendering performance under ideal conditions,
     * assuming all required glyphs are already loaded into memory. */
    lv_obj_t * label2 = lv_label_create(lv_screen_active());
    lv_obj_add_style(label2, &style, 0);
    lv_obj_set_width(label2, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label2, cntxt);
    lv_obj_center(label2);
}

/**
 * Load a font with FreeType
 */
void lv_example_freetype_2(void)
{
    /*Create a font*/
    lv_font_t * font = lv_freetype_font_create("E:Lato-Regular.ttf",
                                               LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                               400,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);

    /* this font is created from a downscaled NotoColorEmoji to 34x32px
     * Subset containing only a single emoji was created using fonttools:
     * Command: fonttools subset NotoColorEmoji.ttf --text=😀 */
    lv_font_t * font_emoji = lv_freetype_font_create("E:NotoColorEmoji-32.subset.ttf",
                                                     LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                                     200,
                                                     LV_FREETYPE_FONT_STYLE_NORMAL);

    if(!font || !font_emoji) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    font->fallback = font_emoji;

    /*Create style with the new font*/
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

    /*Create a label with the new style*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_obj_add_style(label, &style, 0);
    lv_label_set_text(label, "Hello world\nI'm a font created with FreeType 😀");
    lv_obj_center(label);
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

    lv_ambiq_fs_init();

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

    lv_example_freetype_2_vector_font(24, 2);

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