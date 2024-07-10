//*****************************************************************************
//
//! @file am_hal_bootrom_helper.h
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
#ifndef AM_HAL_BOOTROM_HELPER_H
#define AM_HAL_BOOTROM_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
//  BootROM Helper Function calling documentation.
//
// ****************************************************************************
//*****************************************************************************
//
//  @brief entry point for erasing the MAIN block of the main NV region
//
//  @param value         customer program key
//
//  @return              0: success.
//                       1: invalid customer program key (value).
//                       6: NVM controller state failure.
//
// ****************************************************************************
// int nv_mass_erase(uint32_t value, uint32_t dummy_remove_me_FAL152)

//*****************************************************************************
//
//  @brief entry point for erasing one 8kB page in the MAIN block of NVRAM
//
//  Calling this function erases one 8kB page in the MAIN block of NVRAM
//
//  @param value         customer program key
//  @param PageNumber    page number of the 8K page to erase in the NV MAIN block
//
//  @return              0: success.
//                       1: invalid customer program key (value).
//                       2: invalid page page (destination start address).
//                       3: invalid page (desination ending address).
//                       6: NVM controller state failure.
//
// ****************************************************************************
// int nv_page_erase(uint32_t value,
//                   uint32_t dummy_remove_me_FAL152,
//                   uint32_t PageNumber)


//*****************************************************************************
//
//  @brief Program N words of the MAIN block of NVRAM
//
//  Calling this function programs up to N words of the MAIN block of NVRAM
//
//  @param value         customer program key
//  @param pSrc          pointer to word aligned data to program.
//                       WARNING: pSrc can not point back in to NVM itself
//  @param pDst          pointer to word aligned target location to program
//                       in the NVRAM MAIN block
//  @param               NumberOfWords number of 32-bit words to program
//
//  @return              0: success.
//                       1: invalid customer program key (value).
//                       2: invalid pDst (destination start address).
//                       3: invalid destination data region (destination end address).
//                       4: invalid pSrc (source start address).
//                       5: invalid source data region (source end address).
//                       6: NVM controller state failure.
//
//
// ****************************************************************************
// int nv_program_main(uint32_t  value,
//                     uint32_t *pSrc,
//                     uint32_t *pDst,
//                     uint32_t  NumberOfWords)

//*****************************************************************************
//
//  @brief entry point for programming the INFO0 (customer) block NVRAM
//
//  Calling this function programs multiple words in the INFO0 block on one nv instance
//
//  @param value         customer info key
//  @param pSrc          pointer to word aligned array of data to program into the NV INFO block
//  @param Offset        word offset in to info block (0x01 means the second 32 bit word)
//                       NOTE Offset == 0 --> first word in INFO0.
//  @param NumberOfWords number of words to program
//
//  @return              0: success.
//                       1: invalid customer program key (value).
//                       2: invalid Offset (destination start address).
//                       3: invalid NumberOfWords (destination end address).
//                       4: invalid pSrc (source start address).
//                       5: invalid source data region (source end address).
//                       6: NVM controller state failure.
//
// ****************************************************************************
// int nv_program_info_area(uint32_t  value,
//                          uint32_t *pSrc,
//                          uint32_t  Offset,
//                          uint32_t  NumberOfWords)

//*****************************************************************************
//
//  @brief Enhanced version of nv_program_main which passes through more of
//  the features of nv_program.
//
//  Calling this function programs up to N words of the MAIN block of NVRAM
//
//  @param value         customer program key
//  @param Program_nWipe boolean option: 1 = Addr_WipeData is the source address (pointer)
//                                           (pSrc can not point back in to NVRAM itself)
//                                       0 = Addr_WipeData is data word to write repeatedly during wipe
//  @param Addr_WipeData if Program_nWipe is 0, then single word value to write repeatedly to NVRAM
//                       if Program_nWipe is 1, then 32-bit start byte address of region to write to NVRAM
//  @param WordOffset    word offset from the start of NVRAM to program
//  @param NumberOfWords number of 32-bit words to program
//
//  @return              0: success.
//                       1: invalid customer program key (value).
//                       2: invalid WordOffset (destination start address).
//                       3: invalid NumberOfWords (destination end address).
//                       4: invalid Addr_WipeData
//                          (if Program_nWipe = NV_PROGRAM, refers to source start address).
//                          (if Program_nWipe = NV_WIPE this error should not be returned).
//                       5: invalid source data region (source end address).
//                       6: NVM controller state failure.
//
// ****************************************************************************
// int nv_program_main2(uint32_t  value,
//                      uint32_t  Program_nWipe,
//                      uint32_t  Addr_WipeData,
//                      uint32_t  WordOffset,
//                      uint32_t  NumberOfWords)

