//*****************************************************************************
//
//! @file am_hal_mpu.c
//!
//! @brief Hardware abstraction for the MPU.
//!
//! @addtogroup mpu MPU - Memory Protection Unit
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
#include "am_hal_mpu.h"

//*****************************************************************************
//
// Disable MPU
//
//*****************************************************************************
uint32_t
am_hal_mpu_disable(void)
{
    ARM_MPU_Disable();

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Enable MPU
//
//*****************************************************************************
uint32_t
am_hal_mpu_enable(bool bPrivelegedDefault,
                  bool bFaultNMIProtect)
{
    uint32_t ui32MPUControl = ((bFaultNMIProtect << MPU_CTRL_HFNMIENA_Pos) |
                               (bPrivelegedDefault << MPU_CTRL_PRIVDEFENA_Pos));

    if (MPU->CTRL & MPU_CTRL_ENABLE_Msk)
    {
        return AM_HAL_STATUS_IN_USE;
    }

    ARM_MPU_Enable(ui32MPUControl);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Configures MPU attributes.
//
//*****************************************************************************
uint32_t
am_hal_mpu_attr_configure(am_hal_mpu_attr_t *psConfig, uint32_t ui32Cnt)
{
    uint8_t ui8Idx;
    uint8_t ui8Attr;
    uint8_t ui8OutAttr;
    uint8_t ui8InAttr;


    if (MPU->CTRL & MPU_CTRL_ENABLE_Msk)
    {
        return AM_HAL_STATUS_IN_USE;
    }

    for (uint32_t i = 0; i < ui32Cnt; i++)
    {
        ui8Idx = psConfig->ui8AttrIndex;
        if (ui8Idx >= 8)
        {
            return AM_HAL_STATUS_OUT_OF_RANGE;
        }
        if (psConfig->bNormalMem)
        {
            ui8OutAttr = ARM_MPU_ATTR_MEMORY_(psConfig->sOuterAttr.bNonTransient,
                                              psConfig->sOuterAttr.bWriteBack,
                                              psConfig->sOuterAttr.bReadAllocate,
                                              psConfig->sOuterAttr.bWriteAllocate);
            ui8InAttr  = ARM_MPU_ATTR_MEMORY_(psConfig->sInnerAttr.bNonTransient,
                                              psConfig->sInnerAttr.bWriteBack,
                                              psConfig->sInnerAttr.bReadAllocate,
                                              psConfig->sInnerAttr.bWriteAllocate);
        }
        else
        {
            ui8OutAttr = ARM_MPU_ATTR_DEVICE;
            ui8InAttr  = (psConfig->eDeviceAttr << 2);
        }
        ui8Attr = ARM_MPU_ATTR(
                  ui8OutAttr/* Outer or 0's */,
                  ui8InAttr/* Inner or Device */);
        ARM_MPU_SetMemAttr(ui8Idx, ui8Attr);
        psConfig++;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Configures an MPU region.
//
//*****************************************************************************
uint32_t
am_hal_mpu_region_configure(am_hal_mpu_region_config_t *psConfig, uint32_t ui32Cnt)
{
    ARM_MPU_Region_t RegionCfgTab[APOLLO5_MPU_REGIONS];
    uint32_t ui32StartRNR = psConfig->ui32RegionNumber;

    if (MPU->CTRL & MPU_CTRL_ENABLE_Msk)
    {
        return AM_HAL_STATUS_IN_USE;
    }

    if (ui32StartRNR + ui32Cnt > APOLLO5_MPU_REGIONS)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    //
    // Set the new base address for the specified region.
    //
    for (uint32_t i = 0; i < ui32Cnt; i++)
    {
        RegionCfgTab[i].RBAR = ((psConfig->ui32BaseAddress & MPU_RBAR_BASE_Msk) |
                                (psConfig->eShareable << MPU_RBAR_SH_Pos) |
                                (psConfig->eAccessPermission << MPU_RBAR_AP_Pos) |
                                (psConfig->bExecuteNever << MPU_RBAR_XN_Pos));

        RegionCfgTab[i].RLAR = ((psConfig->ui32LimitAddress & MPU_RLAR_LIMIT_Msk) |
                                (psConfig->ui32AttrIndex << MPU_RLAR_AttrIndx_Pos) |
                                (psConfig->bEnable));
        psConfig++;
    }

    ARM_MPU_Load(ui32StartRNR, (ARM_MPU_Region_t const*) RegionCfgTab, ui32Cnt);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Enable an MPU region.
//
//*****************************************************************************
uint32_t
am_hal_mpu_region_enable(uint8_t ui8RegionNumber)
{
    //
    // Set the region number in the MPU_RNR register, and set the enable bit.
    //
    MPU->RNR = ui8RegionNumber;
    MPU->RLAR |= MPU_RLAR_EN_Msk;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Disable an MPU region.
//
//*****************************************************************************
uint32_t
am_hal_mpu_region_disable(uint8_t ui8RegionNumber)
{
    //
    // Set the region number in the MPU_RNR register, and clear the enable bit.
    //
    MPU->RNR = ui8RegionNumber;
    MPU->RLAR &= ~MPU_RLAR_EN_Msk;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Returns the amount of D regions
//
//*****************************************************************************
uint32_t
am_hal_mpu_dregion_amount_get(void)
{
    uint32_t ui32DRegionAmount = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;

    return ui32DRegionAmount;
}

//*****************************************************************************
//
// Clear all MPU regions
//
//*****************************************************************************
uint32_t
am_hal_mpu_region_clear(void)
{
    for (uint32_t i = 0U; i < APOLLO5_MPU_REGIONS; ++i)
    {
        MPU->RNR = i;
        MPU->RBAR = 0U;
        MPU->RLAR = 0U;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

