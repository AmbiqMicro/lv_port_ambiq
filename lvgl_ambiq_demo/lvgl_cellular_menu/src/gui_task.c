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

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "lvgl_cellular_menu.h"
#include "FreeRTOS.h"
#include <math.h>

#include "texture/buttom_argb8888.h"
#include "texture/b_02_argb8888.h"
#include "texture/b_03_argb8888.h"
#include "texture/b_04_argb8888.h"
#include "texture/b_05_argb8888.h"
#include "texture/b_06_argb8888.h"
#include "texture/b_07_argb8888.h"
#include "texture/b_08_argb8888.h"
#include "texture/b_09_argb8888.h"
#include "texture/b_10_argb8888.h"
#include "texture/b_11_argb8888.h"
#include "texture/b_12_argb8888.h"
#include "texture/b_13_argb8888.h"
#include "texture/b_14_argb8888.h"
#include "texture/b_15_argb8888.h"
#include "texture/b_16_argb8888.h"
#include "texture/b_17_argb8888.h"
#include "texture/b_18_argb8888.h"
#include "texture/b_19_argb8888.h"
#include "texture/b_20_argb8888.h"
#include "texture/b_21_argb8888.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

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

lv_image_dsc_t icon_A_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_B_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_C_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_D_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_E_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_F_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_G_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_H_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_I_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_J_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_K_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_L_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_M_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_N_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_O_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_P_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_Q_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_R_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};
lv_image_dsc_t icon_S_argb_psram = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_ARGB8888,
  .header.flags = 0,
  .header.w = 128,
  .header.h = 128,
  .header.stride = 128 * 4,
  .data_size = 128 * 128 * 4,
  .data = NULL,
};

void lv_ambiq_log_printf(lv_log_level_t level, const char * buf)
{
    g_pfnCharPrint((char *)buf);
}

//*****************************************************************************
//
// Cache-coherent memory copy helper function
// This function performs memcpy and ensures cache coherency for PSRAM writes
//
//*****************************************************************************
static inline void am_memcpy_cache_clean(void *dest, const void *src, size_t size)
{
    // Perform the memory copy
    memcpy(dest, src, size);
    
    // Clean the D-cache to ensure data coherency
    // This is essential when copying data to PSRAM that will be accessed by GPU
    am_hal_cachectrl_range_t Range;
    Range.ui32StartAddr = (uint32_t)dest;
    Range.ui32Size = size;
    am_hal_cachectrl_dcache_clean(&Range);
}

//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
    icon_A_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_A_argb_psram.data_size);
    icon_B_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_B_argb_psram.data_size);
    icon_C_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_C_argb_psram.data_size);
    icon_D_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_D_argb_psram.data_size);
    icon_E_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_E_argb_psram.data_size);
    icon_F_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_F_argb_psram.data_size);
    icon_G_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_G_argb_psram.data_size);
    icon_H_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_H_argb_psram.data_size);
    icon_I_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_I_argb_psram.data_size);
    icon_J_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_J_argb_psram.data_size);
    icon_K_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_K_argb_psram.data_size);
    icon_L_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_L_argb_psram.data_size);
    icon_M_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_M_argb_psram.data_size);
    icon_N_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_N_argb_psram.data_size);
    icon_O_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_O_argb_psram.data_size);
    icon_P_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_P_argb_psram.data_size);
    icon_Q_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_Q_argb_psram.data_size);
    icon_R_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_R_argb_psram.data_size);
    icon_S_argb_psram.data = (const uint8_t *)am_mem_psram_malloc(icon_S_argb_psram.data_size);


    am_memcpy_cache_clean((void*)icon_A_argb_psram.data, buttom_argb8888, icon_A_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_B_argb_psram.data, b_02_argb8888, icon_B_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_C_argb_psram.data, b_03_argb8888, icon_C_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_D_argb_psram.data, b_04_argb8888, icon_D_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_E_argb_psram.data, b_05_argb8888, icon_E_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_F_argb_psram.data, b_06_argb8888, icon_F_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_G_argb_psram.data, b_07_argb8888, icon_G_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_H_argb_psram.data, b_08_argb8888, icon_H_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_I_argb_psram.data, b_09_argb8888, icon_I_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_J_argb_psram.data, b_10_argb8888, icon_J_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_K_argb_psram.data, b_11_argb8888, icon_K_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_L_argb_psram.data, b_12_argb8888, icon_L_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_M_argb_psram.data, b_13_argb8888, icon_M_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_N_argb_psram.data, b_14_argb8888, icon_N_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_O_argb_psram.data, b_15_argb8888, icon_O_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_P_argb_psram.data, b_16_argb8888, icon_P_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_Q_argb_psram.data, b_21_argb8888, icon_Q_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_R_argb_psram.data, b_18_argb8888, icon_R_argb_psram.data_size);
    am_memcpy_cache_clean((void*)icon_S_argb_psram.data, b_19_argb8888, icon_S_argb_psram.data_size);

    // // Alloc buffer in PSRAM to hold the font data.
    // uint32_t font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_14.dsc);
    // void* font_buffer = lv_mem_external_alloc(font_length);

    // // Copy and align the font bitmap
    // lv_font_fmt_txt_dsc_t* font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_14.dsc;
    // lv_ambiq_A4_font_align(font, font_buffer);

}

steps_t g_steps;
vector_t g_vector;
icon_layout_t g_layout;
icons_t g_current;
icons_t g_temp_layout;

