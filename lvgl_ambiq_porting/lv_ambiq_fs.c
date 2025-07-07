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

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
FATFS lv_FatFs;
char lv_Path[4];
uint8_t lv_work_buf[FF_MAX_SS];


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_ambiq_fs_init(void)
{
    /*for FatFS initialize the EMMC card and FatFS itself*/
    FRESULT res;

    res = f_mount(&lv_FatFs, (TCHAR const*)lv_Path, 1);
    if(res != FR_OK)
    {
        res = f_mkfs((TCHAR const*)lv_Path, 0, lv_work_buf, sizeof(lv_work_buf));
    }
}
