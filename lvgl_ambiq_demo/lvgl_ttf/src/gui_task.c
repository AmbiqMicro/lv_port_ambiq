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
#include "lv_ambiq_ttf.h"

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

static lv_obj_t * canvas_create(void)
{
    lv_obj_t * canvas = lv_canvas_create(lv_screen_active());
    lv_obj_set_size(canvas, 500, 360);

    lv_draw_buf_t * draw_buf = lv_draw_buf_create(500, 360, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    lv_draw_buf_clear(draw_buf, NULL);
    lv_canvas_set_draw_buf(canvas, draw_buf);

    return canvas;
}

static void canvas_destroy(lv_obj_t * canvas)
{
    lv_draw_buf_destroy(lv_canvas_get_draw_buf(canvas));
    lv_obj_delete(canvas);
}

#if LV_USE_FREETYPE
void test_draw_sin_wave(void)
{
    const char * string = "lol~ I'm wavvvvvvving~";
    const uint32_t string_len = lv_strlen(string);

    lv_font_t * font = lv_freetype_font_create("E:Lato-Regular.ttf",
                                               LV_FREETYPE_FONT_RENDER_MODE_OUTLINE,
                                               24,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);


    if(!font) {
        LV_LOG_ERROR("Freetype font create failed.");
        return;
    }

    lv_obj_t * canvas = canvas_create();

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_draw_letter_dsc_t letter_dsc;
    lv_draw_letter_dsc_init(&letter_dsc);
    letter_dsc.color = lv_color_hex(0xff0000);
    letter_dsc.font = font;

    {
#define CURVE1_X(t) (t * 2 + 20)
#define CURVE1_Y(t) (lv_trigo_sin(t) * 40 / 32767 + 80)
        int32_t pre_x = CURVE1_X(-1);
        int32_t pre_y = CURVE1_Y(-1);

        for(int16_t i = 0; i < 30; i++) {
            const int32_t angle = i * 10;
            const int32_t x = CURVE1_X(angle);
            const int32_t y = CURVE1_Y(angle);
            letter_dsc.unicode = (uint32_t)string[i % string_len];
            letter_dsc.rotation = lv_atan2(y - pre_y, x - pre_x);
            letter_dsc.rotation = (letter_dsc.rotation > 180 ? letter_dsc.rotation - 360 : letter_dsc.rotation) * 5;
            lv_draw_letter(&layer, &letter_dsc, &(lv_point_t) {
                .x = x, .y = y
            });
            pre_x = x;
            pre_y = y;
        }
    }

    {
#define CURVE2_X(t) (t * 3 + 20)
#define CURVE2_Y(t) (lv_trigo_sin((t) * 4) * 40 / 32767 + 230)

        int32_t pre_x = CURVE2_X(-1);
        int32_t pre_y = CURVE2_Y(-1);
        for(int16_t i = 0; i < 30; i++) {
            const int32_t angle = i * 5;
            const int32_t x = CURVE2_X(angle);
            const int32_t y = CURVE2_Y(angle);

            letter_dsc.unicode = (uint32_t)string[i % string_len];
            letter_dsc.rotation = lv_atan2(y - pre_y, x - pre_x) * 10;
            letter_dsc.color = lv_color_hsv_to_rgb(i * 10, 100, 100);
            lv_draw_letter(&layer, &letter_dsc, &(lv_point_t) {
                .x = x, .y = y
            });

            pre_x = x;
            pre_y = y;
        }
    }

    lv_canvas_finish_layer(canvas, &layer);
}
#endif

#if LV_USE_AMBIQ_TTF
/**
 * @brief Creates a new LVGL font from a binary font file with a selectable loading strategy.
 * @details This function can either load the font directly from the filesystem or, for
 *          better performance, pre-load the entire file into a PSRAM buffer first.
 *
 *          **File Mode (`load_into_psram = false`):**
 *          - Calls `lv_ambiq_ttf_create_file`.
 *          - The font loader will keep the file handle open and perform I/O operations
 *            on demand when glyph shapes are needed.
 *          - Lower initial RAM usage.
 *
 *          **PSRAM Mode (`load_into_psram = true`):**
 *          - Reads the entire font file into a buffer in PSRAM.
 *          - Calls `lv_ambiq_ttf_create_data` using the buffer.
 *          - All subsequent operations read from fast PSRAM, eliminating file I/O lag.
 *          - The PSRAM buffer is attached to `font->user_data` and freed by
 *            `lv_ambiq_ttf_destroy`.
 *
 * @param font_size The initial desired font height in pixels.
 * @param load_into_psram If `true`, the font is loaded into PSRAM. If `false`, it is loaded directly from the file.
 * @return A pointer to the newly created `lv_font_t` object, or `NULL` on failure.
 */
lv_font_t * lv_example_ambiq_ttf_create(uint32_t font_size, bool load_into_psram)
{
    const char* path = "E:SourceHanSansSC-Normal.bin"; // Path on the LVGL virtual filesystem

    lv_ambiq_ttf_handle_t * vg_font;
    uint8_t *bin_data = NULL;

    // --- Strategy 1: Load directly from file ---
    if (!load_into_psram) {
        LV_LOG_INFO("Loading font directly from file: %s", path);
        // This is the simpler path. The lv_ambiq_ttf library will handle all file operations.
        vg_font = lv_ambiq_ttf_load(path, 256, 256);
    }
    else
    {
    // --- Strategy 2: Pre-load the entire file into PSRAM for performance ---
    LV_LOG_INFO("Pre-loading font file '%s' into PSRAM.", path);
    
    lv_fs_file_t file;
    lv_fs_res_t res = lv_fs_open(&file, path, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        LV_LOG_ERROR("Failed to open font file: %s (error: %d)", path, res);
        return NULL;
    }

    // --- Get file length ---
    uint32_t length = 0;
    res = lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    if (res == LV_FS_RES_OK) {
        res = lv_fs_tell(&file, &length);
    }
    if (res != LV_FS_RES_OK) {
        LV_LOG_ERROR("Failed to get file size for: %s", path);
        lv_fs_close(&file);
        return NULL;
    }

    // Rewind the file to the beginning before reading.
    lv_fs_seek(&file, 0, LV_FS_SEEK_SET);
    
    if (length == 0) {
        LV_LOG_ERROR("Font file is empty: %s", path);
        lv_fs_close(&file);
        return NULL;
    }

    // --- Allocate PSRAM and read the file content into it ---
    LV_LOG_INFO("Allocating %lu bytes in PSRAM for font file.", (unsigned long)length);
    bin_data = am_mem_psram_malloc(length);
    if (bin_data == NULL) {
        LV_LOG_ERROR("Failed to allocate %lu bytes in PSRAM.", (unsigned long)length);
        lv_fs_close(&file);
        return NULL;
    }

    uint32_t bytes_read = 0;
    res = lv_fs_read(&file, bin_data, length, &bytes_read);

    // After reading, the file is no longer needed.
    lv_fs_close(&file);

    if (res != LV_FS_RES_OK || bytes_read != length) {
        LV_LOG_ERROR("Failed to read the full font file into buffer. Read %lu of %lu bytes.", (unsigned long)bytes_read, (unsigned long)length);
        am_mem_psram_free(bin_data); // CRITICAL: Free the buffer on failure.
        return NULL;
    }

    // --- Create the font from the in-memory buffer ---
    vg_font = lv_ambiq_ttf_load_from_buffer(bin_data, length);

}

    lv_font_t * new_font =  lv_ambiq_ttf_create(vg_font, font_size, 10);

    new_font->fallback = &lv_font_montserrat_14;

    // --- IMPORTANT: Attach the buffer to the font for proper memory management ---
    new_font->user_data = bin_data;

    return new_font;
}
#endif

#if LV_USE_FREETYPE
lv_font_t * lv_example_freetype_create(uint32_t font_size)
{
    return lv_freetype_font_create("E:SourceHanSansSC-Normal.ttf",
                                               LV_FREETYPE_FONT_RENDER_MODE_OUTLINE,
                                               font_size,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);
}
#endif


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


    // LV_UNUSED(border_width); // Use this if border_width is not used
    // LV_UNUSED(entxt);      // Use this if entxt is not used

    lv_font_t * font = lv_example_ambiq_ttf_create(font_size, true);

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
    lv_obj_set_width(label, lv_pct(80)); // Use less than 100% to see movement
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, cntxt);
    lv_obj_center(label);

    // /* Create a second label to test rendering performance under ideal conditions,
    //  * assuming all required glyphs are already loaded into memory. */
    // lv_obj_t * label2 = lv_label_create(lv_screen_active());
    // lv_obj_add_style(label2, &style, 0);
    // lv_obj_set_width(label2, lv_pct(80));
    // lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP); // Corrected: target label2
    // lv_label_set_text(label2, entxt); // Use the English text for variety
    // // Position label2 below label1
    // lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


    // --- ADDED: Animation for the first label ---
    LV_LOG_INFO("Starting animation for the first label.");
    static lv_anim_t a;
    lv_anim_init(&a);

    /* Set the object to animate */
    lv_anim_set_var(&a, label);

    /* Define the animation path (start and end values) */
    // We will animate the Y-coordinate relative to its current centered position.
    int32_t start_y = lv_obj_get_y(label) - 15; // Move 15 pixels up
    int32_t end_y   = lv_obj_get_y(label) + 15; // Move 15 pixels down

    lv_anim_set_values(&a, start_y, end_y);

    /* Set the function that will apply the animation value */
    // lv_obj_set_y is the function to change the Y-coordinate.
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);

    /* Set animation properties */
    lv_anim_set_time(&a, 2000); // Duration of one-way animation (2 seconds)
    lv_anim_set_playback_time(&a, 2000); // Duration of the return animation (2 seconds)
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); // Loop forever

    /* Start the animation */
    lv_anim_start(&a);
}

