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
//
//
//*****************************************************************************
#ifdef LV_AMBIQ_FB_REFRESH_TWO


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "lvgl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_graphics.h"
#include "nema_programHW.h"

#include "display_task.h"
#include "lv_gpu_ambiq_nema.h"
#include "lv_ambiq_nema_hal.h"
//*****************************************************************************
//
// Display setting
//
//*****************************************************************************
//Frame buffer and display panel color format, 0 enable, 1 disable.
#ifndef LV_AMBIQ_FB_USE_RGB565
    #define LV_AMBIQ_FB_USE_RGB565 1
#endif

#ifndef LV_AMBIQ_FB_USE_RGB888
    #define LV_AMBIQ_FB_USE_RGB888 0
#endif

//Frame buffer size
#ifndef LV_AMBIQ_FB_RESX
    #define LV_AMBIQ_FB_RESX (390U)
#endif

#ifndef LV_AMBIQ_FB_RESY
    #define LV_AMBIQ_FB_RESY (390U)
#endif

//Use direct mode, see https://docs.lvgl.io/master/porting/display.html#direct-mode.
#ifndef USE_DIRECT_MODE
    #define USE_DIRECT_MODE 1
#endif

//Use full refresh, see https://docs.lvgl.io/master/porting/display.html#full-refresh.
#ifndef USE_FULL_REFRESH
    #define USE_FULL_REFRESH 0
#endif

//Display refresh timeout
#ifndef DISPLAY_REFRESH_TIMEOUT
    #define DISPLAY_REFRESH_TIMEOUT (1000U) //1000 ticks, 1000*1ms= 1s
#endif
//*****************************************************************************
//
// Setting check.
//
//*****************************************************************************

#if LV_AMBIQ_FB_USE_RGB888
    #define DISPLAY_FORMAT            COLOR_FORMAT_RGB888
    #define FRAME_BUFFER_FORMAT       NEMA_RGB24
#elif LV_AMBIQ_FB_USE_RGB565
    #define DISPLAY_FORMAT            COLOR_FORMAT_RGB565
    #define FRAME_BUFFER_FORMAT       NEMA_RGB565
#else
    #error "Display format not supported!"
#endif

#if LV_AMBIQ_FB_USE_RGB888 && LV_AMBIQ_FB_USE_RGB565
#warning "Both RGB88 and RGB565 color formats are enabled, using RGB888."
#endif

#if !USE_DIRECT_MODE && !USE_FULL_REFRESH
#warning "Display panel must support partial refresh feature, and the panel should \
          have no restriction on the start and end column setting."
#endif

//*****************************************************************************
//
// DISPLAY task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;

//*****************************************************************************
//
// Display refresh command.
//
//*****************************************************************************
typedef struct display_refresh_cmd_t_
{
   uint32_t start_x;
   uint32_t start_y;
   uint32_t width;
   uint32_t hight;
   uint8_t  format;
   void* pFrameBuffer;
   lv_disp_drv_t *disp_drv;
}display_refresh_cmd;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
display_refresh_cmd cmdDisplayRefresh =
{
    .start_x =0,
    .start_y = 0,
    .width = 0,
    .hight = 0,
    .format = 0,
    .pFrameBuffer =NULL,
    .disp_drv = NULL,
};

//*****************************************************************************
//
// Private data
//
//*****************************************************************************
static img_obj_t g_sFrameBuffer[2] =
{
    {{0},  LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, -1, 1, FRAME_BUFFER_FORMAT, NEMA_FILTER_BL,},
    {{0},  LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, -1, 1, FRAME_BUFFER_FORMAT, NEMA_FILTER_BL,},
};

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

