//*****************************************************************************
//
//! @file lv_ambiq_misc.c
//!
//! @brief Defined APIs used by gpu acceleration lib.
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
#include "lv_ambiq_misc.h"
#include "nema_error.h"
#include "nema_graphics.h"
#include "am_mcu_apollo.h"

#if LV_USE_LOG
const char* lv_ambiq_gpu_error_interpret(uint32_t error_code)
{

    const char* message;

    switch(error_code)
    {
        case NEMA_ERR_NO_ERROR:
            message = "NO_ERROR"; 
            break; /**< No error has occured */
        case NEMA_ERR_SYS_INIT_FAILURE:
            message = "SYS_INIT_FAILURE"; 
            break;/**< System initialization failure */
        case NEMA_ERR_GPU_ABSENT:
            message = "GPU_ABSENT";
            break; /**< Nema GPU is absent */
        case NEMA_ERR_RB_INIT_FAILURE:
            message = "RB_INIT_FAILURE";
            break; /**< Ring buffer initialization failure */
        case NEMA_ERR_NON_EXPANDABLE_CL_FULL:
            message = "NON_EXPANDABLE_CL_FULL";
            break; /**< Non expandable command list is full*/
        case NEMA_ERR_CL_EXPANSION:
            message = "CL_EXPANSION"; 
            break;/**< Command list expansion error */
        case NEMA_ERR_OUT_OF_GFX_MEMORY:
            message = "OUT_OF_GFX_MEMORY"; 
            break;/**< Graphics memory is full */
        case NEMA_ERR_OUT_OF_HOST_MEMORY:
            message = "OUT_OF_HOST_MEMORY"; 
            break;/**< Host memory is full */
        case NEMA_ERR_NO_BOUND_CL:
            message = "NO_BOUND_CL"; 
            break;/**< There is no bound command list */
        case NEMA_ERR_NO_BOUND_FONT:
            message = "NO_BOUND_FONT"; 
            break;/**< There is no bound font */
        case NEMA_ERR_GFX_MEMORY_INIT:
            message = "GFX_MEMORY_INIT";
            break;/**< Graphics memory initialization failure */
        case NEMA_ERR_DRIVER_FAILURE:
            message = "DRIVER_FAILURE "; 
            break;/**< Nema GPU Kernel Driver failure*/
        case NEMA_ERR_MUTEX_INIT:
            message = "MUTEX_INIT"; 
            break;/**< Mutex initialization failure*/
        case NEMA_ERR_INVALID_BO:
            message = "INVALID_BO"; 
            break;/**< Invalid buffer provided*/
        case NEMA_ERR_INVALID_CL:
            message = "INVALID_CL"; 
            break;/**< Invalid CL provided*/
        default:
            message = "UNKNOWN"; 
            break;/**< unknown*/
    }
    return message;
}
#endif 

//This function is used by GPU acceleration library.
lv_color_t lv_ambiq_gpu_get_chroma_key(void)
{
    return LV_COLOR_CHROMA_KEY;
}



#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
static void lv_ambiq_get_range_from_disp(lv_disp_t * disp, am_hal_cachectrl_range_t* pRange)
{
    lv_disp_draw_buf_t * draw_buf = lv_disp_get_draw_buf(disp);
    lv_color_t * disp_buf = draw_buf->buf_act;
    img_obj_t * frame_buf = disp->driver->user_data;

    uint32_t size_in_byte = (frame_buf != NULL) ? frame_buf->bo.size : (draw_buf->size * 4);

    pRange->ui32StartAddr = (uint32_t)disp_buf;
    pRange->ui32Size = size_in_byte;
}

static void lv_ambiq_get_range_from_area(void* buffer, const lv_area_t* area, uint32_t pixel_size_in_byte, am_hal_cachectrl_range_t* pRange)
{
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    if((w<=0) && (h<=0))
    {
        pRange->ui32StartAddr = 0;
        pRange->ui32Size = 0;
    }
    else
    {
        pRange->ui32StartAddr = (uint32_t)buffer;
        pRange->ui32Size = w * h * pixel_size_in_byte;
    }
}
#endif

void lv_ambiq_cache_invalid_draw_buffer(lv_disp_t * disp)
{
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    am_hal_cachectrl_range_t range;
    lv_ambiq_get_range_from_disp(disp, &range);
    am_hal_cachectrl_dcache_invalidate(&range, false);
#endif
}

void lv_ambiq_cache_clean_draw_buffer(lv_disp_t * disp)
{
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    am_hal_cachectrl_range_t range;
    lv_ambiq_get_range_from_disp(disp, &range);
    am_hal_cachectrl_dcache_clean(&range);
#endif
}

void lv_ambiq_cache_invalid_and_clean_draw_buffer(lv_disp_t * disp)
{
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    am_hal_cachectrl_range_t range;
    lv_ambiq_get_range_from_disp(disp, &range);
    am_hal_cachectrl_dcache_invalidate(&range, true);
#endif
}

void lv_ambiq_cache_clean_mask_buffer(const lv_opa_t* mask, const lv_area_t* area)
{
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    if(mask)
    {
        am_hal_cachectrl_range_t range;
        lv_ambiq_get_range_from_area((void*)mask, area, 1, &range);
        am_hal_cachectrl_dcache_clean(&range);
    }
#endif
}

void lv_ambiq_cache_clean_map_buffer(const lv_color_t* map, const lv_area_t* area)
{
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    if(map)
    {
        am_hal_cachectrl_range_t range;
        lv_ambiq_get_range_from_area((void*)map, area, 4, &range);
        am_hal_cachectrl_dcache_clean(&range);
    }
#endif
}