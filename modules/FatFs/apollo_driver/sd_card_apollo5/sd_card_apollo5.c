/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  R0.14                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2020, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

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
#include <string.h>
#include <stdlib.h>

#include "am_bsp.h"
#include "am_util.h"
#include "sd_card_apollo5.h"

/*--------------------------------------------------------------------------

   Module Global variable

---------------------------------------------------------------------------*/

am_hal_card_t SD_Card_FatFs[AM_REG_SDIO_NUM_MODULES];

am_device_emmc_config_t     g_SdCardConfig = 
{
    .eHost                  = AM_HAL_SDHC_CARD_HOST,
    .eXferMode              = AM_HAL_HOST_XFER_ADMA,
    .ui32Clock              = 48000000,
    .eBusWidth              = AM_HAL_HOST_BUS_WIDTH_4,
    .eBusVoltage            = AM_HAL_HOST_BUS_VOLTAGE_1_8,
    .eUHSMode               = AM_HAL_HOST_UHS_SDR50,
    .bAsync                 = false,
    .eCardType              = AM_HAL_CARD_TYPE_SDHC,
    .eCardPwrCtrlPolicy     = AM_HAL_CARD_PWR_CTRL_NONE,
    .pCardPwrCtrlFunc       = NULL,
    .pfunCallback           = NULL,
    .ui32SectorCount        = 0x100,
};

am_device_emmc_hw_t   g_SdCardHw = 
{
    .pSdhcHost = NULL,
    .pDevHandle = &SD_Card_FatFs[0],
    .dStat = STA_NOINIT,
    .bAsyncReadIsDone = true,
    .bAsyncWriteIsDone = true,
};
/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

void am_sdio0_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_SdCardHw.pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_SdCardHw.pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_SdCardHw.pSdhcHost->pHandle, ui32IntStatus);
}

void am_sdio1_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_SdCardHw.pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_SdCardHw.pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_SdCardHw.pSdhcHost->pHandle, ui32IntStatus);
}

void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        g_SdCardHw.bAsyncReadIsDone = true;
        am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        g_SdCardHw.bAsyncWriteIsDone = true;
        am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        am_util_debug_printf("SDMA Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        am_util_debug_printf("SDMA Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        am_util_debug_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }
}

//
// Register SD Card power cycle function for card power on/off/reset,
// Usually used for switching to 3.3V signaling in sd card initialization
//
uint32_t sd_card_power_config(am_hal_card_pwr_e eCardPwr)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_POWER_CTRL, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);

    if ( eCardPwr == AM_HAL_CARD_PWR_CYCLE )
    {
        //
        // SD Card power cycle or power on
        //
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_POWER_CTRL);
        am_util_delay_ms(20);
        am_hal_gpio_output_set(AM_BSP_GPIO_SD_POWER_CTRL);

        //
        // wait until the power supply is stable
        //
        am_util_delay_ms(20);

#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif
    }
    else if ( eCardPwr == AM_HAL_CARD_PWR_OFF )
    {
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_POWER_CTRL);