int init_steps(steps_t * steps)
{
    steps->total_step = 0;
    steps->flag = 0;

    for ( int i = 0; i < 3; i++ )
    {
        steps->step[i].inc_x = 0;
        steps->step[i].inc_y = 0;
        steps->total_step ++;
    }

    return steps->total_step;
}

int set_vector(vector_t * v, float direction, float speed, float distance)
{
    v->direction = direction;
    v->speed = speed;
    v->distance = distance;
    v->steps = (short)(distance / speed);

    return 0;
}

float sin_a(float a)
{
    float rtn, val;

    val = PI / 180;
    rtn = sin(a * val);
    return rtn;
}

float cos_a(float a)
{
    float rtn, val;

    val = PI / 180;
    rtn = sin(PI / 2 - a * val);
    return rtn;
}

int add_vector2steps(vector_t * v, steps_t * steps)
{
    if ((v->steps + steps->total_step) > STEP_MAX)
    {
        return -1;
    }

    float a;
    a = v->direction;

    for ( int i = 0 ; i < v->steps ; i ++ )
    {
        steps->step[steps->total_step + i].inc_x = (short)(cos_a(a) * (v->speed));
        steps->step[steps->total_step + i].inc_y = (short)(sin_a(a) * (v->speed));
    }
    steps->total_step = steps->total_step + v->steps;

    return steps->total_step;
}

int caculate_r (float d, int cycle)
{
    float r;
    float b;
    float k;

    b = (float)ICON_ZOOM_MAX;
    k = ((float)ICON_ZOOM_MIN - (float)ICON_ZOOM_MAX) / (float)ICON_REMOTE;
    r = k*d + b;

    switch(cycle)
    {
        case 0:
            r = r;
            break;
        case 1:
            r = r * R_CYCLE_FACTOR_1;
            if ( r > ICON_ZOOM_MAX )
            {
                r = ICON_ZOOM_MAX;
            }
            else if ( r < ICON_ZOOM_MIN )
            {
                r = ICON_ZOOM_MIN;
            }
            break;
        case 2:
            r = r * R_CYCLE_FACTOR_2;
            if ( r > ICON_ZOOM_MAX )
            {
                r = ICON_ZOOM_MAX;
            }
            else if ( r < ICON_ZOOM_MIN)
            {
                r = ICON_ZOOM_MIN;
            }
            break;
        case 3:
            r = r * R_CYCLE_FACTOR_3;
            /*if ( r > ICON_ZOOM_MAX)
            {
                r = ICON_ZOOM_MAX;
            }
            else if ( r <ICON_ZOOM_MIN)
            {
                r = ICON_ZOOM_MIN;
            }*/
            break;
        case 4:
            r = r * R_CYCLE_FACTOR_4;
            /*if ( r > ICON_ZOOM_MAX)
            {
                r = ICON_ZOOM_MAX;
            }
            else if ( r < ICON_ZOOM_MIN)
            {
                r = ICON_ZOOM_MIN;
            }*/
            break;
        default:
            r = r;
    }

    return (int)r;
}

