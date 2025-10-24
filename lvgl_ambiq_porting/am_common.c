//*****************************************************************************
//
//! @file ambiq_common.c
//!
//! @brief This file implements the initialization common part
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
//*****************************************************************************
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "lvgl.h"
#include "am_common.h"
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


#if defined(apollo510_evb) || defined(apollo510b_evb)
#include "am_devices_mspi_psram_aps25616ba_1p2v.h"
#else
#include "am_devices_mspi_psram_aps25616n.h"
#endif

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
typedef struct
{
    uint8_t  devName[30];

    uint32_t (*mspi_init)(uint32_t ui32Module,
                          am_devices_mspi_psram_config_t *psMSPISettings,
                          void **ppHandle,
                          void **ppMspiHandle);

    uint32_t (*mspi_term)(void *pHandle);

    uint32_t (*mspi_read_id)(void *pHandle);

    uint32_t (*mspi_read)(void *pHandle, uint8_t *pui8RxBuffer,
                          uint32_t ui32ReadAddress,
                          uint32_t ui32NumBytes,
                          bool bWaitForCompletion);

    uint32_t (*mspi_read_adv)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_read_callback)(void *pHandle, uint8_t *pui8RxBuffer,
                                   uint32_t ui32ReadAddress,
                                   uint32_t ui32NumBytes);

    uint32_t (*mspi_write)(void *pHandle, uint8_t *ui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_write_adv)(void *pHandle,
                               uint8_t *puiTxBuffer,
                               uint32_t ui32WriteAddress,
                               uint32_t ui32NumBytes,
                               uint32_t ui32PauseCondition,
                               uint32_t ui32StatusSetClr,
                               am_hal_mspi_callback_t pfnCallback,
                               void *pCallbackCtxt);

    uint32_t (*mspi_mass_erase)(void *pHandle);
    uint32_t (*mspi_sector_erase)(void *pHandle, uint32_t ui32SectorAddress);
    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);
    uint32_t (*mspi_scrambling_enable)(void *pHandle);
    uint32_t (*mspi_scrambling_disable)(void *pHandle);

    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                       am_devices_mspi_psram_config_t *pDevCfg,
                                       am_devices_mspi_psram_ddr_timing_config_t *pDevSdrCfg);

    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                       am_devices_mspi_psram_ddr_timing_config_t *pDevSdrCfg);
} mspi_device_func_t;

//static uint32_t        ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

/* defined on linker script file */
extern uint32_t __external_start;
extern uint32_t __external_end;
extern uint32_t __external_load_start;

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);
}

mspi_device_func_t mspi_device_func =
{
#if defined(apollo510_evb) || defined(apollo510b_evb)
    .devName = "MSPI PSRAM APS25616BA",
    .mspi_init = am_devices_mspi_psram_aps25616ba_ddr_init,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616ba_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616ba_apply_ddr_timing,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616ba_ddr_enable_xip,
#else
    .devName = "MSPI PSRAM APS25616N",
    .mspi_init = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616n_apply_ddr_timing,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
#endif
};

//*****************************************************************************
//
// Init GPU
//
//*****************************************************************************
void
am_gpu_init(void)
{
#ifdef GPU_RUN_IN_HP_MODE
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    //Switch to HP mode.
    //
    am_hal_pwrctrl_gpu_mode_e current_mode;
    am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if ( AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != current_mode )
    {
        am_util_stdio_printf("gpu switch to HP mode failed!\n");
    }
#endif

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
#if LV_USE_AMBIQ_VG
    nema_buffer_t stencil_buffer = {.base_phys = 0, .base_virt = 0, .size = 0, .fd = 0};
    nema_vg_init_stencil_prealloc(LV_AMBIQ_DISPLAY_BUFFER_RESX, LV_AMBIQ_DISPLAY_BUFFER_RESY, stencil_buffer);
    if (NEMA_ERR_NO_ERROR != nema_get_error())
    {
        am_util_stdio_printf("NemaVG init failed!\n");
    }
#endif

#if LV_AMBIQ_GPU_POWER_SAVE 
    //
    // If LV_AMBIQ_GPU_POWER_SAVE is defined, we keep GPU power off until an GPU CL is ready to submit.
    //
    nemagfx_power_control(AM_HAL_SYSCTRL_DEEPSLEEP, true);
#endif
}

//*****************************************************************************
//
// Init MSPI
//
//*****************************************************************************
void am_mspi_init(void)
{
    uint32_t ui32Status;
    #ifdef MSPI_PSRAM_TIMING_CHECK
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == mspi_device_func.mspi_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
#if defined(apollo510_evb) || defined(apollo510b_evb)
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.sTimingCfg.ui8RxDQSDelay);
#else
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
#endif
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = mspi_device_func.mspi_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }

#ifdef MSPI_PSRAM_TIMING_CHECK
    //
    // Apply DDR timing setting
    //
    ui32Status = mspi_device_func.mspi_init_timing_apply(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = mspi_device_func.mspi_xip_enable(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
}


//*****************************************************************************
//
// Relocate initialized data to PSRAM from its load location in MRAM
//
// This function should be called during system startup. It copies sections
// whose load address is in MRAM but whose execution (link) address is in PSRAM.
//
//*****************************************************************************
void am_relocate_init_data_to_psram(void)
{
    // Relocate image assets and font bitmaps from MRAM to PSRAM for direct GPU access.
    uint32_t ui32ExternalStart = 0;
    uint32_t ui32TextureSectionLength = 0;
    uint32_t ui32CodeSectionLoadAddr = 0;
    ui32ExternalStart = (uint32_t)&__external_start;
    ui32TextureSectionLength = (uint32_t)&__external_end - (uint32_t)&__external_start;
    ui32CodeSectionLoadAddr = (uint32_t)&__external_load_start;
    memcpy((void * )ui32ExternalStart, (const void * )ui32CodeSectionLoadAddr, ui32TextureSectionLength);

    // Clean the cache for the texture section.
    am_hal_cachectrl_range_t Range;
    Range.ui32Size = ui32TextureSectionLength;
    Range.ui32StartAddr = ui32ExternalStart;
    am_hal_cachectrl_dcache_clean(&Range);
}