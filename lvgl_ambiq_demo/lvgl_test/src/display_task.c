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
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_graphics.h"
#include "nema_programHW.h"
#include "nema_error.h"
#include "nema_vg.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"

#include "display_task.h"

#include "demos/lv_demos.h"

//*****************************************************************************
//
// Display setting
//
//*****************************************************************************
//Frame buffer size
#ifndef LV_AMBIQ_DISPLAY_BUFFER_RESX
    #define LV_AMBIQ_DISPLAY_BUFFER_RESX (480U)
#endif

#ifndef LV_AMBIQ_DISPLAY_BUFFER_RESY
    #define LV_AMBIQ_DISPLAY_BUFFER_RESY (272U)
#endif

// Select render mode
#if LV_AMBIQ_USE_PARTIAL_MODE==1
    #undef LV_AMBIQ_USE_DIRECT_MODE
    #undef LV_AMBIQ_USE_FULL_MODE

    #undef LV_AMBIQ_RENDER_MODE
    #define LV_AMBIQ_RENDER_MODE LV_DISPLAY_RENDER_MODE_PARTIAL
#elif LV_AMBIQ_USE_DIRECT_MODE==1
    #undef LV_AMBIQ_USE_PARTIAL_MODE
    #undef LV_AMBIQ_USE_FULL_MODE

    #undef LV_AMBIQ_RENDER_MODE
    #define LV_AMBIQ_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#elif LV_AMBIQ_USE_FULL_MODE==1
    #undef LV_AMBIQ_USE_PARTIAL_MODE
    #undef LV_AMBIQ_USE_DIRECT_MODE

    #undef LV_AMBIQ_RENDER_MODE
    #define LV_AMBIQ_RENDER_MODE LV_DISPLAY_RENDER_MODE_FULL
#else
    #error "select one of the render mode: PARTIAL, DIRECT, FULL"
#endif

// Draw buffer size, default: draw buffer has the same size as display buffer
#ifndef LV_AMBIQ_DRAW_BUFFER_RATIO
    #define LV_AMBIQ_DRAW_BUFFER_RATIO 4 
#endif

//Display refresh timeout, Unit: ticks.
#ifndef DISPLAY_REFRESH_TIMEOUT
    #define DISPLAY_REFRESH_TIMEOUT (1000U) 
#endif

//*****************************************************************************
//
// Setting check.
//
//*****************************************************************************
#if ((LV_AMBIQ_USE_DIRECT_MODE==1) || (LV_AMBIQ_USE_FULL_MODE==1))
    #if LV_AMBIQ_DRAW_BUFFER_RATIO != 1
        #error "Need a full screen sized draw buffer to enable direct mode or full refresh mode"
    #endif
#endif

#if LV_AMBIQ_DRAW_BUFFER_RATIO==0
    #error "LV_AMBIQ_DRAW_BUFFER_RATIO must be bigger than 1"
#endif

//*****************************************************************************
//
// Buffer size.
//
//*****************************************************************************
#if LV_COLOR_DEPTH==8
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_L8
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT_NEMA        NEMA_L8
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT_NEMA     NEMA_L8
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_8BIT
#elif LV_COLOR_DEPTH==16
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_RGB565
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT_NEMA        NEMA_RGB565
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT_NEMA     NEMA_RGB565
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_RGB565
#elif LV_COLOR_DEPTH==24
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_RGB888
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT_NEMA        NEMA_BGR24
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT_NEMA     NEMA_RGB24  
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_RGB888
#elif LV_COLOR_DEPTH==32
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_XRGB8888
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT_NEMA        NEMA_BGRX8888
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT_NEMA     NEMA_RGB24     
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_RGB888
#endif

#define LV_AMBIQ_DISPLAY_BUFFER_SIZE (LV_AMBIQ_DISPLAY_BUFFER_RESX * LV_AMBIQ_DISPLAY_BUFFER_RESY * (LV_COLOR_DEPTH / 8))
#define LV_AMBIQ_DRAW_BUFFER_SIZE (LV_AMBIQ_DISPLAY_BUFFER_SIZE / LV_AMBIQ_DRAW_BUFFER_RATIO)
#define LV_AMBIQ_STENCIL_BUFFER_SIZE (LV_AMBIQ_DISPLAY_BUFFER_RESX * LV_AMBIQ_DISPLAY_BUFFER_RESY)