#if LV_USE_FREETYPE
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
#endif

//*****************************************************************************
//
// Continuous Text Scroll scenario.
//
// An lv_timer auto-drives a continuous downward scroll and recycles at the
// window boundary, so it "keeps scrolling, keeps turning pages".
// Rendering is left to the native lv_timer_handler() loop in GuiTask.
// FPS can be monitored via LVGL's built-in sysmon (LV_USE_PERF_MONITOR).
//
//*****************************************************************************
#define TR_LINE_HEIGHT      28
#define TR_PAGE_LINES       10
#define TR_PAGE_HEIGHT      (TR_PAGE_LINES * TR_LINE_HEIGHT)
#define TR_WINDOW_PAGES     3                                       /* MAX_DISPLAY_PAGE */
#define TR_VIEWPORT_LINES   9                                       /* MINIMIZED container 9 lines */
#define TR_VIEWPORT_H       (TR_VIEWPORT_LINES * TR_LINE_HEIGHT)
#define TR_DISPLAY_WIDTH    500
#define TR_CHARS_PER_LINE   16
#define TR_TEXT_BUF_SIZE    (TR_PAGE_LINES * TR_CHARS_PER_LINE * 3 + TR_PAGE_LINES + 1)
#define TR_SCROLL_STEP_PX   4       /* px advanced per scroll tick (smooth scroll) */
#define TR_SCROLL_PERIOD_MS 16      /* lv_timer period that drives the scroll */
#define TR_CJK_BASE         0x4E00u
#define TR_CJK_SPAN         0x3000u /* codepoint window pages draw their chars from */

