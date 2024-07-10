//*****************************************************************************
//
//! @file am_hal_mram.c
//!
//! @brief MRAM and ROM Helper Function Access
//!
//! @addtogroup mram4 MRAM Functionality
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
#include "am_hal_bootrom_helper.h"

#define AM_HAL_MRAM_WIPE                   0
#define AM_HAL_MRAM_PROGRAM                1


//*****************************************************************************
//
// This programs up to N words of the Main MRAM
//
//*****************************************************************************
uint32_t
am_hal_mram_main_words_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                               uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    uint32_t    ui32Status;

    //
    // Check that the pui32Dst is word aligned
    if ( (uint32_t)pui32Dst & 0x3 )
    {
        return AM_HAL_MRAM_INVLD_ADDR_ALIGNMENT;
    }

    //
    // This helper function requires a word offset rather than an actual address.
    //
    pui32Dst = (uint32_t*)(((uint32_t)pui32Dst - AM_HAL_MRAM_ADDR) >> 2);

    AM_CRITICAL_BEGIN

    //
    // Enable the ROM for helper function.
    //
    am_hal_pwrctrl_rom_enable();

    ui32Status = nv_program_main2 (ui32ProgramKey, AM_HAL_MRAM_PROGRAM, (uint32_t)pui32Src, (uint32_t)pui32Dst, ui32NumWords);

    //
    // Disable the ROM.
    //
    am_hal_pwrctrl_rom_disable();

    AM_CRITICAL_END

    //
    // Return the status.
    //
    return ( !ui32Status ) ? ui32Status : ( ui32Status | AM_HAL_MRAM_ERROR );
} // am_hal_mram_main_words_program()

//*****************************************************************************
//
// This programs up to N words of the Main MRAM
//
//*****************************************************************************
uint32_t
am_hal_mram_main_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                         uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    //
    // Check for 16 byte aligned pui32Dst & ui32NumWords
    //
    if ( ((uint32_t)pui32Dst & 0xf) || (ui32NumWords & 0x3) )
    {
        return AM_HAL_MRAM_INVLD_ADDR_ALIGNMENT;
    }

    return am_hal_mram_main_words_program(ui32ProgramKey, pui32Src,
                                          pui32Dst, ui32NumWords);
} // am_hal_mram_main_program()

//*****************************************************************************
//
// This Fills up to N words of the Main MRAM
//
//*****************************************************************************
uint32_t
am_hal_mram_main_fill(uint32_t ui32ProgramKey, uint32_t ui32Value,
                      uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    uint32_t    ui32Status;
    //
    // Check for 16 byte aligned pui32Dst & ui32NumWords
    //
    if ( ((uint32_t)pui32Dst & 0xf) || (ui32NumWords & 0x3) )
    {
        return AM_HAL_MRAM_INVLD_ADDR_ALIGNMENT;
    }

    //
    // This helper function requires a word offset rather than an actual address.
    //
    pui32Dst = (uint32_t*)(((uint32_t)pui32Dst - AM_HAL_MRAM_ADDR) >> 2);

    AM_CRITICAL_BEGIN

    //
    // Enable the ROM for helper functions.
    //
    am_hal_pwrctrl_rom_enable();

    ui32Status = nv_program_main2 (ui32ProgramKey, AM_HAL_MRAM_WIPE, (uint32_t)ui32Value, (uint32_t)pui32Dst, ui32NumWords);

    //
    // Disable the ROM.
    //
    am_hal_pwrctrl_rom_disable();

    AM_CRITICAL_END

    return ( !ui32Status ) ? ui32Status : ( ui32Status | AM_HAL_MRAM_ERROR );
} // am_hal_mram_main_fill()

//*****************************************************************************
//
// Initialize MRAM for DeepSleep.
//
//*****************************************************************************
uint32_t
am_hal_mram_ds_init(void)
{

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_mram_ds_init()


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
