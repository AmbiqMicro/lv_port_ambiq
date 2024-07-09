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
// This is part of revision release_sdk_4_5_0-a1ef3b89f9 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "mmc_apollo4.h"

static volatile DSTATUS Stat = STA_NOINIT;    /* Disk status */
am_hal_card_t eMMCard;
am_hal_card_host_t *pSdhcCardHost = NULL;
am_hal_card_info_t eMMCardInfo;
DWORD sec_count = 0;



/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

//*****************************************************************************
//
// Interrupt handler for SDIO
//
//*****************************************************************************
void am_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_initialize (void)
{
    DWORD i = 0;

    if ( Stat == STA_NOINIT )
    {
        //
        // Get the SDHC card host instance
        //
        pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);

        if (pSdhcCardHost == NULL)
        {
            am_util_debug_printf("Error: No card host found!\n");
            Stat |= STA_NOINIT;
            return Stat;
        }

        am_util_debug_printf("card host is found\n");

        //
        // check if card is present
        //
        while (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_debug_printf("No card is present now\n");
            am_util_delay_ms(10);
            am_util_debug_printf("Checking if card is available again\n");
            i++;
            if ( i ==  100 )
            {
                Stat |= STA_NOINIT;
                return Stat;
            }
        }

        //
        // Initialize the card
        //
        i = 0;
        while (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_delay_ms(10);
            am_util_debug_printf("card and host is not ready, try again\n");
            i++;
            if ( i ==  100 )
            {
                Stat |= STA_NOINIT;
                return Stat;
            }
        }

        //
        // Set ADMA transfer mode
        //
        am_hal_card_host_set_xfer_mode(pSdhcCardHost, AM_HAL_HOST_XFER_ADMA);

        //
        // Get eMMC card info
        //
        am_hal_card_get_info(&eMMCard, &eMMCardInfo);

        if (eMMCardInfo.ui32BlkSize != 512)
        {
            am_util_stdio_printf("WARNING: eMMC card block size is not 512 bytes\n");
        }

        //
        // Get the sector count from eMMC
        //
        sec_count = am_hal_card_get_ext_csd_field(&eMMCard, 212, 4);
        if ( sec_count == 0 )
        {
            am_util_debug_printf("Fail to get the sector counter of eMMC\n");
            Stat |= STA_NOINIT;
            return Stat;
        }
        else
        {
            am_util_debug_printf("eMMC EXT CSD Sector Count is: 0x%x\n", sec_count);
        }

        //
        // Configure the card
        //
        if ( am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
                AM_BSP_SDIO_BUS_WIDTH, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
                AM_HAL_HOST_UHS_SDR50) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to configure the card\n");
            Stat |= STA_NOINIT;
            return Stat;
        }

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(&eMMCard) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to power down SDIO peripheral\n");
            Stat |= STA_NOINIT;
            return Stat;
        }
        else
        {
            am_util_debug_printf("Power down SDIO peripheral\n");
        }
#endif

        Stat &= ~STA_NOINIT;
    }

    return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_status (void)
{
    return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_read (
    BYTE *buff,            /* Pointer to the data buffer to store read data */
    LBA_t sector,                /* Start sector number (LBA) */
    UINT count            /* Sector count (1..128) */
)
{
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(&eMMCard) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power up SDIO peripheral\n");
    }
#endif

    am_util_debug_printf("Read Data\n");
    am_util_debug_printf("count = %d  sector= %d\n", count, sector);

    ui32Status = am_hal_card_block_read_sync(&eMMCard, sector, count, (uint8_t *)buff);
    if ( (ui32Status & 0xffff) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to call read API. Read Status = %d\n", ui32Status);
        return RES_ERROR;
    }

    am_util_debug_printf("Read Completed \n");

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(&eMMCard) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power down SDIO peripheral\n");
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

    if ( Stat & STA_NOINIT )
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(&eMMCard) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power up SDIO peripheral\n");
    }
#endif

    am_util_debug_printf("Write Data\n");
    am_util_debug_printf("count = %d  sector= %d\n", count, sector);

    wr_buf = (BYTE *)buff;

    ui32Status = am_hal_card_block_write_sync(&eMMCard, sector, count, (uint8_t *)wr_buf);
    if ( (ui32Status & 0xffff) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to call write API. Write Status = %d\n", ui32Status);
        return RES_ERROR;
    }

    am_util_debug_printf("Write Completed \n");

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(&eMMCard) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power down SDIO peripheral\n");
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

    if (Stat & STA_NOINIT)
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
            *(DWORD*)buff = sec_count;
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