//*****************************************************************************
//
//! @brief GPU memcpy, use the GPU do the memcpy, this much faster than CPU memcpy.
//!
//! @param resx - X resolution.
//! @param resy - Y resolution.
//! @param format - Color format.
//! @param des - Destination buffer.
//! @param src - Source buffer.
//!
//! @note The src and des buffers must have the same resolution and color format.
//!
//! @return 0, success, -1, failed.
//
//*****************************************************************************
int 
GPU_memcpy(uint32_t resx, uint32_t resy, uint8_t format, void* des, void* src)
{
    nema_cmdlist_t cl_memcpy;

    // Create the command list for GPU memcpy.
    cl_memcpy = nema_cl_create_sized(0x100);
    LV_ASSERT_NULL(cl_memcpy.bo.base_virt);
    if(cl_memcpy.bo.base_virt == NULL)
    {
        return -1;
    }

#ifdef NEMA_GFX_POWERSAVE
    //Power up GPU
    NEMA_BUILDCL_START
    lv_ambiq_nema_gpu_power_on();
#endif

    //Rewind and bind the CL
    nema_cl_bind(&cl_memcpy);
    nema_cl_rewind(&cl_memcpy);

    // Bind destination buffer
    nema_bind_dst_tex((uintptr_t)des,
                      resx,
                      resy,
                      format,
                      -1);

    //Set clip
    nema_set_clip(0, 0, resx, resy);

    //Set blend mode
    nema_set_blend_blit(NEMA_BL_SRC);

    //Bind source buffer
    nema_bind_src_tex((uintptr_t)src,
                  resx,
                  resy,
                  format,
                  -1,
                  NEMA_FILTER_PS);

    //Blit
    nema_blit(0, 0);

    //start GPU, submit CL
    nema_cl_submit(&cl_memcpy);

#if defined(NEMA_GFX_POWERSAVE) && defined(NEMA_GFX_POWEROFF_END_CL)
    //Power off GPU in CL end
    NEMA_BUILDCL_END
    lv_ambiq_nema_gpu_check_busy_and_suspend();
#endif

    nema_cl_wait(&cl_memcpy);

#if defined(NEMA_GFX_POWERSAVE) && !defined(NEMA_GFX_POWEROFF_END_CL)
    //Power off GPU in frame end
    NEMA_BUILDCL_END
    lv_ambiq_nema_gpu_check_busy_and_suspend();
#endif

    nema_cl_destroy(&cl_memcpy);

    return 0;
}

//*****************************************************************************
//
// display flush callback.
//
//*****************************************************************************
void 
display_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, 
                 lv_color_t * color_p)
{
    cmdDisplayRefresh.format = FRAME_BUFFER_FORMAT;
    cmdDisplayRefresh.pFrameBuffer = color_p;
    cmdDisplayRefresh.start_x = area->x1;
    cmdDisplayRefresh.start_y = area->y1;
    cmdDisplayRefresh.width = area->x2 - area->x1  + 1;
    cmdDisplayRefresh.hight = area->y2 - area->y1  + 1;
    cmdDisplayRefresh.disp_drv = disp_drv;

#if USE_DIRECT_MODE && !USE_FULL_REFRESH
    //Do the frame buffer synchronization.
    lv_color_t *src;
    lv_color_t *des;
    if(color_p == g_sFrameBuffer[0].bo.base_virt)
    {
        src = g_sFrameBuffer[0].bo.base_virt;
        des = g_sFrameBuffer[1].bo.base_virt;
    }
    else if(color_p == g_sFrameBuffer[1].bo.base_virt)
    {
        src = g_sFrameBuffer[1].bo.base_virt;
        des = g_sFrameBuffer[0].bo.base_virt;
    }
    else
    {
        LV_ASSERT(0);
    }

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_1);
#endif

    GPU_memcpy(LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, FRAME_BUFFER_FORMAT, des, src);

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_clear(DEBUG_PIN_1);
#endif

#endif //USE_DIRECT_MODE && !USE_FULL_REFRESH

    //Trigger the display refresh task.
#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_2);
    am_hal_gpio_output_clear(DEBUG_PIN_2);
#endif
    xTaskNotifyGive( DisplayTaskHandle);

}


//*****************************************************************************
//
// Allocate buffer in SSRAM using tsi_malloc
//
//*****************************************************************************
int
SSRAM_buffer_alloc(img_obj_t* img)
{
    uint32_t size;

    size = nema_texture_size(img->format, 0, img->w, img->h);

    img->bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);

    if ( img->bo.base_virt == NULL )
    {
      LV_LOG_ERROR("TSI buffer Malloc failed!\n");
      return -1;
    }

    memset(img->bo.base_virt, 0, size);

    return 0;
}


//*****************************************************************************
//
// LVGL display driver setup.
//
//*****************************************************************************
int
lv_disp_drv_setup(void)
{
    int ret;

    //Alloc SSRAM memory for frame buffer
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[0]);
    if ( ret < 0 )
    {
       return ret;
    }

    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[1]);
    if ( ret < 0 )
    {
       return ret;
    }

    // Init display buffer
    lv_disp_draw_buf_init(&disp_buf, 
                          g_sFrameBuffer[0].bo.base_virt, 
                          g_sFrameBuffer[1].bo.base_virt,
                          g_sFrameBuffer[0].w * g_sFrameBuffer[0].h);

    // Init display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.gpu_wait_cb = lv_gpu_ambiq_nema_wait;
    disp_drv.hor_res = LV_AMBIQ_FB_RESX;
    disp_drv.ver_res = LV_AMBIQ_FB_RESY;
    disp_drv.full_refresh = USE_FULL_REFRESH;
    disp_drv.direct_mode = USE_DIRECT_MODE;
    disp_drv.user_data = (void*)&g_sFrameBuffer[0];

    // Register the driver and save the created display objects.
    lv_disp_t * disp;
    disp = lv_disp_drv_register(&disp_drv);
    LV_UNUSED(disp);

    return ret;
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
    display_refresh_cmd cmdDisplayRefreshRecord = cmdDisplayRefresh;

    LV_LOG_INFO("Display task start!\n");

