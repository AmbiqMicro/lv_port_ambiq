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
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_sdk_4_5_0-a1ef3b89f9 of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "lvgl_test.h"
#include "lv_ambiq_font_align.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define DRAW_BUTTON 0
#define DRAW_STYLE_BUTTON 0
#define DRAW_BUTTON_IMAGE_BG 0
#define DRAW_BUTTON_IMAGE_BG_TILE 0
#define DRAW_SLIDER 0
#define DRAW_IMAGE  0
#define DRAW_LINE   0
#define DRAW_WATCHFACE  1
#define DRAW_LABEL  0



//Address of the textures in PSRAM
#if DRAW_IMAGE==1
#include "texture/phone_true_color.h"
#elif DRAW_WATCHFACE==1

//#define HEART_BEAT_COLOR_FORMAT_TRUE_COLOR_ALPHA
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_ARGB8888
#define HEART_BEAT_COLOR_FORMAT_AMBIQ_BGRA8888
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_RGB565
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_TSC6
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_RGB24
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_RGBA4444
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_TSC6A
//#define HEART_BEAT_COLOR_FORMAT_AMBIQ_TSC4
//#define HEART_BEAT_COLOR_FORMAT_INDEXED_1BIT
//#define HEART_BEAT_COLOR_FORMAT_INDEXED_2BIT
//#define HEART_BEAT_COLOR_FORMAT_INDEXED_4BIT
//#define HEART_BEAT_COLOR_FORMAT_INDEXED_8BIT
//#define HEART_BEAT_COLOR_FORMAT_ALPHA_1BIT
//#define HEART_BEAT_COLOR_FORMAT_ALPHA_2BIT
//#define HEART_BEAT_COLOR_FORMAT_ALPHA_4BIT
//#define HEART_BEAT_COLOR_FORMAT_ALPHA_8BIT

#if defined( HEART_BEAT_COLOR_FORMAT_TRUE_COLOR_ALPHA)
    #include "texture/heartbeat_true_color.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_TRUE_COLOR_ALPHA
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_ARGB8888)
    #include "texture/heartbeat_argb8888.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_ARGB8888
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_BGRA8888)
    #include "texture/heartbeat_bgra8888.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_BGRA8888
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_RGB565)
    #include "texture/heartbeat_rgba565.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_RGB565
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_TSC6)
    #include "texture/heartbeat_tsc6.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_TSC6
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_RGB24)
    #include "texture/heartbeat_rgb24.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_RGB24
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_RGBA4444)
    #include "texture/heartbeat_rgba4444.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_RGBA4444
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_TSC4)
    #include "texture/heartbeat_tsc4.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_TSC4
#elif defined( HEART_BEAT_COLOR_FORMAT_AMBIQ_TSC6A)
    #include "texture/heartbeat_tsc6a.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_AMBIQ_TSC6A
#elif defined( HEART_BEAT_COLOR_FORMAT_INDEXED_1BIT)
    #include "texture/heartbeat_indexed_1bits.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_INDEXED_1BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_INDEXED_2BIT)
    #include "texture/heartbeat_indexed_2bits.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_INDEXED_2BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_INDEXED_4BIT)
    #include "texture/heartbeat_indexed_4bits.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_INDEXED_4BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_INDEXED_8BIT)
    #include "texture/heartbeat_indexed_8bits.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_INDEXED_8BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_ALPHA_1BIT)
    #include "texture/heartbeat_l1.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_ALPHA_1BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_ALPHA_2BIT)
    #include "texture/heartbeat_l2.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_ALPHA_2BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_ALPHA_4BIT)
    #include "texture/heartbeat_l4.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_ALPHA_4BIT
#elif defined( HEART_BEAT_COLOR_FORMAT_ALPHA_8BIT)
    #include "texture/heartbeat_l8.h"
    #define HEART_BEAT_COLOR_FORMAT LV_IMG_CF_ALPHA_8BIT
#endif

#elif (DRAW_BUTTON_IMAGE_BG == 1)
#include "texture/color_wheel_300_300_argb8888.h"
#elif (DRAW_BUTTON_IMAGE_BG_TILE == 1)
#include "texture/brick_64_64_rgba565.h"
#include "texture/wood_50_50_argb8888.h"
#else
#endif
//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************
#if DRAW_IMAGE==1
LV_IMG_DECLARE(phone);
lv_img_dsc_t img_phone_argb_psram = {
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 100,
  .header.h = 100,
  .data_size = 10000 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = phone_map,
};

