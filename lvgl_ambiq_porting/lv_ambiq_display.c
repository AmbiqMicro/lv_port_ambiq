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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_display_generic.h"
#include "am_debug_pin.h"

#include "lvgl.h"
#include "lv_draw_ambiq_private.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "lv_ambiq_display.h"



//*****************************************************************************
//
// Display setting
//
//*****************************************************************************

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
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT          LV_COLOR_FORMAT_L8    
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_8BIT
#elif LV_COLOR_DEPTH==16
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_RGB565
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT          LV_COLOR_FORMAT_RGB565    
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_RGB565
#elif LV_COLOR_DEPTH==24
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_RGB888
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT          LV_COLOR_FORMAT_BGR888
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_RGB888
#elif LV_COLOR_DEPTH==32
    #define LV_AMBIQ_DRAW_BUFFER_FORMAT             LV_COLOR_FORMAT_XRGB8888
    #define LV_AMBIQ_DISPLAY_BUFFER_FORMAT          LV_COLOR_FORMAT_BGR888    
    #define LV_AMBIQ_DISPLAY_PANEL_FORMAT           COLOR_FORMAT_RGB888
#endif

#define LV_AMBIQ_DISPLAY_BUFFER_SIZE (LV_AMBIQ_DISPLAY_BUFFER_RESX * LV_AMBIQ_DISPLAY_BUFFER_RESY * (LV_COLOR_DEPTH / 8))
#define LV_AMBIQ_DRAW_BUFFER_SIZE (LV_AMBIQ_DISPLAY_BUFFER_SIZE / LV_AMBIQ_DRAW_BUFFER_RATIO)

// Draw buffer, GPU or CPU will always render to this buffer.
lv_draw_buf_t* draw_buffer = NULL;
// Display buffer. Draw buffer will be copied to this buffer in display->flush_cb, and DC will always read from this buffer when TE is recived.
lv_draw_buf_t* display_buffer = NULL;

//*****************************************************************************
//
// Semaphores
//
//*****************************************************************************
SemaphoreHandle_t display_buffer_lock = NULL;

void transfer_complete_cb(void* args)
{
    SemaphoreHandle_t sem = (SemaphoreHandle_t)args;

    AM_DEBUG_PIN_CLEAR(DEBUG_PIN_2);

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

    // If this is the last part, wait for GPU complete all the queued jobs 
    // and start transfer the display buffer to display panel.
    bool is_last = lv_display_flush_is_last(display);

    if((LV_AMBIQ_RENDER_MODE != LV_DISPLAY_RENDER_MODE_PARTIAL) && !is_last)
    {
        // Inform LVGL that the draw buffer is available to be used.
        lv_disp_flush_ready(display); 
        
        return;
    }

    // Lock display buffer, prevent display interface from reading this buffer.
    bool ret = xSemaphoreTake(display_buffer_lock, DISPLAY_REFRESH_TIMEOUT);
    if(ret == pdFALSE)
    {
        LV_LOG_ERROR("display buffer transfer timer out!");
    }

    // Copy draw buffer to display buffer.
	lv_area_t *target_area = (LV_AMBIQ_RENDER_MODE == LV_DISPLAY_RENDER_MODE_PARTIAL) ? &area_display : NULL;
	lv_draw_ambiq_display_buffer_sync(display_buffer, target_area, (void*)px_map, LV_AMBIQ_DRAW_BUFFER_FORMAT);

    // Unlock this buffer.
    xSemaphoreGive(display_buffer_lock);

    // Inform LVGL that the draw buffer is available to be used.
    lv_disp_flush_ready(display);

    if(is_last)
    {
        // Lock display buffer, prevent GPU from writing to this buffer.
        // It will be released when display interface transfer complete.
        xSemaphoreTake(display_buffer_lock, portMAX_DELAY);       

        AM_DEBUG_PIN_SET(DEBUG_PIN_2);

        am_devices_display_transfer_frame(LV_AMBIQ_DISPLAY_BUFFER_RESX,
                                          LV_AMBIQ_DISPLAY_BUFFER_RESY,
                                          (uintptr_t)display_buffer->data,
                                          transfer_complete_cb, 
                                          (void*)display_buffer_lock);
    }
}

#ifdef USE_DEBUG_PIN
static void display_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_RENDER_START) {
        AM_DEBUG_PIN_SET(DEBUG_PIN_1);
    }
    else if(code == LV_EVENT_RENDER_READY) {
        AM_DEBUG_PIN_CLEAR(DEBUG_PIN_1);
    }
}
#endif

//*****************************************************************************
//
// LVGL display driver setup.
//
//*****************************************************************************
void lv_disp_drv_setup(void)
{
    // Create the display buffer.
    display_buffer = lv_draw_buf_create(LV_AMBIQ_DISPLAY_BUFFER_RESX, LV_AMBIQ_DISPLAY_BUFFER_RESY, LV_AMBIQ_DISPLAY_BUFFER_FORMAT, 0);

    // Create the display.
    lv_display_t * display = lv_display_create(LV_AMBIQ_DISPLAY_BUFFER_RESX, LV_AMBIQ_DISPLAY_BUFFER_RESY);

    // Set flush cb
    lv_display_set_flush_cb(display, display_flush_cb);

#ifdef USE_DEBUG_PIN
    lv_display_add_event_cb(display, display_event_cb, LV_EVENT_RENDER_START, NULL);
    lv_display_add_event_cb(display, display_event_cb, LV_EVENT_RENDER_READY, NULL);
#endif

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

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
lv_ambiq_display_init(void)
{
    int ret;

    LV_LOG_INFO("Display Init!\n");


    display_buffer_lock = xSemaphoreCreateBinary();
    if( display_buffer_lock == NULL )
    {
        LV_LOG_ERROR("display_buffer_lock mutex create failed!\n");
    }

    xSemaphoreGive(display_buffer_lock);

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
            if (LV_AMBIQ_DISPLAY_PANEL_FORMAT == COLOR_FORMAT_RGB565)
            {
                if ((g_sDispCfg.eDsiFreq & 0x0f) > (AM_HAL_DSI_FREQ_TRIM_X13 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X13;
                    LV_LOG_WARN("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X13\n");
                }
            }
            else if (LV_AMBIQ_DISPLAY_PANEL_FORMAT == COLOR_FORMAT_RGB888)
            {
                if ((g_sDispCfg.eDsiFreq & 0x0f) >= (AM_HAL_DSI_FREQ_TRIM_X20 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X20;
                    LV_LOG_WARN("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X20\n");
                }
            }

            //reinit
            ret = am_devices_display_init(LV_AMBIQ_DISPLAY_BUFFER_RESX,
                                  LV_AMBIQ_DISPLAY_BUFFER_RESY,
                                  LV_AMBIQ_DISPLAY_PANEL_FORMAT,
                                  false);
            if (ret != 0)
            {
                LV_LOG_ERROR("display init failed!\n");
            }
        }
        else
#endif
        {
            LV_LOG_ERROR("display init failed!\n");
        }
    }

    //
    // Set up LVGL display driver.
    //
    lv_disp_drv_setup();
}