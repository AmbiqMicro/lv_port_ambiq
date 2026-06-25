//*****************************************************************************
//
//! @file lv_ambiq_fs.c
//!
//! @brief Ambiq fs porting.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

/*********************
 *      INCLUDES
 *********************/
#include "lv_ambiq_fs.h"
#include "lvgl.h"
#include "ff.h"
#include "am_util.h"
#include "am_bsp.h"
#include "mmc_apollo5.h"

/*********************
 *      DEFINES
 *********************/
#define SDIO_EMMC_MODULE    0

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ambiq_emmc_set_speed(bool fast);
static void lv_ambiq_emmc_board_init(void);
static FRESULT lv_ambiq_emmc_mount(void);

/**********************
 *  STATIC VARIABLES
 **********************/
FATFS lv_FatFs;
char lv_Path[4];
uint8_t lv_work_buf[FF_MAX_SS];

extern am_hal_card_t eMMC_FatFs[AM_REG_SDIO_NUM_MODULES];

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void lv_ambiq_emmc_set_speed(bool fast)
{
    if(fast) {
        /* Same as apollo510_evb and Ambiq emmc_bm_fatfs on apollo510dL. */
        g_sEMMC_config.ui32Clock = 48000000;
        g_sEMMC_config.eUHSMode = AM_HAL_HOST_UHS_SDR50;
    }
    else {
        g_sEMMC_config.ui32Clock = 24000000;
        g_sEMMC_config.eUHSMode = AM_HAL_HOST_UHS_NONE;
    }
}

static void lv_ambiq_emmc_board_init(void)
{
    am_bsp_sdio_reset(SDIO_EMMC_MODULE);
    am_bsp_sdio_pins_enable(SDIO_EMMC_MODULE, AM_HAL_HOST_BUS_WIDTH_8);

    mmc_disk_invalidate_host_state();

    g_sEMMC_hw.pDevHandle = &eMMC_FatFs[SDIO_EMMC_MODULE];
    g_sEMMC_config.eHost = AM_HAL_SDHC_CARD_HOST;
    g_sEMMC_config.eBusWidth = AM_HAL_HOST_BUS_WIDTH_8;
    g_sEMMC_config.eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8;
    g_sEMMC_config.eXferMode = AM_HAL_HOST_XFER_ADMA;
    g_sEMMC_config.bAsync = false;
    g_sEMMC_config.eCardType = AM_HAL_CARD_TYPE_EMMC;
    g_sEMMC_config.eCardPwrCtrlPolicy = AM_HAL_CARD_PWR_CTRL_NONE;
    g_sEMMC_config.pCardPwrCtrlFunc = NULL;

    lv_ambiq_emmc_set_speed(true);
}

static FRESULT lv_ambiq_emmc_mount(void)
{
    return f_mount(&lv_FatFs, (TCHAR const *)lv_Path, 1);
}

void lv_ambiq_fs_init(void)
{
    FRESULT res;
    bool fast_mode = true;

    lv_ambiq_emmc_board_init();
    res = lv_ambiq_emmc_mount();
    if(res != FR_OK) {
        am_util_stdio_printf("FatFS: mount failed (%d), retry at 24MHz\r\n", (int)res);
        mmc_disk_invalidate_host_state();
        lv_ambiq_emmc_set_speed(false);
        fast_mode = false;
        res = lv_ambiq_emmc_mount();
    }

    am_util_stdio_printf("FatFS: mount %s (%d) %s\r\n",
                         (res == FR_OK) ? "ok" : "failed",
                         (int)res,
                         fast_mode ? "48MHz SDR50" : "24MHz");
}