#elif DRAW_WATCHFACE==1

#ifdef HEART_BEAT_COLOR_FORMAT_TRUE_COLOR_ALPHA
LV_IMG_DECLARE(heartbeat);
lv_img_dsc_t img_watchface_psram = heartbeat;
lv_img_dsc_t img_heartbeat_psram = heartbeat;
#else
lv_img_dsc_t img_watchface_psram = {
  .header.cf = HEART_BEAT_COLOR_FORMAT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 388,
  .header.h = 388,
};

lv_img_dsc_t img_heartbeat_psram = {
  .header.cf = HEART_BEAT_COLOR_FORMAT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 388,
  .header.h = 388,
};
#endif

#elif (DRAW_BUTTON_IMAGE_BG == 1)
lv_img_dsc_t img_color_wheel_argb_psram = {
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 300,
  .header.h = 300,
  .data_size = 300*300 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = color_wheel_300_300_argb8888,
};

#elif (DRAW_BUTTON_IMAGE_BG_TILE == 1)
lv_img_dsc_t img_wood_argb_psram = {
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50*50* LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = wood_50_50_argb8888,
};

lv_img_dsc_t img_brick_rgb565_psram = {
  .header.cf = LV_IMG_CF_AMBIQ_RGB565,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 64,
  .header.h = 64,
  .data_size = 64*64* 2,
  .data = brick_64_64_rgba565,
};

#endif

#if DRAW_WATCHFACE==1
static lv_obj_t * img_watchface;
static lv_obj_t * img_heartbeat;
#endif

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
#if DRAW_IMAGE==1
    img_phone_argb_psram.data = (const uint8_t *)lv_mem_external_alloc(img_phone_argb_psram.data_size);
    memcpy((void*)img_phone_argb_psram.data, phone.data, img_phone_argb_psram.data_size);
#elif DRAW_WATCHFACE==1
#ifdef HEART_BEAT_COLOR_FORMAT_TRUE_COLOR_ALPHA
    img_watchface_psram.data = (const uint8_t *)lv_mem_external_alloc(heartbeat.data_size);
    img_heartbeat_psram.data = (const uint8_t *)lv_mem_external_alloc(heartbeat.data_size);
    memcpy((void*)img_watchface_psram.data, heartbeat.data, heartbeat.data_size);
    memcpy((void*)img_heartbeat_psram.data, heartbeat.data, heartbeat.data_size);
#else
    img_watchface_psram.data = (const uint8_t *)lv_mem_external_alloc(heartbeat_length);
    img_heartbeat_psram.data = (const uint8_t *)lv_mem_external_alloc(heartbeat_length);
    img_heartbeat_psram.data_size = heartbeat_length;
    img_watchface_psram.data_size = heartbeat_length;
    memcpy((void*)img_watchface_psram.data, heartbeat, heartbeat_length);
    memcpy((void*)img_heartbeat_psram.data, heartbeat, heartbeat_length);
#endif

#elif (DRAW_BUTTON_IMAGE_BG == 1)
    img_color_wheel_argb_psram.data = (const uint8_t *)lv_mem_external_alloc(img_color_wheel_argb_psram.data_size);

    memcpy((void*)img_color_wheel_argb_psram.data, color_wheel_300_300_argb8888, img_color_wheel_argb_psram.data_size);

#elif (DRAW_BUTTON_IMAGE_BG_TILE == 1)
    img_wood_argb_psram.data = (const uint8_t *)lv_mem_external_alloc(img_wood_argb_psram.data_size);
    memcpy((void*)img_wood_argb_psram.data, wood_50_50_argb8888, img_wood_argb_psram.data_size);

    img_brick_rgb565_psram.data = (const uint8_t *)lv_mem_external_alloc(img_brick_rgb565_psram.data_size);
    memcpy((void*)img_brick_rgb565_psram.data, brick_64_64_rgba565, img_brick_rgb565_psram.data_size);
#endif

    // Alloc buffer in PSRAM to hold the font data.
    uint32_t font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_14.dsc);
    void* font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    lv_font_fmt_txt_dsc_t* font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_14.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);
}

