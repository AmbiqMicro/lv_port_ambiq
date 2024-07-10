//*****************************************************************************
//
//! @file am_hal_systick.c
//!
//! @brief Functions for interfacing with the SYSTICK
//!
//! @addtogroup systick4 SYSTICK - System Tick Timer
//! @ingroup apollo5b_hal
//! @{
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
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SYSTICK_MAX_TICKS   ((1 << 24)-1)
#define MAX_U32             (0xffffffff)

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
am_hal_systick_t g_SysTickClockSource = AM_HAL_SYSTICK_CLKSRC_EXT;

uint32_t
am_hal_systick_init(am_hal_systick_t eClockSource)
{
    //
    // Check if external is implemented and return error if it is not allowed.
    //
    if ((SysTick->CALIB & SysTick_CALIB_NOREF_Msk) && (AM_HAL_SYSTICK_CLKSRC_EXT == eClockSource))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Save the configured clock source.
    //
    g_SysTickClockSource = eClockSource;

    //
    // Set the desired clock source.
    //
    SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= _VAL2FLD(SysTick_CTRL_CLKSOURCE, eClockSource);


    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Start the SYSTICK.
//
// This function starts the systick timer.
//
//*****************************************************************************
void
am_hal_systick_start(void)
{
    //
    // Start the systick timer.
    //
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

//*****************************************************************************
//
// Stop the SYSTICK.
//
// This function stops the systick timer.
//
//*****************************************************************************
void
am_hal_systick_stop(void)
{
    //
    // Stop the systick timer.
    //
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//*****************************************************************************
//
// Enable the interrupt in the SYSTICK.
//
// This function enables the interupt in the systick timer.
//
//*****************************************************************************
void
am_hal_systick_int_enable(void)
{
    //
    // Enable the systick timer interrupt.
    //
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

//*****************************************************************************
//
// Disable the interrupt in the SYSTICK.
//
// This function disables the interupt in the systick timer.
//
//*****************************************************************************
void
am_hal_systick_int_disable(void)
{
    //
    // Disable the systick timer interrupt.
    //
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

//*****************************************************************************
//
// Reads the interrupt status.
//
// This function reads the interrupt status in the systick timer.
//
//*****************************************************************************
uint32_t
am_hal_systick_int_status_get(void)
{
    //
    // Return the systick timer interrupt status.
    //
    return SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
}

//*****************************************************************************
//
// Reset the interrupt in the SYSTICK.
//
// This function resets the systick timer by clearing out the configuration
// register.  This requires a new call to am_hal_systick_init().
//
//*****************************************************************************
void
am_hal_systick_reset(void)
{
    //
    // Reset the systick timer interrupt.
    //
    SysTick->CTRL = 0x0;
}

//*****************************************************************************
//
// Load the value into the SYSTICK.
//
// This function loads the desired value into the systick timer.
//
//*****************************************************************************
void
am_hal_systick_load(uint32_t ui32LoadVal)
{
    //
    // Write the given value to the reload register.
    //
    SysTick->LOAD = ui32LoadVal;

    //
    // Write the Current Value Register to clear it to 0.
    //
    SysTick->VAL = 0;
}

//*****************************************************************************
//
// Get the current count value in the SYSTICK.
//
// This function gets the current count value in the systick timer.
//
//*****************************************************************************
uint32_t
am_hal_systick_count(void)
{
    //
    // Return the current systick timer count value.
    //
    return SysTick->VAL;
}

//*****************************************************************************
//
// Wait the specified number of ticks.
//
// This function delays for the given number of SysTick ticks.
//
//*****************************************************************************
uint32_t
am_hal_systick_wait_ticks(uint32_t ui32Ticks)
{

    if ( ui32Ticks == 0 )
    {
        ui32Ticks++;                // Make sure we get the COUNTFLAG
    }

    //
    // Set the reload value to the required number of ticks.
    //
    SysTick->LOAD = ui32Ticks;

    //
    // Clear the current count.
    //
    SysTick->VAL = 0x0;

    //
    // Set to use the processor clock, but don't cause an exception (we'll poll).
    //
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    //
    // Poll till done
    //
    while ( !(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) );

    //
    // And disable systick before exiting preserving the desired clock source.
    //
    SysTick->CTRL = _VAL2FLD(SysTick_CTRL_CLKSOURCE, g_SysTickClockSource);


    return 0;
}

//*****************************************************************************
//
// Return the number of ticks per millisecond.
//
//*****************************************************************************
uint32_t
am_hal_systick_ticks_per_ms(uint32_t *TicksPerMs)
{
    am_hal_pwrctrl_mcu_mode_e   eCurrentMcuMode;
    uint32_t                    retval = AM_HAL_STATUS_SUCCESS;
    uint32_t                    val;

    //
    // Check if we are running on internal (core) or external clock reference.
    //
    switch(g_SysTickClockSource)
    {
        case AM_HAL_SYSTICK_CLKSRC_EXT:
            val = SysTick->CALIB;
            //
            // TENMS field must be programmend to be one less than the number of cycles that make up the 10ms period.
            // so increment TENMS value by 1 is actual 10ms period.
            //
            *TicksPerMs = (_FLD2VAL(SysTick_CALIB_TENMS, val) + 1) / 10;
            break;

        case AM_HAL_SYSTICK_CLKSRC_INT:
            // Using internal (core) clock.
            retval = am_hal_pwrctrl_mcu_mode_status(&eCurrentMcuMode);
            if (AM_HAL_STATUS_SUCCESS == retval)
            {
                switch(eCurrentMcuMode)
                {
                    case AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER:
                        *TicksPerMs = AM_HAL_CLKGEN_FREQ_MAX_HZ / 1000;
                        break;
                    case AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE:
                        *TicksPerMs = AM_HAL_CLKGEN_FREQ_HP250_HZ / 1000;
                        break;
                }
            }
            else
            {
                return retval;
            }
            break;
    }

    return retval;
}


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