//*****************************************************************************
//
//  @brief entry point for reading one 32-bit value from specified location.
//
//  Calling this function reads the specified location and returns the 32-bit value from there.
//
//  @param pSrc          pointer to word aligned data to read.
//
//  @return              32-bit value read from specified location.
//
// ****************************************************************************
// uint32_t bootrom_util_read_word(uint32_t *pSrc)

//*****************************************************************************
//
//  @brief entry point for writing a 32-bit value from the specified location.
//
//  Calling this function writes the specified location and returns the 32-bit value from there.
//  Note that this function should NOT be used on registers on which reading may have a side-
//  effect, due to the fact that it not only flushes but reads the register back by way of
//  br_util_read_word().
//  See also JIRA FAL-37 - https://ambiqmicro.atlassian.net/browse/FAL-37
//
//  @param pDst          pointer to word aligned data to write.
//  @param value         value to write to specified location.
//
//  @return              nothing
//
// ****************************************************************************
// void bootrom_util_write_word(uint32_t *pDst, uint32_t value)

//*****************************************************************************
//
//  @brief entry point for erasing the customer INFO block of NVRAM
//
//  Calling this function erases the the customer INFO (0) block of NVRAM
//
//  @param value         customer info key
//
//  @return              0: success.
//                       1: invalid customer program key (value).
//                       6: NVM controller state failure.
//
// ****************************************************************************
// int nv_info_erase(uint32_t value)

//******************************************************************************
//
//  @brief Test for valid address range taking into aaccount the SKU memory sizes
//
//  This function returns true if the memory range is valid for the region mask supplied
//
//
//  @param pStart        pointer to the starting address of the region (void// )
//  @param size          size of the memory block
//  @param regionMask    the region specifies which regions the address ramge must be in
//
//                       valid regions are:
//                          AM_HAL_MRAM_MRAM_REGION
//                          AM_HAL_MRAM_ITCM_REGION
//                          AM_HAL_MRAM_DTCM_REGION
//                          AM_HAL_MRAM_SSRAM_REGION  // Only SSRAM
//                          AM_HAL_MRAM_SRAM_REGION   // All of SRAM (allows spanning if no hole between DTCM/TCM & SSRAM)
//
//                       any combination of regions can be supplied as a or'd mask
//
//                       the region AM_HAL_MRAM_SRAM_REGION allows the address range to in and span
//                       DTCM/SSRAM (Apollo5b)
//
//                       specify the individual regions if the address range must be fully
//                       contained in that region. the SRAM_REGION allows the address range to be
//                       contained in either region and span across if there is no memory hole per
//                       the SKU
//
//  @return              'true' if the address range is valid for the device's SKU, 'false' otherwise
//
// ****************************************************************************/
// bool valid_address_range(void *pStart, uint32_t size, uint32_t regionMask)

//*****************************************************************************
//
//  @brief entry point for counting of CPU delay cycles
//
//  @param count         number of CPU cycles to delay
//
//  @return              none
//
// ****************************************************************************
// void bootrom_delay_cycles(uint32_t ui32Cycles)