void create_layout(icon_layout_t * layout, lv_obj_t * bg)
{
    int i = 0;
    memset(layout, 0, sizeof(layout));

    for ( i = 0; i < LAYOUT_ICON_MAX; i++ )
    {
        // ac_x
        if ((i == 45) || (i == 46) || (i == 47) || (i == 48) || (i == 49))
        {
            layout->Icon[i].ac_x = 4;
        } else if ((i == 44) || (i == 25) || (i == 26) || (i == 27) || (i == 28) || (i == 50))
        {
            layout->Icon[i].ac_x = 3;
        } else if ((i == 43) || (i == 24) || (i == 11) || (i == 12) || (i == 13) || (i == 29) || (i == 51))
        {
            layout->Icon[i].ac_x = 2;
        } else if ((i == 42) || (i == 23) || (i == 10) || (i == 3) || (i == 4) || (i == 14) || (i == 30) || (i == 52))
        {
            layout->Icon[i].ac_x = 1;
        } else if ((i == 41) || (i == 22) || (i == 9) || (i == 2) || (i == 0) || (i == 5) || (i == 15) || (i == 31) || (i == 53))
        {
            layout->Icon[i].ac_x = 0;
        } else if ((i == 40) || (i == 21) || (i == 8) || (i == 1) || (i == 6) || (i == 16) || (i == 32) || (i == 54))
        {
            layout->Icon[i].ac_x = -1;
        } else if ((i == 39) || (i == 20) || (i == 7) || (i == 18) || (i == 17) || (i == 33) || (i == 55))
        {
            layout->Icon[i].ac_x = (-2);
        } else if ((i == 38) || (i == 19) || (i == 36) || (i == 35) || (i == 34) || (i == 56))
        {
            layout->Icon[i].ac_x = -3;
        } else if ((i == 37) || (i == 60) || (i == 59) || (i == 58) || (i == 57))
        {
            layout->Icon[i].ac_x = -4;
        }

        // ac_y
        if ((i == 41) || (i == 42) || (i == 43) || (i == 44) || (i == 45))
        {
            layout->Icon[i].ac_y = -4;
        } else if ((i == 40) || (i == 22) || (i == 23) || (i == 24) || (i == 25) || (i == 46))
        {
            layout->Icon[i].ac_y = -3;
        } else if ((i == 39) || (i == 21) || (i == 9) || (i == 10) || (i == 11) || (i == 26) || (i == 47))
        {
            layout->Icon[i].ac_y = (-2);
        } else if ((i == 38) || (i == 20) || (i == 8) || (i == 2) || (i == 3) || (i == 12) || (i == 27) || (i == 48))
        {
            layout->Icon[i].ac_y = -1;
        } else if ((i == 37) || (i == 19) || (i == 7) || (i == 1) || (i == 0) || (i == 4) || (i == 13) || (i == 28) || (i == 49))
        {
            layout->Icon[i].ac_y = 0;
        } else if ((i == 60) || (i == 36) || (i == 18) || (i == 6) || (i == 5) || (i == 14) || (i == 29) || (i == 50))
        {
            layout->Icon[i].ac_y = 1;
        } else if ((i == 59) || (i == 35) || (i == 17) || (i == 16) || (i == 15) || (i == 30) || (i == 51))
        {
            layout->Icon[i].ac_y = 2;
        } else if ((i == 58) || (i == 34) || (i == 33) || (i == 32) || (i == 31) || (i == 52))
        {
            layout->Icon[i].ac_y = 3;
        } else if ((i == 57) || (i == 56) || (i == 55) || (i == 54) || (i == 53))
        {
            layout->Icon[i].ac_y = 4;
        }

        //ac_z
        layout->Icon[i].ac_z = -(layout->Icon[i].ac_x + layout->Icon[i].ac_y);

        //cycle
        int max_ac_xyz;
        max_ac_xyz = abs(layout->Icon[i].ac_x);
        if (max_ac_xyz<abs(layout->Icon[i].ac_y))
        {
            max_ac_xyz = abs(layout->Icon[i].ac_y);
        }
        if (max_ac_xyz<abs(layout->Icon[i].ac_z))
        {
            max_ac_xyz = abs(layout->Icon[i].ac_z);
        }
        layout->Icon[i].cycle = max_ac_xyz;

        //x
        int tacx;
        int tacy;
        float tempx;
        tacx = layout->Icon[i].ac_x;
        tacy = layout->Icon[i].ac_y;
        tempx = 0;
        if ((tacx == (-4)) && (tacy == 0))
        { //i = 37
            layout->Icon[i].origin_x = tempx - ICON_D * 4;
        } else if (((tacx == (-3)) && (tacy == (-1))) || ((tacx == (-4)) && (tacy == 1)))
        { //i = 38,60
            layout->Icon[i].origin_x = tempx - ICON_D * 7 / 2;
        } else if (((tacx == (-2)) && (tacy == (-2))) || ((tacx == (-3)) && (tacy == 0)) || ((tacx == (-4)) && (tacy == 2)))
        { //i = 39,19,59
            layout->Icon[i].origin_x = tempx - ICON_D * 3;
        } else if (((tacx == (-1)) && (tacy == (-3))) || ((tacx == (-2)) && (tacy == (-1))) || ((tacx == (-3)) && (tacy == 1)) || ((tacx == (-4)) && (tacy == 3)))
        { //i = 40,20,36,58
            layout->Icon[i].origin_x = tempx - ICON_D * 5 / 2;
        } else if (((tacx == 0) && (tacy == (-4))) || ((tacx == (-1)) && (tacy == (-2))) || ((tacx == (-2)) && (tacy == 0)) || ((tacx == (-3)) && (tacy == 2)) || ((tacx == (-4)) && (tacy == 4)))
        { //i = 41,21,7,35,57
            layout->Icon[i].origin_x = tempx - ICON_D * 2;
        } else if (((tacx == 0) && (tacy == (-3))) || ((tacx == (-1)) && (tacy == (-1))) || ((tacx == (-2)) && (tacy == 1)) || ((tacx == (-3)) && (tacy == 3)))
        { //i = 22,8,18,34
            layout->Icon[i].origin_x = tempx - ICON_D * 3 / 2;
        } else if (((tacx == 1) && (tacy == (-4))) || ((tacx == 0) && (tacy == (-2))) || ((tacx == (-1)) && (tacy == 0)) || ((tacx == (-2)) && (tacy == 2)) || ((tacx == (-3)) && (tacy == 4)))
        { //i = 42,9,1,17,56
            layout->Icon[i].origin_x = tempx - ICON_D;
        } else if (((tacx == 1) && (tacy == (-3))) || ((tacx == 0) && (tacy == (-1))) || ((tacx == (-1)) && (tacy == 1)) || ((tacx == (-2)) && (tacy == 3)))
        { //i = 23,2,6,33
            layout->Icon[i].origin_x = tempx - ICON_D / 2;
        } else if (((tacx == 2) && (tacy == (-4))) || ((tacx == 1) && (tacy == (-2))) || ((tacx == 0) && (tacy == 0)) || ((tacx == (-1)) && (tacy == 2)) || ((tacx == (-2)) && (tacy == 4)))
        { //i = 43,10,0,16,55
            layout->Icon[i].origin_x = 0;
        } else if (((tacx == 2) && (tacy == (-3))) || ((tacx == 1) && (tacy == (-1))) || ((tacx == 0) && (tacy == 1)) || ((tacx == (-1)) && (tacy == 3)))
        { //i = 24,3,5,32
            layout->Icon[i].origin_x = tempx + ICON_D / 2;
        } else if (((tacx == 3) && (tacy == (-4))) || ((tacx == 2) && (tacy == (-2))) || ((tacx == 1) && (tacy == 0)) || ((tacx == 0) && (tacy == 2)) || ((tacx == (-1)) && (tacy == 4)))
        { //i = 44,11,4,15,54
            layout->Icon[i].origin_x = tempx + ICON_D;
        } else if (((tacx == 3) && (tacy == (-3))) || ((tacx == 2) && (tacy == (-1))) || ((tacx == 1) && (tacy == 1)) || ((tacx == 0) && (tacy == 3)))
        { //i = 25,12,14,31
            layout->Icon[i].origin_x = tempx + ICON_D * 3 / 2;
        } else if (((tacx == 4) && (tacy == (-4))) || ((tacx == 3) && (tacy == (-2))) || ((tacx == 2) && (tacy == 0)) || ((tacx == 1) && (tacy == 2)) || ((tacx == 0) && (tacy == 4)))
        { //i = 45,26,13,30,53
            layout->Icon[i].origin_x = tempx + ICON_D * 2;
        } else if (((tacx == 4) && (tacy == (-3))) || ((tacx == 3) && (tacy == (-1))) || ((tacx == 2) && (tacy == 1)) || ((tacx == 1) && (tacy == 3)))
        { //i = 46,27,29,52
            layout->Icon[i].origin_x = tempx + ICON_D * 5 / 2;
        } else if (((tacx == 4) && (tacy == (-2))) || ((tacx == 3) && (tacy == 0)) || ((tacx == 2) && (tacy == 2)))
        { //i = 47,28,51
            layout->Icon[i].origin_x = tempx + ICON_D*3;
        } else if (((tacx == 4) && (tacy == (-1))) || ((tacx == 3) && (tacy == 1)))
        { //i = 48,50
            layout->Icon[i].origin_x = tempx + ICON_D * 7 / 2;
        } else if ((tacx == 4) && (tacy == 0))
        { //i = 49
            layout->Icon[i].origin_x = tempx + ICON_D*4;
        }

        if (layout->Icon[i].cycle == 0)
        {
            layout->Icon[i].origin_x = layout->Icon[i].origin_x;
        } else if (layout->Icon[i].cycle == 1)
        {
            layout->Icon[i].origin_x = layout->Icon[i].origin_x * CYC_1_FACTOR / CYC_0_FACTOR;
        } else if (layout->Icon[i].cycle == 2)
        {
            //if ((i%2) == 0){
                layout->Icon[i].origin_x = layout->Icon[i].origin_x * CYC_2_FACTOR_1 / CYC_0_FACTOR;
            //} else{
                //layout->Icon[i].origin_x = layout->Icon[i].origin_x * CYC_2_FACTOR_2 / CYC_0_FACTOR;
            //}
        } else if (layout->Icon[i].cycle == 3)
        {
            layout->Icon[i].origin_x = layout->Icon[i].origin_x * CYC_3_FACTOR / CYC_0_FACTOR;
        } else
        { //4
            layout->Icon[i].origin_x = layout->Icon[i].origin_x * CYC_4_FACTOR / CYC_0_FACTOR;
        }

        //y
        layout->Icon[i].origin_y = (float)(layout->Icon[i].ac_y) * ICON_D * ROOT3 / 2;

        if (layout->Icon[i].cycle == 0)
        {
            layout->Icon[i].origin_y = layout->Icon[i].origin_y;
        } else if (layout->Icon[i].cycle == 1)
        {
            layout->Icon[i].origin_y = layout->Icon[i].origin_y * CYC_1_FACTOR / CYC_0_FACTOR;
        } else if (layout->Icon[i].cycle == 2)
        {
            //if ((i%2) == 0){
                layout->Icon[i].origin_y = layout->Icon[i].origin_y * CYC_2_FACTOR_1 / CYC_0_FACTOR;
            //} else{
                //layout->Icon[i].origin_y = layout->Icon[i].origin_y * CYC_2_FACTOR_2 / CYC_0_FACTOR;
            //}
        } else if (layout->Icon[i].cycle == 3)
        {
            layout->Icon[i].origin_y = layout->Icon[i].origin_y * CYC_3_FACTOR / CYC_0_FACTOR;
        } else
        { //4
            layout->Icon[i].origin_y = layout->Icon[i].origin_y * CYC_4_FACTOR / CYC_0_FACTOR;
        }

        //distance
        if (layout->Icon[i].cycle == 0)
        {
            layout->Icon[i].distance = 0;
        } else if (layout->Icon[i].cycle == 1)
        {
            layout->Icon[i].distance = ICON_D * CYC_1_FACTOR / CYC_0_FACTOR;
        } else if (layout->Icon[i].cycle == 2)
        {
            //if ((i%2) == 0){
                layout->Icon[i].distance = ROOT3 * ICON_D * CYC_2_FACTOR_1 / CYC_0_FACTOR;
            //} else{
                //layout->Icon[i].distance = 2 * ICON_D * CYC_2_FACTOR_2 / CYC_0_FACTOR;
            //}
        } else if (layout->Icon[i].cycle == 3)
        {
            layout->Icon[i].distance = 3 * ICON_D * CYC_3_FACTOR / CYC_0_FACTOR;
        } else if (layout->Icon[i].cycle == 4)
        {
            layout->Icon[i].distance = 4 * ICON_D * CYC_4_FACTOR / CYC_0_FACTOR;
        }

        //r
        layout->Icon[i].r = caculate_r(layout->Icon[i].distance, layout->Icon[i].cycle);

        //icon pointer
        if (i<ICON_MAX)
        {
            layout->Icon[i].icon = lv_image_create(bg);
        }
    }

    lv_image_set_src(layout->Icon[0].icon, &icon_A_argb_psram);
    lv_image_set_src(layout->Icon[1].icon, &icon_B_argb_psram);
    lv_image_set_src(layout->Icon[2].icon, &icon_C_argb_psram);
    lv_image_set_src(layout->Icon[3].icon, &icon_D_argb_psram);
    lv_image_set_src(layout->Icon[4].icon, &icon_E_argb_psram);
    lv_image_set_src(layout->Icon[5].icon, &icon_F_argb_psram);
    lv_image_set_src(layout->Icon[6].icon, &icon_G_argb_psram);
    lv_image_set_src(layout->Icon[7].icon, &icon_H_argb_psram);
    lv_image_set_src(layout->Icon[8].icon, &icon_I_argb_psram);
    lv_image_set_src(layout->Icon[9].icon, &icon_J_argb_psram);
    lv_image_set_src(layout->Icon[10].icon, &icon_K_argb_psram);
    lv_image_set_src(layout->Icon[11].icon, &icon_L_argb_psram);
    lv_image_set_src(layout->Icon[12].icon, &icon_M_argb_psram);
    lv_image_set_src(layout->Icon[13].icon, &icon_N_argb_psram);
    lv_image_set_src(layout->Icon[14].icon, &icon_O_argb_psram);
    lv_image_set_src(layout->Icon[15].icon, &icon_P_argb_psram);
    lv_image_set_src(layout->Icon[16].icon, &icon_Q_argb_psram);
    lv_image_set_src(layout->Icon[17].icon, &icon_R_argb_psram);
    lv_image_set_src(layout->Icon[18].icon, &icon_S_argb_psram);
}

