//*****************************************************************************
//
//! @file lvgl_test.c
//!
//! @brief LVGL Test example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup lvgl_test LVGL Test Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates how to use LVGL.
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
//
//
//*****************************************************************************

#include "lvgl_test.h"
#if defined(AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    #include "am_devices_mspi_psram_aps12808l.h"
    #include "am_devices_mspi_psram_aps12808l.c"
#elif defined(AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    #include "am_devices_mspi_psram_aps25616n.h"
    #include "am_devices_mspi_psram_aps25616n.c"
#else // Note that this #else would be needed only once in each driver.
    #error PSRAM device was not defined.
#endif

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static uint32_t        ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

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

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

#ifdef CONFIG_DAXI
    //
    //  am_bsp_low_power_init will config DAXI with the default setting,
    //  If you need a special setting, set it here.
    //
    am_hal_daxi_config_t sDAXIConfig =
    {
        .bDaxiPassThrough         = false,
        .bAgingSEnabled           = true,
        .eAgingCounter            = AM_HAL_DAXI_CONFIG_AGING_1024,
        .eNumBuf                  = AM_HAL_DAXI_CONFIG_NUMBUF_32,
        .eNumFreeBuf              = AM_HAL_DAXI_CONFIG_NUMFREEBUF_3,
    };
    am_hal_daxi_config(&sDAXIConfig);
    am_hal_delay_us(100);
#endif

    //
    //  Set up the cache.
    //
    am_hal_cachectrl_config_t sCacheConfig =
    {
        .bLRU                       = true,
        .eDescript                  = AM_HAL_CACHECTRL_DESCR_2WAY_128B_2048E,
        .eMode                      = AM_HAL_CACHECTRL_CONFIG_MODE_INSTR_DATA,
    };
    am_hal_cachectrl_config(&sCacheConfig);
    am_hal_cachectrl_enable();

#ifdef USE_EXTENDED_RAM
    //
    // Config DSP ram
    //
    am_hal_pwrctrl_dsp_memory_config_t      DSPMemCfg =
    {
        .bEnableICache      = true,
        .bRetainCache       = false,
        .bEnableRAM         = true,
        .bActiveRAM         = false,
        .bRetainRAM         = true
    };
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &DSPMemCfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &DSPMemCfg);
#endif

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

#ifndef APOLLO4_FPGA
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
#if defined (AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
#elif defined (AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps12808l_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
#endif
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_debug_printf("==== Scan Result: Failed, no valid setting.  \n");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
#if defined (AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#elif defined (AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

#ifndef APOLLO4_FPGA
    //
    // Apply DDR timing setting
    //
#if defined (AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
#elif defined (AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    ui32Status = am_devices_mspi_psram_aps12808l_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }
#endif

    //
    // Enable XIP mode.
    //
#if defined (AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
#elif defined (AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPsramHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    // Init LVGL.
    lv_init();

    //If LVGL GPU acceleration feature is enabled, NemaGFX will be initialized in lv_init.
    //If not enabled, GPU is still needed for GPU_memcpy in display tasks.
#if !LV_USE_GPU_AMBIQ_NEMA
    //Power on GPU
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //Initialize NemaGFX.
    int ret = nema_init();
    if (ret != 0)
    {
        am_util_stdio_printf("Ambiq GPU init failed!\n");
    }
#endif

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("lvgl_test Example\n");

    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