#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif
    }
    else if ( eCardPwr == AM_HAL_CARD_PWR_SWITCH )
    {
        //
        // set level shifter to 1.8V
        //
        am_hal_gpio_output_set(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
        am_util_delay_ms(20);
    }

    return true;
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_initialize (void)
{
    am_util_debug_printf("mmc_disk_initialize : g_SdCardHw.dStat = %x\n", g_SdCardHw.dStat);

    if ( g_SdCardHw.dStat == STA_NOINIT )
    {
        //
        // Get the SDHC card host instance
        //
        g_SdCardHw.pSdhcHost = am_hal_get_card_host(g_SdCardConfig.eHost, true);

        if (g_SdCardHw.pSdhcHost == NULL)
        {
            am_util_stdio_printf("No such card host !!!\n");
            am_util_stdio_printf("SDIO Test Fail !!!\n");
            g_SdCardHw.dStat |= STA_NOINIT;
            return g_SdCardHw.dStat;
        }

        //
        // enable sdio pins
        //
        am_bsp_sdio_pins_enable(g_SdCardHw.pSdhcHost->ui32Module, AM_HAL_HOST_BUS_WIDTH_4);

        //
        // check if card is present
        //
        uint32_t ui32Retrials = 10;
        while (am_hal_card_host_find_card(g_SdCardHw.pSdhcHost, g_SdCardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("No card is present now\n");
            am_util_delay_ms(1000);
            am_util_stdio_printf("Checking if card is available again\n");
            ui32Retrials--;
            if(ui32Retrials == 0)
            {
                g_SdCardHw.dStat |= STA_NOINIT;
                return g_SdCardHw.dStat;
            }
        }

        ui32Retrials = 10;
        while (am_hal_card_init(g_SdCardHw.pDevHandle, g_SdCardConfig.eCardType, sd_card_power_config, g_SdCardConfig.eCardPwrCtrlPolicy) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_delay_ms(1000);
            am_util_stdio_printf("card and host is not ready, try again\n");
            ui32Retrials--;
            if(ui32Retrials == 0)
            {
                g_SdCardHw.dStat |= STA_NOINIT;
                return g_SdCardHw.dStat;
            }
        }

        //
        // Get the sector count from SD card
        //
        g_SdCardConfig.ui32SectorCount = am_hal_sd_card_get_block_count(g_SdCardHw.pDevHandle);

        if ( g_SdCardConfig.ui32SectorCount == 0 )
        {
            am_util_debug_printf("Fail to get the sector counter of Card\n");
            g_SdCardHw.dStat |= STA_NOINIT;
            return g_SdCardHw.dStat;
        }
        else
        {
            am_util_debug_printf("Card EXT CSD Sector Count is: 0x%x\n", g_SdCardConfig.ui32SectorCount);
        }

        if(AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(g_SdCardHw.pDevHandle, g_SdCardConfig.eCardType,
                                                        g_SdCardConfig.eBusWidth,
                                                        g_SdCardConfig.ui32Clock,
                                                        g_SdCardConfig.eBusVoltage,
                                                        g_SdCardConfig.eUHSMode))
        {
            am_util_stdio_printf("card config set failed\n");
            g_SdCardHw.dStat |= STA_NOINIT;
            return g_SdCardHw.dStat;
        }

        am_hal_card_host_set_xfer_mode(g_SdCardHw.pSdhcHost, g_SdCardConfig.eXferMode);

        g_SdCardConfig.pfunCallback = am_hal_card_event_test_cb;
        am_hal_card_register_evt_callback(g_SdCardHw.pDevHandle, g_SdCardConfig.pfunCallback);

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(g_SdCardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to power down SDIO peripheral\n");
            g_SdCardHw.dStat |= STA_NOINIT;
            return g_SdCardHw.dStat;
        }
#endif
        g_SdCardHw.dStat &= ~STA_NOINIT;
    }

    return g_SdCardHw.dStat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_status (void)
{
    return g_SdCardHw.dStat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_read (
    BYTE *buff,            /* Pointer to the data buffer to store read data */
    LBA_t sector,          /* Start sector number (LBA) */
    UINT count            /* Sector count (1..128) */
)
{
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if (g_SdCardHw.dStat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(g_SdCardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
#endif

    //am_util_debug_printf("Read, count = %d  sector= %d\n", count, sector);

    //
    // Calling Read API
    //
    if(!g_SdCardConfig.bAsync)
    {
        ui32Status = am_hal_sd_card_block_read_sync(g_SdCardHw.pDevHandle, sector, count, (uint8_t *)buff);
        if ( ui32Status == 0 )
        {
            am_util_debug_printf("Fail to call read API. number of bytes read = %d\n", ui32Status);
            return RES_ERROR;
        }
    }
    else
    {
        g_SdCardHw.bAsyncReadIsDone = false;
        ui32Status = am_hal_sd_card_block_read_async(g_SdCardHw.pDevHandle, sector, count, (uint8_t *)buff);
        if ( ui32Status != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call read API. Read Status = %d\n", ui32Status);
            return RES_ERROR;
        }

        //
        // wait until the async read is done
        //
        uint32_t ui32CounterTimeout = 0;
        while (!g_SdCardHw.bAsyncReadIsDone)
        {
            am_util_delay_us(100); //!  bumped up from 1ms
            ui32CounterTimeout++;
            if ( ui32CounterTimeout == DELAY_MAX_COUNT ) //! bumped up from 1000
            {
                am_util_debug_printf("Read Timeout\n");
                return RES_ERROR;
            }
        }
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(g_SdCardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
    }
#endif

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_write (
    const BYTE *buff,    /* Pointer to the data to be written */
    LBA_t sector,        /* Start sector number (LBA) */
    UINT count        /* Sector count (1..128) */
)
{
    BYTE * wr_buf;
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if ( g_SdCardHw.dStat & STA_NOINIT )
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(g_SdCardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
#endif

    //am_util_debug_printf("Write, count = %d  sector= %d\n", count, sector);

    wr_buf = (BYTE *)buff;

    //
    // Calling Write API
    //
    if(!g_SdCardConfig.bAsync)
    {
        ui32Status = am_hal_sd_card_block_write_sync(g_SdCardHw.pDevHandle, sector, count, (uint8_t *)wr_buf);
        if ( ui32Status == 0 )
        {
            am_util_debug_printf("Fail to call write API.  Number of bytes written is = %d\n", ui32Status);
            return RES_ERROR;
        }
    }
    else
    {
        g_SdCardHw.bAsyncWriteIsDone = false;
        ui32Status = am_hal_sd_card_block_write_async(g_SdCardHw.pDevHandle, sector, count, (uint8_t *)wr_buf);
        if ( ui32Status != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call write API. Write Status = %d\n", ui32Status);
            return RES_ERROR;
        }

        //
        // wait until the async write is done
        //
        uint32_t ui32CounterTimeout = 0;
        while (!g_SdCardHw.bAsyncWriteIsDone)
        {
            am_util_delay_us(100);  // was 1ms
            ui32CounterTimeout++;
            if ( ui32CounterTimeout == DELAY_MAX_COUNT )
            {
                am_util_debug_printf("Write Timeout\n");
                return RES_ERROR;
            }
        }
    }


#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(g_SdCardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
    }
#endif

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_ioctl (
    BYTE cmd,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
       DRESULT res = RES_ERROR;

#if FF_USE_TRIM
    LBA_t *range;
    DWORD st, ed;
#endif
#if _USE_ISDIO
    SDIO_CTRL *sdi;
    BYTE rc, *bp;
    UINT dc;
#endif

    if (g_SdCardHw.dStat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        /* Make sure that no pending write process */
        case CTRL_SYNC :
            res = RES_OK;
            break;

        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT :
            *(DWORD*)buff = g_SdCardConfig.ui32SectorCount;
            res = RES_OK;
            break;

        /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE :
            *(DWORD*)buff = 1;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
    }

    return res;
}