int add_loop(vector_t * v, float angle, float speed, float distance)
{
    if (distance > 200)
    {
        distance = 200;
    }
    float loop_angle;
    loop_angle = ((int)(angle + 180)) % 360;

    set_vector(v, angle, speed, distance);
    add_vector2steps(&g_vector, &g_steps);
    set_vector(v, loop_angle, speed, distance);
    add_vector2steps(&g_vector, &g_steps);

    return g_steps.total_step;
}

int c_menu_test1_draw(icons_t * layout_current)
{
    for ( int i = 0; i < ICON_MAX; i++ )
    {
        lv_obj_align(layout_current->Icon[i].icon, LV_ALIGN_CENTER, (lv_coord_t)(layout_current->Icon[i].origin_x), (lv_coord_t)(layout_current->Icon[i].origin_y));
        lv_image_set_scale(layout_current->Icon[i].icon, layout_current->Icon[i].r);
    }
    return 0;
}

int c_menu_test1_anim_init(lv_obj_t * bg)
{
    int total_step;
    total_step = init_steps(&g_steps);

    add_loop(&g_vector, 22, 10, 300);
    add_loop(&g_vector, 110, 10, 300);

    add_loop(&g_vector, 10, 30, 300);
    add_loop(&g_vector, 22, 30, 300);
    add_loop(&g_vector, 229, 30, 300);
    add_loop(&g_vector, 138, 30, 300);
    add_loop(&g_vector, 40, 30, 300);
    add_loop(&g_vector, 350, 30, 300);
    add_loop(&g_vector, 110, 30, 300);

    add_loop(&g_vector, 22, 10, 300);
    add_loop(&g_vector, 110, 10, 300);

    add_loop(&g_vector, 10, 40, 300);
    add_loop(&g_vector, 22, 40, 300);
    add_loop(&g_vector, 229, 40, 300);
    add_loop(&g_vector, 138, 40, 300);
    add_loop(&g_vector, 40, 40, 300);
    add_loop(&g_vector, 350, 40, 300);
    add_loop(&g_vector, 110, 40, 300);

    add_loop(&g_vector, 22, 10, 300);
    add_loop(&g_vector, 110, 10, 300);

    add_loop(&g_vector, 10, 50, 300);
    add_loop(&g_vector, 22, 50, 300);
    add_loop(&g_vector, 229, 50, 300);
    add_loop(&g_vector, 138, 50, 300);
    add_loop(&g_vector, 40, 50, 300);
    add_loop(&g_vector, 350, 50, 300);
    add_loop(&g_vector, 110, 50, 300);

    add_loop(&g_vector, 22, 10, 300);
    add_loop(&g_vector, 229, 10, 300);
    add_loop(&g_vector, 40, 10, 300);
    add_loop(&g_vector, 110, 10, 300);

    add_loop(&g_vector, 10, 2, 200);
    add_loop(&g_vector, 22, 2, 200);
    add_loop(&g_vector, 229, 2, 200);
    add_loop(&g_vector, 138, 2, 200);
    add_loop(&g_vector, 40, 2, 200);
    add_loop(&g_vector, 350, 2, 200);
    add_loop(&g_vector, 110, 2, 200);

    return total_step;
}