#ifdef USE_DEBUG_PIN
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output); //keep high when the GPU memcpy is working
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output); //Toggle when the GPU finished his work
    am_hal_gpio_pinconfig(DEBUG_PIN_3, am_hal_gpio_pincfg_output); //Keep high when the display task is active
    am_hal_gpio_pinconfig(DEBUG_PIN_4, am_hal_gpio_pincfg_output); //Keep high when the display data transfer is active
    am_hal_gpio_pinconfig(DEBUG_PIN_5, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_6, am_hal_gpio_pincfg_output);
#endif

    //
    // Init display hardware, including display interface and panel.
    //
    ret = am_devices_display_init(LV_AMBIQ_FB_RESX,
                                  LV_AMBIQ_FB_RESY,
                                  DISPLAY_FORMAT,
                                  false);
    if (ret != 0)
    {
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
        if(ret == AM_DEVICES_DISPLAY_STATUS_OUT_OF_RANGE)
        {
            //Check the dsi frequency does exceed the limit of the screen or not, if it is exceeded, set it according to the screen maximum
            #if (DISPLAY_FORMAT == COLOR_FORMAT_RGB565)
                if ((g_sDispCfg.eDsiFreq & 0x0f) > (AM_HAL_DSI_FREQ_TRIM_X13 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X13;
                    LV_LOG_WARN("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X13\n");
                }
            #elif (DISPLAY_FORMAT == COLOR_FORMAT_RGB888)
                if ((g_sDispCfg.eDsiFreq & 0x0f) >= (AM_HAL_DSI_FREQ_TRIM_X20 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X20;
                    LV_LOG_WARN("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X20\n");
                }
            #endif

            //reinit
            ret = am_devices_display_init(LV_AMBIQ_FB_RESX,
                                  LV_AMBIQ_FB_RESY,
                                  DISPLAY_FORMAT,
                                  true);
            if (ret != 0)
            {
                LV_LOG_ERROR("display init failed!\n");
                //suspend and delete this task.
                vTaskDelete(NULL);
            }
        }
        else
#endif
        {
            LV_LOG_ERROR("display init failed!\n");
            //suspend and delete this task.
            vTaskDelete(NULL);
        }
    }

    //
    // Set up LVGL display driver.
    //
    ret = lv_disp_drv_setup();
    if (ret != 0)
    {
        LV_LOG_ERROR("LVGL display driver setup failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    while(1)
    {
        //Wait start.
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_set(DEBUG_PIN_3);
#endif

        if( ulNotificationValue != 1 )
        {
            LV_LOG_ERROR("Display task wait notify timeout!\n");
        }

#if (!USE_DIRECT_MODE) && (!USE_FULL_REFRESH)
        //Check whether the display region is updated
        if((cmdDisplayRefresh.width != cmdDisplayRefreshRecord.width) || 
           (cmdDisplayRefresh.hight != cmdDisplayRefreshRecord.hight) ||
           (cmdDisplayRefresh.start_x != cmdDisplayRefreshRecord.start_x) || 
           (cmdDisplayRefresh.start_y != cmdDisplayRefreshRecord.start_y))
        {
            //Set display region
            am_devices_display_set_region(cmdDisplayRefresh.width,
                                        cmdDisplayRefresh.hight,
                                        cmdDisplayRefresh.start_x,
                                        cmdDisplayRefresh.start_y);

            //Updated display region record.
            cmdDisplayRefreshRecord = cmdDisplayRefresh;
        }
#else
        LV_UNUSED(cmdDisplayRefreshRecord);
#endif

        am_devices_display_transfer_frame(cmdDisplayRefresh.width,
                                          cmdDisplayRefresh.hight,
                                          (uint32_t)cmdDisplayRefresh.pFrameBuffer,
                                          NULL, NULL);

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_set(DEBUG_PIN_4);
#endif

        //Wait transfer done.
        uint32_t transfer_ret = am_devices_display_wait_transfer_done();
        if(AM_DEVICES_DISPLAY_STATUS_ERROR == transfer_ret)
        {
            LV_LOG_ERROR("Display refresh timeout!\n");
        }

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_clear(DEBUG_PIN_4);
#endif

        // Inform LVGL library that the framebuffer is avaliable*/
        lv_disp_flush_ready(cmdDisplayRefresh.disp_drv);

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_clear(DEBUG_PIN_3);
#endif

    }

}

#endif