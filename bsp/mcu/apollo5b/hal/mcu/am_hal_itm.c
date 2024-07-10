//*****************************************************************************
//
//! @file am_hal_itm.c
//!
//! @brief Functions for operating the instrumentation trace macrocell
//!
//! @addtogroup itm4 ITM - Instrumentation Trace Macrocell
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

#define ITM_LAR_KEYVAL  0xC5ACCE55

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
// Enables the ITM
//
//*****************************************************************************
uint32_t
am_hal_itm_enable(void)
{
    uint32_t ui32Ret;

    {
        //
        // Workaround to avoid hanging during initialization.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    //
    // To be able to access ITM registers, set the Trace Enable bit
    // in the Debug Exception and Monitor Control Register (DEMCR).
    //
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;

    //
    // Wait for the changes to take effect with a 10us timeout.
    //
    ui32Ret = am_hal_delay_us_status_change(10,
                                            (uint32_t)&DCB->DEMCR,
                                            DCB_DEMCR_TRCENA_Msk,
                                            DCB_DEMCR_TRCENA_Msk );
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Ret;
    }

    //
    // Write the key to the ITM Lock Access register to unlock the ITM_TCR.
    //
    ITM->LAR = ITM_LAR_KEYVAL;

    //
    // Set the enable bits in the ITM trace enable register, and the ITM
    // control registers to enable trace data output.
    //
    ITM->TPR = 0x0000000F;
    ITM->TER = 0xFFFFFFFF;

    //
    // Write to the ITM control and status register.
    //
    ITM->TCR =
        _VAL2FLD(ITM_TCR_TRACEBUSID, 0x15)      |
        _VAL2FLD(ITM_TCR_GTSFREQ, 1)            |
        _VAL2FLD(ITM_TCR_TSPRESCALE, 1)         |
        _VAL2FLD(ITM_TCR_STALLENA, 0)           |
        _VAL2FLD(ITM_TCR_SWOENA, 1)             |
        _VAL2FLD(ITM_TCR_DWTENA, 0)             |
        _VAL2FLD(ITM_TCR_SYNCENA, 0)            |
        _VAL2FLD(ITM_TCR_TSENA, 0)              |
        _VAL2FLD(ITM_TCR_ITMENA, 1);

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_itm_enable()

//*****************************************************************************
//
// Disables the ITM
//
//*****************************************************************************
uint32_t
am_hal_itm_disable(void)
{
    uint32_t ui32Ret;

    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    //
    // Before disabling, make sure all printing has completed,
    // and also that the ITM is not busy.
    //
    if ( !(am_hal_itm_print_not_busy() && am_hal_itm_not_busy()) )
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // Unlock the ITM_TCR.
    //
    ITM->LAR = ITM_LAR_KEYVAL;

    //
    // Disable SWO and the ITM.
    //
    ITM->TCR &= ~ITM_TCR_SWOENA_Msk;
    ITM->TCR &= ~ITM_TCR_ITMENA_Msk;

    //
    // Wait for the changes to take effect with a 1ms timeout.
    //
    ui32Ret = am_hal_delay_us_status_change(1000,
                                            (uint32_t)&ITM->TCR,
                                            (ITM_TCR_ITMENA_Msk & ITM_TCR_BUSY_Msk),
                                            0 );
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Ret;     // AM_HAL_STATUS_TIMEOUT
    }

    //
    // Reset the TRCENA bit in the DEMCR register, which should disable the ITM
    // for operation.
    //
    DCB->DEMCR &= ~DCB_DEMCR_TRCENA_Msk;

    //
    // Wait for the changes to take effect with a 10us timeout.
    //
    ui32Ret = am_hal_delay_us_status_change(10,
                                            (uint32_t)&DCB->DEMCR,
                                            DCB_DEMCR_TRCENA_Msk,
                                            0 );
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Ret;     // AM_HAL_STATUS_TIMEOUT
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_itm_disable()

//*****************************************************************************
//
// Checks if itm is busy and provides a delay to flush the fifo
// Return: true  = ITM not busy and no timeout occurred.
//         false = Timeout occurred.
//
//*****************************************************************************
bool
am_hal_itm_not_busy(void)
{
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    //
    // Make sure the ITM is not busy.
    //
    uint32_t ui32Ret;
    ui32Ret = am_hal_delay_us_status_change(1000,
                                            (uint32_t)&ITM->TCR,
                                            ITM_TCR_BUSY_Msk,
                                            0 );
    //
    // Short wait for any data to flush out.
    //
    am_hal_delay_us(50);

    return (ui32Ret == AM_HAL_STATUS_SUCCESS) ? true : false;

} // am_hal_itm_not_busy()

//*****************************************************************************
//
// Enables tracing on a given set of ITM ports
//
//*****************************************************************************
void
am_hal_itm_trace_port_enable(uint8_t ui8portNum)
{
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    ITM->TPR |= (0x00000001 << (ui8portNum>>3));
} // am_hal_itm_trace_port_enable()

//*****************************************************************************
//
// Disable tracing on the given ITM stimulus port.
//
//*****************************************************************************
void
am_hal_itm_trace_port_disable(uint8_t ui8portNum)
{
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    ITM->TPR &= ~(0x00000001 << (ui8portNum >> 3));
} // am_hal_itm_trace_port_disable()

//*****************************************************************************
//
// Poll the given ITM stimulus register until not busy.
// Returns true if not busy, false on timeout (not ready).
//
//*****************************************************************************
bool
am_hal_itm_stimulus_not_busy(uint32_t ui32StimReg)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Waiting until port is available for writing, non-zero means ready to
    // accept additional characters.
    // Note that this doesn't necessarily mean the the port has emptied, and
    // there doesn't seem to be a way to determine that, so unfortunately this
    // function is a bit of a misnomer.
    //
    // M55 ITM stimulus registers on read:
    //  Bit1 DISABLED:  0=Stimulus port and ITM are enabled.
    //                  1=Stimulus port or ITM are disabled.
    //  Bit0 FIFOREADY: 0=Stimulus port cannot accept data,
    //                  1=Stimulus port can accept at least one word.
    //
    // Since this could likely occur while characters are still being output,
    // a hefty timeout period needs to be applied.
    //
    uint32_t ui32Ret;
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    ui32Ret = am_hal_delay_us_status_change(1000,
                                            ui32StimAddr,
                                            ITM_STIM_DISABLED_Msk | ITM_STIM_FIFOREADY_Msk,
                                            ITM_STIM_FIFOREADY_Msk );

    return (ui32Ret == AM_HAL_STATUS_SUCCESS) ? true : false;
} // am_hal_itm_stimulus_not_busy()