/* Self-contained UTF-8 encoder */
static void tr_encode_utf8(char ** pout, uint32_t cp)
{
    char * p = *pout;
    if(cp < 0x80u) {
        *p++ = (char)cp;
    }
    else if(cp < 0x800u) {
        *p++ = (char)(0xC0u | (cp >> 6));
        *p++ = (char)(0x80u | (cp & 0x3Fu));
    }
    else {
        *p++ = (char)(0xE0u |  (cp >> 12));
        *p++ = (char)(0x80u | ((cp >> 6) & 0x3Fu));
        *p++ = (char)(0x80u |  (cp & 0x3Fu));
    }
    *pout = p;
}

/* Build TR_PAGE_LINES lines x TR_CHARS_PER_LINE CJK chars for a logical page.
 * Each page is offset in codepoint space so pages look different and the glyph
 * cache sees fresh chars across page turns (realistic translation/scroll load). */
static void tr_fill_page_text(char * buf, uint32_t page_id)
{
    char * p = buf;
    uint32_t page_off = (page_id * TR_PAGE_LINES * TR_CHARS_PER_LINE) % TR_CJK_SPAN;
    uint32_t k = 0;
    for(int line = 0; line < TR_PAGE_LINES; line++) {
        for(int c = 0; c < TR_CHARS_PER_LINE; c++) {
            uint32_t cp = TR_CJK_BASE + (page_off + k) % TR_CJK_SPAN;
            tr_encode_utf8(&p, cp);
            k++;
        }
        if(line < TR_PAGE_LINES - 1) {
            *p++ = '\n';
        }
    }
    *p = '\0';
}

