//*****************************************************************************
//
//  lv_ambiq_nema_hal.c
//! @file
//!
//! @brief Support functions for nema_hal.c, these functions will be called from
//! nema_hal.c to allocated memory for NemaSDK. 
//!
//
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

#include "lvgl.h"
#include "nema_sys_defs.h"
#include "nema_regs.h"
#include "nema_error.h"
#include "nema_hal.h"
#include "nema_graphics.h"
#include "am_mcu_apollo.h"

#if !defined(AM_PART_APOLLO4L) && !defined(AM_PART_APOLLO4B)
    #include "nema_vg.h"
#endif


/**
 * Allocate memory from the specified memory pool
 * @param pool The memory pool
 * @param size_in_byte Size to be allocate
 * @return Pointer to the allocated memory, NULL if failed.
 */
void* lv_ambiq_nema_hal_malloc(int pool, uint32_t size_in_byte)
{
    void* ptr;

    switch(pool)
    {
        case NEMA_MEM_POOL_CL_RB:
            ptr = lv_mem_ssram_alloc(size_in_byte);
            break;
        case NEMA_MEM_POOL_FB_TEX:
            ptr = lv_mem_ssram_alloc(size_in_byte);
            break;
        case NEMA_MEM_POOL_MISC:
            ptr = lv_mem_alloc(size_in_byte);
            break;
        default:
            ptr = NULL;
            break;        
    }
    return ptr;
}

/**
 * Free the specified memory
 * @param ptr Pointer to the memory section to be free.
 * @return 
 */
void lv_ambiq_nema_hal_free(void* ptr)
{
    lv_mem_free(ptr);
}


static volatile bool building_cl = false;

void nema_build_cl_start(void)
{
    AM_CRITICAL_BEGIN
    building_cl = true;
    AM_CRITICAL_END
}

void nema_build_cl_end(void)
{
    AM_CRITICAL_BEGIN
    building_cl = false;
    AM_CRITICAL_END
}

//*****************************************************************************
//
//! @brief Power on GPU
//!
//! @return
//
//*****************************************************************************
lv_res_t lv_ambiq_nema_gpu_power_on(void)
{
    bool enabled;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &enabled);

    if(enabled)
        return LV_RES_OK;

#ifdef AM_PART_APOLLO4L

    //A workaround has been added to nema_hal.c for Apollo4l.
    am_gpu_power_enable(AM_GPU_PWRCTRL_USER);
    return LV_RES_OK;
#elif defined(AM_PART_APOLLO4B)
    uint32_t ret = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    if(ret == AM_HAL_STATUS_SUCCESS)
    {
        nema_init();

        ret = nema_get_error();
        if(ret != NEMA_ERR_NO_ERROR)
        {
            return LV_RES_INV;
        }  
        return LV_RES_OK;
    }
    else
    {
        return LV_RES_INV;
    }
#else
    uint32_t ret = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    if(ret == AM_HAL_STATUS_SUCCESS)
    {
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
        nema_reinit();
        nema_reset_last_cl_id();
#else
        //nema_reset_last_cl_id() is not available on apollo4, need use nema_init().
        nema_init();
#endif

        ret = nema_get_error();
        if(ret != NEMA_ERR_NO_ERROR)
        {
            LV_LOG_ERROR("nema_get_error = %08lX\n", ret);
            return LV_RES_INV;
        }

        nema_vg_reinit();

        ret = nema_vg_get_error();
        if(ret != NEMA_VG_ERR_NO_ERROR)
        {
            LV_LOG_ERROR("nema_vg_get_error = %08lX\n", ret);
            return LV_RES_INV;
        }

        return LV_RES_OK;
    }
    else
    {
        LV_LOG_ERROR("GPU power up failed!\n");
        return LV_RES_INV;
    }
#endif
    
}

//*****************************************************************************
//
//! @brief Check GPU status and power down if it is idle
//!
//! @return
//
//*****************************************************************************
lv_res_t  lv_ambiq_nema_gpu_check_busy_and_suspend(void)
{
    bool enabled;

    AM_CRITICAL_BEGIN
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &enabled);
    if(enabled)
    {
        // If we are building CL, no need to power it down as we will soon submit it.
        if(building_cl == false)
        {
            //If we are idle, we can power off GPU safely.
            if(nema_reg_read(NEMA_STATUS) == 0)
            {
    #ifdef AM_PART_APOLLO4L
                //A workaround has been added to nema_hal.c for Apollo4l.
                am_gpu_power_disable(AM_GPU_PWRCTRL_USER);
    #else
                am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    #endif
            }
        }

    }
    AM_CRITICAL_END

    return LV_RES_OK;
}

void lv_ambiq_nema_gpu_isr_cb(int last_cl_id)
{
    lv_ambiq_nema_gpu_check_busy_and_suspend();
}