//*****************************************************************************
//
// Writes a 32-bit value to the given ITM stimulus register.
//
//*****************************************************************************
void
am_hal_itm_stimulus_reg_word_write(uint32_t ui32StimReg, uint32_t ui32Value)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available, non-zero means ready
    //
    while (!AM_REGVAL(ui32StimAddr));

    //
    // Write the register.
    //
    AM_REGVAL(ui32StimAddr) = ui32Value;
} // am_hal_itm_stimulus_reg_word_write()

//*****************************************************************************
//
// Writes a short to the given ITM stimulus register.
//
//*****************************************************************************
void
am_hal_itm_stimulus_reg_short_write(uint32_t ui32StimReg, uint16_t ui16Value)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available non-zero means ready
    //
    while ( !AM_REGVAL(ui32StimAddr) );

    //
    // Write the register.
    //
    *((volatile uint16_t *) ui32StimAddr) = ui16Value;
} // am_hal_itm_stimulus_reg_short_write()

//*****************************************************************************
//
// Writes a byte to the given ITM stimulus register.
//
//*****************************************************************************
void
am_hal_itm_stimulus_reg_byte_write(uint32_t ui32StimReg, uint8_t ui8Value)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available (non-zero means ready)
    //
    while (!AM_REGVAL(ui32StimAddr));

    //
    // Write the register.
    //
    *((volatile uint8_t *) ui32StimAddr) = ui8Value;
} // am_hal_itm_stimulus_reg_byte_write()

//*****************************************************************************
//
// Sends a Sync Packet.
//
//*****************************************************************************
void
am_hal_itm_sync_send(void)
{
    //
    // Write the register.
    //
    am_hal_itm_stimulus_reg_word_write(AM_HAL_ITM_SYNC_REG,
                                       AM_HAL_ITM_SYNC_VAL);
} // am_hal_itm_sync_send()

//*****************************************************************************
//
// Poll the print stimulus registers until not busy.
//
//*****************************************************************************
bool
am_hal_itm_print_not_busy(void)
{
    //
    // Poll stimulus register allocated for printing.
    //
    return am_hal_itm_stimulus_not_busy(0);

} // am_hal_itm_print_not_busy()

//*****************************************************************************
//
// Prints a char string out of the ITM.
//
//*****************************************************************************
void
am_hal_itm_print(char *pcString)
{
    uint32_t ui32Length = 0;

    //
    // Determine the length of the string.
    //
    while (*(pcString + ui32Length))
    {
        ui32Length++;
    }

    //
    // If there is no longer a word left, empty out the remaining characters.
    //
    while (ui32Length)
    {
        //
        // Print string out the ITM.
        //
        am_hal_itm_stimulus_reg_byte_write(0, (uint8_t)*pcString++);

        //
        // Subtract from length.
        //
        ui32Length--;
    }
} // am_hal_itm_print()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
