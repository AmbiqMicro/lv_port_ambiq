//*****************************************************************************
//
//! @file lvgl_cellular_menu.c
//!
//! @brief LVGL Cellular Menu with Touch Support Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup lvgl_cellular_menu LVGL Cellular Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates LVGL cellular menu interface with
//! extensive icon assets loaded from MRAM into PSRAM. During animation,
//! the GPU fetches data from PSRAM, significantly reducing SSRAM usage
//! while maintaining high performance. The application includes touch
//! support for interactive menu navigation and displays PFS (Performance)
//! metrics at the bottom of the screen.
//!
//! @section lvgl_cellular_menu_features Key Features
//!
//! 1. @b Cellular @b Menu @b Interface: Implements a modern cellular-style
//!    menu with smooth animations and transitions
//!
//! 2. @b Icon @b Asset @b Management: Loads large icon assets from MRAM to
//!    PSRAM for efficient memory usage
//!
//! 3. @b Touch @b Support: Provides full touch interaction for menu
//!    navigation and selection
//!
//! 4. @b Performance @b Monitoring: Displays PFS (Performance) metrics
//!    for real-time performance analysis
//!
//! 5. @b GPU @b Acceleration: Utilizes GPU for efficient icon rendering
//!    and animation processing
//!
//! @section lvgl_cellular_menu_hardware Hardware Requirements
//!
//! - Compatible Development Board
//! - Touch-enabled display panel
//! - PSRAM for large icon assets
//!
//! @section lvgl_cellular_menu_usage Usage
//!
//! The application automatically displays a cellular menu with touch
//! support. Performance metrics are shown at the bottom of the screen.
//! Use touch gestures to navigate the menu interface.
//!
//! @note Icon assets are loaded from MRAM to PSRAM for memory efficiency
//!
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "lvgl_cellular_menu.h"

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
#if defined( AM_PART_APOLLO510 ) 
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("CPU enter HP mode failed!\n");
    }
#elif defined( AM_PART_APOLLO510L )
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE2) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("CPU enter HP mode failed!\n");
    }
#endif
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

