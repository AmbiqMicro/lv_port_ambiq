//*****************************************************************************
//
//! @file lvgl_ttf.c
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

#include "lvgl_ttf.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

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

    //
    // Init mspi.
    //
    am_mspi_init();

    //
    // Init memory heap.
    //
    am_mem_heap_init();

    //
    // Relocate image assets and font bitmaps from MRAM to PSRAM
    //
    am_relocate_init_data_to_psram();

    //
    // Init GPU.
    //
    am_gpu_init();

    //
    // Init debug pin.
    //
    am_debug_pin_init();

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