//*****************************************************************************
//
// GUI functions
//
//*****************************************************************************

#if DRAW_IMAGE == 1
static void set_angle(void * img, int32_t v)
{
    lv_img_set_angle(img, v);
}

static void set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom(img, v);
}
#endif

#if DRAW_WATCHFACE==1
static void ofs_x_anim(void * img, int32_t v)
{
    lv_obj_set_pos(img_watchface, v, 0);
    lv_obj_set_pos(img_heartbeat, v + 390, 0);
}
#endif

#if (DRAW_STYLE_BUTTON == 1) || (DRAW_BUTTON == 1)
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_btn_red;
#endif

#if DRAW_DARKEN == 1
static lv_color_t darken(const lv_color_filter_dsc_t * dsc, lv_color_t color, lv_opa_t opa)
{
    LV_UNUSED(dsc);
    return lv_color_darken(color, opa);
}
#endif

#if DRAW_STYLE_BUTTON == 1
static void style_init(void)
{
    /*Create a simple button style*/
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, 50);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn, lv_color_make(0x00, 0xff, 0x00));

    lv_style_set_border_color(&style_btn, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_border_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_border_width(&style_btn, 4);

    /*Add outline*/
    lv_style_set_outline_width(&style_btn, 4);
    lv_style_set_outline_color(&style_btn, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_outline_pad(&style_btn, 2);

    // /*Add a shadow*/
    // lv_style_set_shadow_width(&style, 25);
    // lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_shadow_ofs_x(&style, 10);
    // lv_style_set_shadow_ofs_y(&style, 20);

    lv_style_set_text_color(&style_btn, lv_color_black());
}
#endif

#if DRAW_BUTTON == 1
static void style_init(void)
{
    /*Create a simple button style*/
    lv_style_init(&style_btn);

    lv_style_set_radius(&style_btn, 50);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn, lv_palette_main(LV_PALETTE_BLUE));

    /*Add a shadow*/
    lv_style_set_shadow_width(&style_btn, 25);
    lv_style_set_shadow_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_shadow_ofs_x(&style_btn, 8);
    lv_style_set_shadow_ofs_y(&style_btn, 8);

    lv_style_set_text_color(&style_btn, lv_color_white());
}
#endif

void lv_ex_line_1(void)
{
    /*Create an array for the points of the line*/
    static lv_point_t line_points_1[] = { {5, 5}, {70, 70}, {120, 10}, {180, 60}, {240, 10} };

    static lv_point_t line_points_2[] = { {5, 10}, {70, 10}, {70, 40} };

    static lv_point_t line_points_3[] = { {5, 10}, {5, 40}, {70, 40} };   

    /*Create style*/
    static lv_style_t style_line_1;
    lv_style_init(&style_line_1);
    lv_style_set_line_width(&style_line_1, 8);
    lv_style_set_line_color(&style_line_1, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_rounded(&style_line_1, true);
    lv_style_set_line_dash_width(&style_line_1, 3);
    lv_style_set_line_dash_gap(&style_line_1, 2);

    static lv_style_t style_line_2;
    lv_style_init(&style_line_2);
    lv_style_set_line_width(&style_line_2, 8);
    lv_style_set_line_color(&style_line_2, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_rounded(&style_line_2, true);   

    /*Create a line and apply the new style*/
    lv_obj_t * line1;
    line1 = lv_line_create(lv_scr_act());
    lv_line_set_points(line1, line_points_1, 5);     /*Set the points*/
    lv_obj_add_style(line1, &style_line_1, 0);
    lv_obj_center(line1);

    lv_obj_t * line2;
    line2 = lv_line_create(lv_scr_act());
    lv_line_set_points(line2, line_points_2, 3);     /*Set the points*/
    lv_obj_add_style(line2, &style_line_2, 0);
    lv_obj_align(line2, LV_ALIGN_CENTER, 0, 40);

    lv_obj_t * line3;
    line3 = lv_line_create(lv_scr_act());
    lv_line_set_points(line3, line_points_3, 3);     /*Set the points*/
    lv_obj_add_style(line3, &style_line_1, 0);
    lv_obj_align(line3, LV_ALIGN_CENTER, 0, 80);   

    lv_obj_t * line4;
    line4 = lv_line_create(lv_scr_act());
    lv_line_set_points(line4, line_points_2, 3);     /*Set the points*/
    lv_obj_add_style(line4, &style_line_1, 0);
    lv_obj_align(line4, LV_ALIGN_CENTER, 100, 40);

    lv_obj_t * line5;
    line5 = lv_line_create(lv_scr_act());
    lv_line_set_points(line5, line_points_3, 3);     /*Set the points*/
    lv_obj_add_style(line5, &style_line_2, 0);
    lv_obj_align(line5, LV_ALIGN_CENTER, 100, 80);  
}



//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("GUI task start!\n");

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    // Load texture.
    texture_load();

#if DRAW_BUTTON == 1
    /*Initialize the style*/
    style_init();

    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_remove_style_all(btn);
    lv_obj_set_pos(btn, 80, 80);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/
    lv_obj_add_style(btn, &style_btn, 0);

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);

#elif DRAW_BUTTON_IMAGE_BG == 1
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, 200, 100);                          /*Set its size*/
    lv_obj_center(btn);

    lv_obj_set_style_bg_img_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_bg_img_src(btn, &img_color_wheel_argb_psram, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 25, 0);