// Draw buffer, GPU or CPU will always render to this buffer.
//AM_SHARED_RW __attribute__((aligned(32))) uint8_t draw_buffer[LV_AMBIQ_DRAW_BUFFER_SIZE];
lv_draw_buf_t* draw_buffer = NULL;
// Display buffer. Draw buffer will be copied to this buffer in display->flush_cb, and DC will always read from this buffer when TE is recived.
//AM_SHARED_RW __attribute__((aligned(32))) uint8_t display_buffer[LV_AMBIQ_DISPLAY_BUFFER_SIZE];
nema_buffer_t display_buffer = {.base_phys= 0, .base_virt= 0, .size= 0};

#if LV_USE_DRAW_AMBIQ_VG
// Stencil buffer, Used by NemaVG.
//AM_SHARED_RW __attribute__((aligned(32))) uint8_t stencil_buffer[LV_AMBIQ_STENCIL_BUFFER_SIZE];
nema_buffer_t stencil_buffer = {.base_phys= 0, .base_virt= 0, .size= 0};;
#endif

//*****************************************************************************
//
// DISPLAY task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

//*****************************************************************************
//
// Semaphores
//
//*****************************************************************************
SemaphoreHandle_t lvgl_mutex = NULL;
SemaphoreHandle_t display_buffer_lock = NULL;

//*****************************************************************************
//
// Private data
//
//*****************************************************************************
// static img_obj_t g_sFrameBuffer[2];

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
void buffer_sync(const lv_area_t * area, lv_display_render_mode_t render_mode, void* src)
{
    nema_cmdlist_t cl_memcpy;
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    if((w <= 0) || (h <= 0))
        return;

    // Create the command list for GPU memcpy.
    cl_memcpy = nema_cl_create_sized(0x100);
    LV_ASSERT_NULL(cl_memcpy.bo.base_virt);
    if(cl_memcpy.bo.base_virt == NULL)
    {
        return;
    }

#if LV_USE_DRAW_AMBIQ
    lv_draw_ambiq_nema_context_lock();
#endif

    //Rewind and bind the CL
    nema_cl_bind(&cl_memcpy);
    nema_cl_rewind(&cl_memcpy);

    // Bind destination buffer
    nema_bind_dst_tex(display_buffer.base_phys,
                      LV_AMBIQ_DISPLAY_BUFFER_RESX,
                      LV_AMBIQ_DISPLAY_BUFFER_RESY,
                      LV_AMBIQ_DISPLAY_BUFFER_FORMAT_NEMA,
                      -1);

    //Set clip
    nema_set_clip(area->x1, area->y1, w, h);

    //Set blend mode
    lv_ambiq_set_blend_blit(NULL, NEMA_BL_SRC);

    //Bind source buffer
    uint32_t source_width;
    uint32_t source_hight;
    if((LV_AMBIQ_RENDER_MODE==LV_DISPLAY_RENDER_MODE_DIRECT) || (LV_AMBIQ_RENDER_MODE==LV_DISPLAY_RENDER_MODE_FULL))
    {
        source_width = LV_AMBIQ_DISPLAY_BUFFER_RESX;
        source_hight = LV_AMBIQ_DISPLAY_BUFFER_RESY;
    }
    else
    {
        source_width = w;
        source_hight = h;
    }
    nema_bind_src_tex((uintptr_t)src,
                  source_width,
                  source_hight,
                  LV_AMBIQ_DRAW_BUFFER_FORMAT_NEMA,
                  -1,
                  NEMA_FILTER_PS);

    //Blit
    if((LV_AMBIQ_RENDER_MODE==LV_DISPLAY_RENDER_MODE_DIRECT) || (LV_AMBIQ_RENDER_MODE==LV_DISPLAY_RENDER_MODE_FULL))
    {
        nema_blit_subrect(area->x1, area->y1, w, h, area->x1, area->y1);
    }
    else
    {
        nema_blit_rect(area->x1, area->y1, w, h);
    }

    //start GPU, submit CL
    nema_cl_submit(&cl_memcpy);

#if LV_USE_DRAW_AMBIQ
    lv_draw_ambiq_nema_context_unlock();
#endif

    nema_cl_wait(&cl_memcpy);

    nema_cl_destroy(&cl_memcpy);

    if(nema_get_error() != NEMA_ERR_NO_ERROR)
        LV_LOG_ERROR("gpu memory copy error!");

    return;
}

