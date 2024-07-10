//*****************************************************************************
//
//! @file am_hal_bootrom_helper.c
//!
//! @brief BootROM Helper Function Table
//!
//! @addtogroup bootrom4 Bootrom Functionality
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

//
//! Look-up table
//
const am_hal_bootrom_helper_t g_am_hal_bootrom_helper =
{
         ((int  (*)(uint32_t, uint32_t *, uint32_t, uint32_t))                      (0x0200ff0c + 1)),  // nv_program_info_area
         ((int  (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))              (0x0200ff20 + 1)),  // nv_program_main2
         ((uint32_t (*)(uint32_t *))                                                (0x0200ff28 + 1)),  // br_util_read_word
         ((void (*)(uint32_t *, uint32_t))                                          (0x0200ff2c + 1)),  // br_util_write_word
         ((bool (*) (void *, uint32_t, uint32_t))                                   (0x0200ff40 + 1)),  // valid_address_range
         ((void (*)(uint32_t ))                                                     (0x0200ff50 + 1)),  // br_util_delay_cycles
         ((int  (*)(uint32_t, uint32_t*, uint32_t, uint32_t))                       (0x0200ff54 + 1)),  // otp_program_info_area
         ((uint32_t (*)(void))                                                      (0x0200ff5c + 1)),  // br_util_rom_version
};

int
nv_program_info_area(uint32_t  value,
                     uint32_t *pSrc,
                     uint32_t  Offset,
                     uint32_t  NumberOfWords)
{
    int iRet = g_am_hal_bootrom_helper.nv_program_info_area(value, pSrc, Offset, NumberOfWords);
    AM_REGVAL(0x40014008) = 0xC3;
    AM_REGVAL(0x40014024) = 0;
    AM_REGVAL(0x40014008) = 0;
    return iRet;
}

int
nv_program_main2(uint32_t  value,
                 uint32_t  Program_nWipe,
                 uint32_t  Addr_WipeData,
                 uint32_t  WordOffset,
                 uint32_t  NumberOfWords)
{
    int iRet = g_am_hal_bootrom_helper.nv_program_main2(value, Program_nWipe, Addr_WipeData, WordOffset, NumberOfWords);
    AM_REGVAL(0x40014008) = 0xC3;
    AM_REGVAL(0x40014024) = 0;
    AM_REGVAL(0x40014008) = 0;
    return iRet;
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