#elif DRAW_BUTTON_IMAGE_BG_TILE == 1
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, 200, 100);                          /*Set its size*/
    lv_obj_set_pos(btn, 80, 80); 

    lv_obj_set_style_bg_img_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_bg_img_src(btn, &img_brick_rgb565_psram, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 25, 0);
    lv_obj_set_style_bg_img_tiled(btn, true, 0);

    lv_obj_t * btn_rect = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_remove_style_all(btn_rect);
    lv_obj_set_size(btn_rect, 200, 100);                          /*Set its size*/
    lv_obj_set_pos(btn_rect, 80, 200); 

    lv_obj_set_style_bg_img_opa(btn_rect, LV_OPA_50, 0);
    lv_obj_set_style_bg_img_src(btn_rect, &img_wood_argb_psram, LV_PART_MAIN);
    //lv_obj_set_style_radius(btn, 25, 0);
    lv_obj_set_style_bg_img_tiled(btn_rect, true, 0);

#elif DRAW_STYLE_BUTTON == 1
    /*Initialize the style*/
    style_init();

    /*Create a button and use the new styles*/
    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    /* Remove the styles coming from the theme
     * Note that size and position are also stored as style properties
     * so lv_obj_remove_style_all will remove the set size and position too */
    lv_obj_remove_style_all(btn);
    lv_obj_set_pos(btn, 100, 100);
    lv_obj_set_size(btn, 200, 100);
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);

    /*Add a label to the button*/
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
#elif DRAW_LABEL == 1

    lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
                      "and wrap long text automatically.");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t * label2 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(label2, 150);
    lv_label_set_text(label2, "It is a circularly scrolling text. ");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);

#elif DRAW_SLIDER == 1
    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_set_width(slider, 200);                          /*Set the width*/
    lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
    //lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/

    /*Create a label below the slider*/
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "0");
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
#elif DRAW_IMAGE == 1

    /*Now create the actual image*/
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_phone_argb_psram);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_pivot(img, 50, 50);    /*Rotate around the top left corner*/

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_values(&a, 0, 150);
    lv_anim_set_time(&a, 80);
    //lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&a, 80);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, set_zoom);
    lv_anim_set_values(&a, 256, 400);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_start(&a);

#elif DRAW_LINE == 1

    lv_ex_line_1();

#elif DRAW_WATCHFACE == 1

    /*Now create the actual image*/
    img_watchface = lv_img_create(lv_scr_act());
    lv_img_set_src(img_watchface, &img_watchface_psram);

    img_heartbeat = lv_img_create(lv_scr_act());
    lv_img_set_src(img_heartbeat, &img_heartbeat_psram);

    lv_obj_set_pos(img_watchface, 0, 0);
    lv_obj_set_pos(img_heartbeat, lv_obj_get_width(img_watchface), 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img_watchface);
    lv_anim_set_exec_cb(&a, ofs_x_anim);
    lv_anim_set_values(&a, 0, -390);
    lv_anim_set_time(&a, 500);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&a, 700);
    lv_anim_start(&a);

#endif

    while (1)
    {
        lv_timer_handler();

        vTaskDelay(5);
    }
}

