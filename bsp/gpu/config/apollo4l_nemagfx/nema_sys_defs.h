// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#ifndef NEMA_SYS_DEFS_H__
#define NEMA_SYS_DEFS_H__

//Start of User Definitions
//-------------------------

#define DONT_SUPPORT_NEMATS
//#define NEMA_MULTI_PROCESS
//#define NEMA_MULTI_THREAD

//use multiple memory pools (implemented in nema_hal.c)
//#define NEMA_MULTI_MEM_POOLS

//if NEMA_MULTI_MEM_POOLS is defined, use NEMA_MULTI_MEM_POOLS_CNT pools
//must be equal or less than 4
#ifndef NEMA_MULTI_MEM_POOLS_CNT
#define NEMA_MULTI_MEM_POOLS_CNT    1
#endif

//Memory pool definition
#define NEMA_MEM_POOL_CL_RB         0
#define NEMA_MEM_POOL_FB_TEX        1
#define NEMA_MEM_POOL_MISC          2

//Alias for the memory pool. These are defined to provide backward compatibility.
#define NEMA_MEM_POOL_CL            NEMA_MEM_POOL_CL_RB
#define NEMA_MEM_POOL_FB            NEMA_MEM_POOL_FB_TEX
#define NEMA_MEM_POOL_ASSETS        NEMA_MEM_POOL_FB_TEX

//End of User Definitions
//-------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "am_hal_status.h"

#if defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD)

#endif

#ifdef NEMA_MULTI_THREAD

#ifdef __STDC_NO_THREADS__
#define TLS_VAR __thread
#else
// https://en.cppreference.com/w/c/thread
// If the macro constant __STDC_NO_THREADS__(C11) is defined by the
// compiler, the header <threads.h> and all of the names listed here
// are not provided.
// YE: Some compilers implementing C11 don't define __STDC_NO_THREADS__
// but still don't provide <threads.h>
#include <threads.h>
#define TLS_VAR thread_local
#endif

#else
#define TLS_VAR
#endif

// turn off printf, or map to am_util_stdio_printf
#define printf(...)


//
// declaration of display controller interrupt callback function.
//
typedef void (*nema_gfx_interrupt_callback)(int);
//*****************************************************************************
//
//! @brief Declaration of the GFX interrupt callback initialize function
//!
//! @param  fnGFXCallback                - GFX interrupt callback function
//!
//! this function hooks the Nema GFX GPU interrupt with a callback function.
//!
//! The fisrt paramter to the callback is a volatile int containing the ID of
//! the last serviced command list. This is useful for quickly responding
//! to the completion of an issued CL.
//!
//! @return None.
//
//*****************************************************************************
void nemagfx_set_interrupt_callback(nema_gfx_interrupt_callback fnGFXCallback);

//*****************************************************************************
//
//! @brief Check wether the core ring buffer is full or not
//!
//! @return True, the core ring buffer is full, we need wait for GPU before 
//!         submit the next CL.
//!         False, the core ring buffer is not full, we can submit the next CL.
//*****************************************************************************
extern bool nema_rb_check_full(void);

//*****************************************************************************
//
//! @brief nema_get_cl_status
//!
//!
//! @return 32-bit am_hal_status_e status
//! @return AM_HAL_STATUS_IN_USE means GPU still busy in processing
//! @return AM_HAL_STATUS_SUCCESS means GPU is complete all operation
//
//*****************************************************************************
am_hal_status_e nema_get_cl_status(int32_t cl_id);

//*****************************************************************************
//
//! GPU power ctrl enums.
//
//*****************************************************************************
typedef enum
{
    AM_GPU_PWRCTRL_DMA_ADC,
    AM_GPU_PWRCTRL_DMA_I2S_0,
    AM_GPU_PWRCTRL_DMA_I2S_1,

    AM_GPU_PWRCTRL_DMA_IOM_0,
    AM_GPU_PWRCTRL_DMA_IOM_1,
    AM_GPU_PWRCTRL_DMA_IOM_2,
    AM_GPU_PWRCTRL_DMA_IOM_3,   
    AM_GPU_PWRCTRL_DMA_IOM_4,
    AM_GPU_PWRCTRL_DMA_IOM_5,
    AM_GPU_PWRCTRL_DMA_IOM_6,
    AM_GPU_PWRCTRL_DMA_IOM_7,   

    AM_GPU_PWRCTRL_DMA_MSPI_0,
    AM_GPU_PWRCTRL_DMA_MSPI_1,
    AM_GPU_PWRCTRL_DMA_MSPI_2,

    AM_GPU_PWRCTRL_DMA_PDM_0,
    AM_GPU_PWRCTRL_DMA_PDM_1,
    AM_GPU_PWRCTRL_DMA_PDM_2,
    AM_GPU_PWRCTRL_DMA_PDM_3,   

    AM_GPU_PWRCTRL_DMA_SDIO,

    AM_GPU_PWRCTRL_CQ_IOM_0,
    AM_GPU_PWRCTRL_CQ_IOM_1,
    AM_GPU_PWRCTRL_CQ_IOM_2,
    AM_GPU_PWRCTRL_CQ_IOM_3,   
    AM_GPU_PWRCTRL_CQ_IOM_4,
    AM_GPU_PWRCTRL_CQ_IOM_5,
    AM_GPU_PWRCTRL_CQ_IOM_6,
    AM_GPU_PWRCTRL_CQ_IOM_7,

    AM_GPU_PWRCTRL_CQ_MSPI_0,
    AM_GPU_PWRCTRL_CQ_MSPI_1,
    AM_GPU_PWRCTRL_CQ_MSPI_2,

    AM_GPU_PWRCTRL_USER ,
    AM_GPU_PWRCTRL_MAX = 32,
} am_gpu_powerctrl_e;

//*****************************************************************************
//
//! @brief this function used to enable GPU power
//!
//! @param gfxPowerCtrl  GPU power ctrl enums.
//!
//! @note Turn on GPU when any DMA transaction starts running, or when the user 
//!       requests to turn on GPU
//!
//
//*****************************************************************************
extern void am_gpu_power_enable(uint32_t gfxPowerCtrl);

//*****************************************************************************
//
//! @brief this function is for the user to disable GPU power
//!
//! @note the actual power off is not done here, this function sets a flag
//!       to indicate the user wants to do turn off GPU
//!
//! @param gfxPowerCtrl - the bit to set for GFX power control 
//!
//! @return none
//
//*****************************************************************************
extern void am_gpu_power_disable(uint32_t gfxPowerCtrl);


#ifndef BAREMETAL
void gpu_power_check_and_set_task(void);
#endif
/** Suppress warning message for MDKv5.27 and later
 *
 * warning:  #61-D: integer operation result is out of range
 * warning:  #1295-D: Deprecated declaration nema_rand - give arg types
 * warning:  #1-D: last line of file ends without a newline
 * waiting ThinkSilicon release the new Nema SDK
 */

#if defined(__CC_ARM)
#pragma diag_suppress 61
#pragma diag_suppress 1295
#pragma diag_suppress 1
#endif

#endif
