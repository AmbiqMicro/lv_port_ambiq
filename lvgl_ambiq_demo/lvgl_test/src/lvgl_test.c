//*****************************************************************************
//
//! @file lvgl_test.c
//!
//! @brief LVGL base  example.
//! This example demonstrates GPU acceleration for fundamental widgets in the LVGL
//! as well as various texture formats supporting. Change the macro defines in
//! the gui_tasks.c to switch between different widgets and texture formats.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "lvgl_test.h"

#if defined(apollo510_evb)
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
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
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

mspi_device_func_t mspi_device_func =
{
#if defined(apollo510_evb)
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

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_uart_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

#ifdef CPU_RUN_IN_HP_MODE
    //
    // CPU switch to HP mode.
    //
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("CPU enter HP mode failed!\n");
    }
#endif

#ifdef MSPI_PSRAM_TIMING_CHECK
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == mspi_device_func.mspi_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
#if defined(apollo510_evb)
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

    am_mem_init();

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