/* Create a page label. */
static lv_obj_t * tr_page_label_create(lv_obj_t * parent, lv_font_t * font)
{
    lv_obj_t * page_label = lv_label_create(parent);
    /* full container width so LV_TEXT_ALIGN_CENTER can h-center each line
     * (LV_SIZE_CONTENT would shrink to the text width, leaving nothing to
     * center within). */
    lv_obj_set_width(page_label, lv_pct(100));
    lv_obj_set_height(page_label, TR_PAGE_HEIGHT);
    lv_label_set_long_mode(page_label, LV_LABEL_LONG_CLIP);
    lv_label_set_recolor(page_label, false);
    lv_obj_set_style_text_color(page_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_color(page_label, lv_color_hex(0x444444), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(page_label, lv_color_hex(0x0), LV_PART_SELECTED);
    lv_obj_set_style_text_font(page_label, font, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(page_label,
                                     TR_LINE_HEIGHT - lv_font_get_line_height(font),
                                     LV_PART_MAIN);
    lv_obj_set_style_text_align(page_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(page_label, "");
    return page_label;
}

/* Scenario state, shared between setup and the scroll lv_timer callback. */
typedef struct {
    lv_obj_t * cont;
    int32_t    scroll_y;
    uint32_t   window_start;
    char       buf[TR_TEXT_BUF_SIZE];
} tr_state_t;

static tr_state_t g_tr;

/* lv_timer callback: advance the scroll one step, recycle a label at the window
 * boundary (page turn). Rendering happens later in lv_timer_handler(). */
static void tr_scroll_timer_cb(lv_timer_t * timer)
{
    tr_state_t * st = (tr_state_t *)lv_timer_get_user_data(timer);

    st->scroll_y += TR_SCROLL_STEP_PX;

    /* window bottom reached (top line entered the 3rd page) -> switch next:
     * recycle child0 to index2, refill with the next page, re-anchor one page
     * up. */
    if(st->scroll_y >= 2 * TR_PAGE_HEIGHT) {
        lv_obj_t * lbl = lv_obj_get_child(st->cont, 0);
        tr_fill_page_text(st->buf, st->window_start + TR_WINDOW_PAGES);
        lv_label_set_text(lbl, st->buf);
        lv_obj_move_to_index(lbl, 2);
        lv_obj_update_layout(st->cont);
        st->window_start++;
        st->scroll_y -= TR_PAGE_HEIGHT;
    }

    lv_obj_scroll_to_y(st->cont, st->scroll_y, LV_ANIM_OFF);
}

/* Build the scroll UI and start the scroll timer, then return. Rendering is
 * driven by the native lv_timer_handler() loop in GuiTask; FPS is read from
 * LVGL's built-in perf monitor. */
void run_text_scroll_scenario(lv_font_t * font, const char * tag)
{
    if(!font) {
        LV_LOG_ERROR("[%s] font is NULL", tag);
        return;
    }

    lv_obj_clean(lv_screen_active());
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    /* --- scroll_container --- */
    lv_obj_t * cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont, TR_DISPLAY_WIDTH, TR_VIEWPORT_H);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLL_CHAIN | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    /* main axis START (stack pages top-down), cross axis CENTER (h-center the
     * content-width labels inside the container). */
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(cont, 0, LV_PART_MAIN);

    /* --- pre-create 3 page labels, fill window [0,1,2] --- */
    g_tr.cont         = cont;
    g_tr.scroll_y     = 0;
    g_tr.window_start = 0;
    for(uint32_t i = 0; i < TR_WINDOW_PAGES; i++) {
        lv_obj_t * lbl = tr_page_label_create(cont, font);
        tr_fill_page_text(g_tr.buf, g_tr.window_start + i);   /* set_text copies, buf reuse ok */
        lv_label_set_text(lbl, g_tr.buf);
    }
    lv_obj_update_layout(cont);
    lv_obj_scroll_to_y(cont, 0, LV_ANIM_OFF);

    LV_LOG_USER("[%s] start: font_line_h=%d line_space=%d viewport=%dx%d page_h=%d",
                tag, (int)lv_font_get_line_height(font),
                (int)(TR_LINE_HEIGHT - lv_font_get_line_height(font)),
                TR_DISPLAY_WIDTH, TR_VIEWPORT_H, TR_PAGE_HEIGHT);

    /* Drive the continuous scroll + page turn; rendering is the lv_timer_handler
     * loop's job, FPS comes from the built-in perf monitor. */
    lv_timer_create(tr_scroll_timer_cb, TR_SCROLL_PERIOD_MS, &g_tr);
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

    // === Continuous Text Scroll scenario, vector font ===
    lv_font_t * tr_font = lv_example_ambiq_ttf_create(22, true);
    run_text_scroll_scenario(tr_font, "TR-Vec");

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