void transfer_complete_cb(void* args)
{
    SemaphoreHandle_t sem = (SemaphoreHandle_t)args;

    if(xPortIsInsideInterrupt())
    {
        xSemaphoreGiveFromISR(sem, NULL);
    }
    else
    {
        xSemaphoreGive(sem);
    }
}

//*****************************************************************************
//
// display flush callback.
//
//*****************************************************************************
void 
display_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    // offset the draw area.
    lv_area_t area_display = *area;
    int32_t x_off = lv_display_get_offset_x(display);
    int32_t y_off = lv_display_get_offset_y(display);
    lv_area_move(&area_display, -x_off, -y_off);

    // Clean the draw buffer
    lv_draw_buf_flush_cache(draw_buffer, NULL);

    // Lock display buffer, prevent display interface from reading this buffer.
    bool ret = xSemaphoreTake(display_buffer_lock, DISPLAY_REFRESH_TIMEOUT);
    if(ret == pdFALSE)
    {
        LV_LOG_ERROR("display buffer transfer timer out!");
    }


    // Copy draw buffer to display buffer.
    buffer_sync(&area_display, LV_AMBIQ_RENDER_MODE, (void*)px_map);

    // Unlock this buffer.
    xSemaphoreGive(display_buffer_lock);

    // Inform LVGL that the draw buffer is available to be used.
    lv_disp_flush_ready(display);

    // If this is the last part, start transfer the display buffer to display panel.
    bool is_last = lv_display_flush_is_last(display);

    if(is_last)
    {
        // Lock display buffer, prevent GPU from writing to this buffer.
        // It will be released when display interface transfer complete.
        xSemaphoreTake(display_buffer_lock, portMAX_DELAY);       

        am_devices_display_transfer_frame(LV_AMBIQ_DISPLAY_BUFFER_RESX,
                                          LV_AMBIQ_DISPLAY_BUFFER_RESY,
                                          display_buffer.base_phys,
                                          transfer_complete_cb, 
                                          (void*)display_buffer_lock);


    }
}

