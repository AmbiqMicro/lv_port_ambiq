//*****************************************************************************
//
//! @file lvgl_smartwatch.c
//!
//! @brief LVGL base example.
//! This example implements the main logic for a smartwatch demo based on LVGL,
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "lvgl_smartwatch.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

void lv_ambiq_log_printf(lv_log_level_t level, const char * buf)
{
    g_pfnCharPrint(buf);
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
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
    am_bsp_itm_printf_enable();

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

    //
    // Init mspi.
    //
    am_mspi_init();

    //
    // Init memory heap.
    //
    am_mem_init();

    //
    // Relocate image assets and font bitmaps from MRAM to PSRAM
    //
    am_external_data_load();

    //
    // Init GPU.
    //
    am_gpu_init();

    //
    // Init LVGL.
    //
    lv_init();

    lv_tick_set_cb(xTaskGetTickCount);

#if LV_USE_LOG == 1
    lv_log_register_print_cb(lv_ambiq_log_printf);
#endif

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("lvgl_smartwatch Example\n");

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