//******************************************************************************
//
//  @brief entry point for reading a number of words from the AMBIQ INFO block of NVRAM
//
//  This function operates on the AMBIQ INFO block of NVRAM
//
//  Calling this function reads words from the AMBIQ INFO block of NVRAM
//
//  @param value         value must be irrational
//  @param offset        offset into Ambiq Internal Only INFO block (in words)
//  @param number        number of words to read
//  @param pDst          pointer to where to store the data that is read from the INFO block
//
//  @return 0 for success, non-zero for failure
//
// ****************************************************************************/
// int otp_program_info_area(uint32_t  value,
//                      uint32_t *pSrc,
//                      uint32_t  Offset,
//                      uint32_t  NumberOfWords)

///******************************************************************************
//
//   @brief entry point for obtaining version information for boot loader ROM
//
//   @return      MSB 16bits -- day count since jan 1st 2000 up to todays date
//                example -- jan 1 st 2000 -- jan 17th 2023 - 8417 days
//
//                LSB 16 bits -- AM_SBR_VERSION number
//
// *****************************************************************************/
// uint32_t bootrom_version_info(void)


//*****************************************************************************
//
//! Structure of pointers to helper functions invoking flash operations.
//
//! The functions we are pointing to here are in the Apollo 4
//! integrated BOOTROM.
//
//*****************************************************************************
typedef struct am_hal_bootrom_helper_struct
{
    //
    //! MRAM Infospace programming function.
    //
    int  (*nv_program_info_area)(uint32_t, uint32_t *, uint32_t, uint32_t);

    //
    //! Helpful utilities.
    //
    int  (*nv_program_main2)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

    uint32_t (*bootrom_util_read_word)( uint32_t *);
    void (*bootrom_util_write_word)( uint32_t *, uint32_t);

    //
    //! Valid address range.
    //
    bool (*valid_address_range)(void *pStart, uint32_t size, uint32_t regionMask);
    //
    //! Cycle accurate delay function.
    //
    void (*bootrom_delay_cycles)(uint32_t ui32Cycles);

    //
    //! OTP Infospace programming function.
    //
    int  (*otp_program_info_area)(uint32_t, uint32_t*, uint32_t, uint32_t);

    //
    //! BootROM version info.
    //
    uint32_t (*bootrom_version_info)(void);

} am_hal_bootrom_helper_t;

extern const am_hal_bootrom_helper_t g_am_hal_bootrom_helper;

/******************************************************************************
 *
 * @brief entry point for programming the INFO0(customer) block NVRAM
 *
 * Calling this function programs multiple words in the INFO0 block on one nv instance
 *
 * @param value         customer info key
 * @param pSrc          pointer to word aligned array of data to program into the NV INFO block
 * @param Offset        word offset in to info block (0x01 means the second 32 bit word)
 *                      NOTE Offset == 0 --> first word in INFO0.
 * @param NumberOfWords number of words to program
 *
 * @return              0 for success, non-zero for failure
 *
 *****************************************************************************/
int
nv_program_info_area(uint32_t  value,
                     uint32_t *pSrc,
                     uint32_t  Offset,
                     uint32_t  NumberOfWords);

/******************************************************************************
 *
 * @brief Enhanced version of nv_program_main which passes through more of
 * the features of nv_program.
 *
 * Calling this function programs up to N words of the MAIN block of NVRAM
 *
 * @param value         customer program key
 * @param Program_nWipe boolean option: 1 = Addr_WipeData is the source address (pointer)
 *                                          (pSrc can not point back in to NVRAM itself)
 *                                      0 = Addr_WipeData is data word to write repeatedly during wipe
 * @param Addr_WipeData if Program_nWipe is 0, then single word value to write repeatedly to NVRAM
 *                      if Program_nWipe is 1, then 32-bit start byte address of region to write to NVRAM
 * @param WordOffset    word offset from the start of NVRAM to program
 * @param NumberOfWords number of 32-bit words to program
 *
 * @return              0 for success, non-zero for failure
 *
 *****************************************************************************/
int
nv_program_main2(uint32_t  value,
                 uint32_t  Program_nWipe,
                 uint32_t  Addr_WipeData,
                 uint32_t  WordOffset,
                 uint32_t  NumberOfWords);


#ifdef __cplusplus
}
#endif

#endif // AM_HAL_BOOTROM_HELPER_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