//*****************************************************************************
//
// LVGL display driver setup.
//
//*****************************************************************************
void lv_disp_drv_setup(void)
{
    // Create the display buffer.
    display_buffer = nema_buffer_create_pool(NEMA_MEM_POOL_FB, LV_AMBIQ_DISPLAY_BUFFER_SIZE);
    if(display_buffer.base_virt == NULL)
    {
        LV_LOG_ERROR("display buffer create failed!");
    }

    // Create the display.
    lv_display_t * display = lv_display_create(LV_AMBIQ_DISPLAY_BUFFER_RESX, LV_AMBIQ_DISPLAY_BUFFER_RESY);

    // Set flush cb
    lv_display_set_flush_cb(display, display_flush_cb);

    // Create draw buffer.
    draw_buffer = lv_draw_buf_create(LV_AMBIQ_DISPLAY_BUFFER_RESX, LV_AMBIQ_DISPLAY_BUFFER_RESY/LV_AMBIQ_DRAW_BUFFER_RATIO, LV_AMBIQ_DRAW_BUFFER_FORMAT, 0);
    //Set draw buffer
    lv_display_set_draw_buffers(display, draw_buffer, NULL);

    //Set display refresh mode
    lv_display_set_render_mode(display, LV_AMBIQ_RENDER_MODE);

    // Set display physical resolution.
    lv_display_set_physical_resolution(display, g_sDispCfg.ui16ResX, g_sDispCfg.ui16ResY);

    // Set display offset, the display refresh area is always in the center of the display panel.
    uint32_t offset_x = (LV_AMBIQ_DISPLAY_BUFFER_RESX < g_sDispCfg.ui16ResX) ? \
                        (((g_sDispCfg.ui16ResX - LV_AMBIQ_DISPLAY_BUFFER_RESY) >> 2) << 1) : 0;
    uint32_t offset_y = (LV_AMBIQ_DISPLAY_BUFFER_RESY < g_sDispCfg.ui16ResY) ? \
                         (((g_sDispCfg.ui16ResY - LV_AMBIQ_DISPLAY_BUFFER_RESY) >> 2) << 1) : 0;
    lv_display_set_offset(display, offset_x, offset_y);
}

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
DisplayTask(void *pvParameters)
{
    int ret;

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
    // If NEMA_GFX_POWERSAVE is defined, we keep GPU power off until an GPU CL is ready to submit.
    //
#ifndef NEMA_GFX_POWERSAVE
    //
    // Power on GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX.
    //
    nema_init();
    if (NEMA_ERR_NO_ERROR != nema_get_error())
    {
        am_util_stdio_printf("NemaGFX init failed!\n");
    }

    //
    // Initialize NemaVG.
    //
#if LV_USE_DRAW_AMBIQ_VG
    stencil_buffer = nema_buffer_create_pool(NEMA_MEM_POOL_FB, LV_AMBIQ_STENCIL_BUFFER_SIZE);
    nema_vg_init_stencil_prealloc(LV_AMBIQ_DISPLAY_BUFFER_RESX, LV_AMBIQ_DISPLAY_BUFFER_RESY, stencil_buffer);
    if (NEMA_ERR_NO_ERROR != nema_get_error())
    {
        am_util_stdio_printf("NemaVG init failed!\n");
    }
#endif

#endif

    //
    // Init display hardware, including display interface and panel.
    //
    ret = am_devices_display_init(LV_AMBIQ_DISPLAY_BUFFER_RESX,
                                  LV_AMBIQ_DISPLAY_BUFFER_RESY,
                                  LV_AMBIQ_DISPLAY_PANEL_FORMAT,
                                  false);
    if (ret != 0)
    {
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
        if(ret == AM_DEVICES_DISPLAY_STATUS_OUT_OF_RANGE)
        {
            //Check the dsi frequency does exceed the limit of the screen or not, if it is exceeded, set it according to the screen maximum
            #if (LV_AMBIQ_DISPALY_PANEL_FORMAT == COLOR_FORMAT_RGB565)
                if ((g_sDispCfg.eDsiFreq & 0x0f) > (AM_HAL_DSI_FREQ_TRIM_X13 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X13;
                    LV_LOG_WARN("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X13\n");
                }
            #elif (LV_AMBIQ_DISPALY_PANEL_FORMAT == COLOR_FORMAT_RGB888)
                if ((g_sDispCfg.eDsiFreq & 0x0f) >= (AM_HAL_DSI_FREQ_TRIM_X20 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X20;
                    LV_LOG_WARN("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X20\n");
                }
            #endif

            //reinit
            ret = am_devices_display_init(LV_AMBIQ_DISPLAY_BUFFER_RESX,
                                  LV_AMBIQ_DISPLAY_BUFFER_RESY,
                                  LV_AMBIQ_DISPLAY_PANEL_FORMAT,
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
    // Init LVGL.
    //
    lv_init();

    //
    // Set up LVGL display driver.
    //
    lv_disp_drv_setup();

    /* Create a mutex to avoid the concurrent calling of LVGL functions. */
    lvgl_mutex = xSemaphoreCreateMutex();
    if( lvgl_mutex == NULL )
    {
        LV_LOG_ERROR("LVGL mutex create failed!\n");
        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    display_buffer_lock = xSemaphoreCreateBinary();
    if( display_buffer_lock == NULL )
    {
        LV_LOG_ERROR("display_buffer_lock mutex create failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    xSemaphoreGive(display_buffer_lock);

    lv_tick_set_cb(xTaskGetTickCount);

#if LV_USE_LOG == 1
    lv_log_register_print_cb(lv_ambiq_log_printf);
#endif

    lv_demo_render(LV_DEMO_RENDER_SCENE_TRIANGLE, LV_OPA_COVER);
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