float getS(coordinate_t * p_pA, coordinate_t * p_pB, coordinate_t * p_pC)
{
    float x1, y1, x2, y2, x3, y3;
    float S;

    x1 = p_pA->x;
    y1 = p_pA->y;
    x2 = p_pB->x;
    y2 = p_pB->y;
    x3 = p_pC->x;
    y3 = p_pC->y;

    S = (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2;

    return S;
}

int isInTriangle(coordinate_t * p_pA, coordinate_t * p_pB, coordinate_t * p_pC, coordinate_t * p_pP)
{
    float S, Sa, Sb, Sc, Ssum;

    S = getS(p_pA, p_pB, p_pC);
    Sa = getS(p_pP, p_pB, p_pC);
    Sb = getS(p_pA, p_pP, p_pC);
    Sc = getS(p_pA, p_pB, p_pP);
    //Ssum = abs(Sa) + abs(Sb) + abs(Sc);

    float Ssum2;
    Ssum2 = 0;
    if (Sa >= 0)
    {
        Ssum2 = Ssum2 + Sa;
    }
    else
    {
        Ssum2 = Ssum2 - Sa;
    }

    if (Sb >= 0)
    {
        Ssum2 = Ssum2 + Sb;
    }
    else
    {
        Ssum2 = Ssum2 - Sb;
    }

    if (Sc >= 0)
    {
        Ssum2 = Ssum2 + Sc;
    }
    else
    {
        Ssum2 = Ssum2 - Sc;
    }

    Ssum = Ssum2;

    S = fabs(S);

    if (fabs(Ssum - S) < 2)
    {
        Ssum = S;
    }

    if (S == Ssum)
    {
        return 1;
    } else if (Ssum > S)
    {
        return 0;
    } else
    {
        while(1);
        return -1;
    }
}

int getFactors(coordinate_t * p_pA, coordinate_t * p_pB, coordinate_t * p_pC, coordinate_t * p_pP, factor_t * p_factor)
{
    float S, Sa, Sb, Sc;
    float sum_factor;

    S = getS(p_pA, p_pB, p_pC);
    Sa = getS(p_pP, p_pB, p_pC);
    Sb = getS(p_pA, p_pP, p_pC);
    Sc = getS(p_pA, p_pB, p_pP);

    p_factor->a = Sa / S;
    p_factor->b = Sb / S;
    p_factor->c = Sc / S;
    sum_factor = p_factor->a + p_factor->b + p_factor->c;

    return 0;
}

int getPpoint(rb_icon_t * p_pA, rb_icon_t * p_pB, rb_icon_t * p_pC, factor_t * p_factor, rb_icon_t * p_pP)
{
    float x1, y1, x2, y2, x3, y3;
    int   r1, r2, r3;
    float fa, fb, fc;
    float j1, k1, m1, j2, k2, m2;

    x1 = p_pA->x;
    y1 = p_pA->y;
    x2 = p_pB->x;
    y2 = p_pB->y;
    x3 = p_pC->x;
    y3 = p_pC->y;
    fa = p_factor->a;
    fb = p_factor->b;
    fc = p_factor->c;

    j1 = x3 - x2;
    k1 = y2 - y3;
    m1 = (fa - 1) * (x2 * y3 - x3 * y2) + fa * (x1 * y2 + x3 * y1 - x1 * y3 - x2 * y1);
    j2 = x1 - x3;
    k2 = y3 - y1;
    m2 = (fb - 1) * (x3 * y1 - x1 * y3) + fb * (x1 * y2 + x2 * y3 - x2 * y1 - x3 * y2);

    if (k1 == 0)
    {
        k1 = k1 + 0.00001;
    }
    if (j1 == 0)
    {
        j1 = j1 + 0.00001;
    }

    p_pP->x = (m2 - j2 / j1 * m1) / (k2 - j2 / j1 * k1);
    p_pP->y = (m2 - k2 / k1 * m1) / (j2 - k2 / k1 * j1);

    r1 = p_pA->r;
    r2 = p_pB->r;
    r3 = p_pC->r;
    p_pP->r = fa*r1 + fb*r2 + fc*r3;

    return 0;
}

log_t g_log;

coordinate_t point_A = {
    .x = 0,
    .y = 0,
};
coordinate_t point_B = {
    .x = 0,
    .y = 0,
};
coordinate_t point_C = {
    .x = 0,
    .y = 0,
};
coordinate_t point_D = {
    .x = 0,
    .y = 0,
};
coordinate_t point_Temp = {
    .x = 0,
    .y = 0,
};

coordinate_t point_O = {
    .x = 0,
    .y = 0,
};

rb_icon_t lp_A = { //layout_point_A
    .x = 0,
    .y = 0,
    .r = 0,
};
rb_icon_t lp_B = { //layout_point_B
    .x = 0,
    .y = 0,
    .r = 0,
};
rb_icon_t lp_C = { //layout_point_C
    .x = 0,
    .y = 0,
    .r = 0,
};

rb_icon_t lp_T = { //layout_point_temp
    .x = 0,
    .y = 0,
    .r = 0,
};

factor_t f = {
    .a = 0,
    .b = 0,
    .c = 0,
};

icon_layout_t layout_A;
icon_layout_t layout_B;
icon_layout_t layout_C;

#if defined(ANIMATION)
static void cell_cb(void * img, int32_t step)//step = 0 <--> g_steps.total_step
{
    if (g_steps.step_counter < g_steps.total_step)
    {
        c_menu_test1_anim_loop(g_steps.step_counter);
        g_steps.step_counter ++;
    }
    else
    {
        g_steps.step_counter = 0;
    }
}
#else
void calc_step(float* x, float* y)
{
    float dis_x = *x - LV_AMBIQ_DISPLAY_BUFFER_RESX / 2;
    float dis_y = *y - LV_AMBIQ_DISPLAY_BUFFER_RESY / 2;
    float hypotenuse = sqrt(pow(dis_x, 2) + pow(dis_y, 2));
    *x = 30 * dis_x / hypotenuse;
    *y = 30 * dis_y / hypotenuse;
}
#endif

#if defined(ANIMATION)
int c_menu_test1_anim_loop(int step)
#else
int c_menu_test1_anim_loop(float x, float y)
#endif
{
    int i, j;
    float min_d_0, min_d_1, min_d_2, min_d_3;
    int cii_0, cii_1, cii_2, cii_3, cii_temp;
    float S, Sa, Sb, Sc, Ssum, sum_factor;
    int is_in_triangle_1, is_in_triangle_2;
    float inc_x, inc_y;

#if defined(ANIMATION)
    if (step > g_steps.total_step)
    {
        return -1;
    }
    inc_x = g_steps.step[step].inc_x;
    inc_y = g_steps.step[step].inc_y;
#else
    calc_step(&x, &y);
    inc_x = x;
    inc_y = y;
#endif

    memcpy(&g_temp_layout, &g_current, sizeof(icons_t));

    for ( i = 0; i < ICON_MAX; i++ )
    {
        g_temp_layout.Icon[i].origin_x += inc_x;
        g_temp_layout.Icon[i].origin_y += inc_y;
    }

    for ( i = 0; i < ICON_MAX; i++ )
    {
        g_temp_layout.Icon[i].distance = g_temp_layout.Icon[i].origin_x * g_temp_layout.Icon[i].origin_x + g_temp_layout.Icon[i].origin_y * g_temp_layout.Icon[i].origin_y;
        if (fabs(g_temp_layout.Icon[i].distance) < 1)
        {
            g_temp_layout.Icon[i].distance = 0;
        } else
        {
            g_temp_layout.Icon[i].distance = sqrt(g_temp_layout.Icon[i].distance);
        }
    }

    min_d_0 = g_temp_layout.Icon[0].distance;
    cii_0 = 0;
    for ( i = 1; i < ICON_MAX; i++ )
    {
        if (g_temp_layout.Icon[i].distance < min_d_0)
        {
            min_d_0 = g_temp_layout.Icon[i].distance;
            cii_0 = i;
        }
    }
    cii_1 = 0;
    for ( i = 0; i < ICON_MAX; i++ )
    {
        if (cii_1 == cii_0)
        {
            cii_1 ++;
        } else
        {
            break;
        }
    }
    min_d_1 = g_temp_layout.Icon[cii_1].distance;
    for ( i = 0; i < ICON_MAX; i++ )
    {
        if (i != cii_0)
        {
            if (g_temp_layout.Icon[i].distance < min_d_1)
            {
                min_d_1 = g_temp_layout.Icon[i].distance;
                cii_1 = i;
            }
        }
    }
    cii_2 = 0;
    for ( i = 0; i < ICON_MAX; i++ )
    {
        if ((cii_2 == cii_0) || (cii_2 == cii_1))
        {
            cii_2 ++;
        } else
        {
            break;
        }
    }
    min_d_2 = g_temp_layout.Icon[cii_2].distance;
    for ( i = 0; i < ICON_MAX; i++ )
    {
        if ((i != cii_0) && (i != cii_1))
        {
            if (g_temp_layout.Icon[i].distance < min_d_2)
            {
                min_d_2 = g_temp_layout.Icon[i].distance;
                cii_2 = i;
            }
        }
    }
    cii_3 = 0;
    for ( i = 0; i < ICON_MAX; i++ )
    {
        if ((cii_3 == cii_0) || (cii_3 == cii_1) || (cii_3 == cii_2))
        {
            cii_3 ++;
        }
        else
        {
            break;
        }
    }
    min_d_3 = g_temp_layout.Icon[cii_3].distance;
    for ( i = 0; i < ICON_MAX; i++ )
    {
        if ((i != cii_0) && (i != cii_1) && (i != cii_2))
        {
            if (g_temp_layout.Icon[i].distance < min_d_3)
            {
                min_d_3 = g_temp_layout.Icon[i].distance;
                cii_3 = i;
            }
        }
    }

    g_log.min_d_0 = min_d_0;
    g_log.min_d_1 = min_d_1;
    g_log.min_d_2 = min_d_2;
    g_log.min_d_3 = min_d_3;
    g_log.cii_0 = cii_0;
    g_log.cii_1 = cii_1;
    g_log.cii_2 = cii_2;
    g_log.cii_3 = cii_3;

    int triangle_case = -100;

    point_A.x = g_temp_layout.Icon[cii_0].origin_x;
    point_A.y = g_temp_layout.Icon[cii_0].origin_y;
    point_B.x = g_temp_layout.Icon[cii_1].origin_x;
    point_B.y = g_temp_layout.Icon[cii_1].origin_y;
    point_C.x = g_temp_layout.Icon[cii_2].origin_x;
    point_C.y = g_temp_layout.Icon[cii_2].origin_y;
    point_D.x = g_temp_layout.Icon[cii_3].origin_x;
    point_D.y = g_temp_layout.Icon[cii_3].origin_y;

    is_in_triangle_1 = isInTriangle(&point_A, &point_B, &point_C, &point_O);
    is_in_triangle_2 = isInTriangle(&point_A, &point_B, &point_D, &point_O);
    if (is_in_triangle_1 == 1)
    {
        ;
    } else if (is_in_triangle_2 == 1)
    {
        cii_temp = cii_2;
        cii_2 = cii_3;
        cii_3 = cii_temp;
        point_Temp.x = point_C.x;
        point_C.x = point_D.x;
        point_D.x = point_Temp.x;
        point_Temp.y = point_C.y;
        point_C.y = point_D.y;
        point_D.y = point_Temp.y;
    } else if ((is_in_triangle_1 == -1) || (is_in_triangle_2 == -1))
    {
        while(1);
        return -1;
    }
    else
    {
        triangle_case = -1;
    }

    if (fabs(min_d_0) <= PROXIMITY)
    {
        triangle_case = 1;
    }
    else
    {
        triangle_case = 3;
    }

    if ((triangle_case == 2) || (triangle_case == 3))
    {
        getFactors(&point_A, &point_B, &point_C, &point_O, &f);
        memcpy(&layout_A, &g_layout, sizeof(icon_layout_t));
        memcpy(&layout_B, &g_layout, sizeof(icon_layout_t));
        memcpy(&layout_C, &g_layout, sizeof(icon_layout_t));
        for ( i = 0; i < LAYOUT_ICON_MAX; i++ )
        {
            layout_A.Icon[i].ac_x += g_temp_layout.Icon[cii_0].ac_x;
            layout_A.Icon[i].ac_y += g_temp_layout.Icon[cii_0].ac_y;
            layout_A.Icon[i].ac_z += g_temp_layout.Icon[cii_0].ac_z;
            layout_B.Icon[i].ac_x += g_temp_layout.Icon[cii_1].ac_x;
            layout_B.Icon[i].ac_y += g_temp_layout.Icon[cii_1].ac_y;
            layout_B.Icon[i].ac_z += g_temp_layout.Icon[cii_1].ac_z;
            layout_C.Icon[i].ac_x += g_temp_layout.Icon[cii_2].ac_x;
            layout_C.Icon[i].ac_y += g_temp_layout.Icon[cii_2].ac_y;
            layout_C.Icon[i].ac_z += g_temp_layout.Icon[cii_2].ac_z;
        }

        for ( i = 0; i < ICON_MAX; i++ )
        {
            for ( j = 0; j < LAYOUT_ICON_MAX; j++ )
            {
                if ((g_current.Icon[i].ac_x == layout_A.Icon[j].ac_x) && (g_current.Icon[i].ac_y == layout_A.Icon[j].ac_y))
                {
                    lp_A.x = layout_A.Icon[j].origin_x;
                    lp_A.y = layout_A.Icon[j].origin_y;
                    lp_A.r = layout_A.Icon[j].r;
                }
                if ((g_current.Icon[i].ac_x == layout_B.Icon[j].ac_x) && (g_current.Icon[i].ac_y == layout_B.Icon[j].ac_y))
                {
                    lp_B.x = layout_B.Icon[j].origin_x;
                    lp_B.y = layout_B.Icon[j].origin_y;
                    lp_B.r = layout_B.Icon[j].r;
                }
                if ((g_current.Icon[i].ac_x == layout_C.Icon[j].ac_x) && (g_current.Icon[i].ac_y == layout_C.Icon[j].ac_y))
                {
                    lp_C.x = layout_C.Icon[j].origin_x;
                    lp_C.y = layout_C.Icon[j].origin_y;
                    lp_C.r = layout_C.Icon[j].r;
                }
            }
            getPpoint(&lp_A, &lp_B, &lp_C, &f, &lp_T);
            if (isnan(lp_T.x))
            {
                while(1);
            }
            if (isnan(lp_T.y))
            {
                while(1);
            }
            g_current.Icon[i].origin_x = lp_T.x;
            g_current.Icon[i].origin_y = lp_T.y;
            g_current.Icon[i].r = lp_T.r;
        }

        c_menu_test1_draw(&g_current);
    }

    return 0;
}

// Task
void
c_menu_test1_task(void)
{
    int total_step;

    // create a background without scrollbars.
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_bg_color(&style, lv_color_make(0x00, 0x00, 0x00));

    lv_obj_t * bg = lv_screen_active();
    lv_obj_add_style(bg, &style, 0);
    lv_obj_set_size(bg, (LV_AMBIQ_DISPLAY_BUFFER_RESX + 8 + 40), (LV_AMBIQ_DISPLAY_BUFFER_RESY + 8 + 120));
    lv_obj_center(bg);
    lv_obj_set_scrollbar_mode(bg, LV_SCROLLBAR_MODE_OFF);

    create_layout(&g_layout, bg);
    memcpy(&g_current, &g_layout, sizeof(icons_t));
    c_menu_test1_draw(&g_current);

#if defined(ANIMATION)
    total_step = c_menu_test1_anim_init(bg);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

    lv_anim_set_exec_cb(&a, cell_cb);
    lv_anim_set_values(&a, 0, total_step);
    lv_anim_set_time(&a, (12*total_step));
    lv_anim_start(&a);
#endif
}

//*****************************************************************************
//
// GUI Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    int ret;

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

    //
    // Create a mutex to avoid the concurrent calling of LVGL functions.
    //
    lvgl_mutex = xSemaphoreCreateMutex();
    if( lvgl_mutex == NULL )
    {
        LV_LOG_ERROR("LVGL mutex create failed!\n");
        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    //
    // Load texture.
    //
    texture_load();

    c_menu_test1_task();

    while (1)
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

