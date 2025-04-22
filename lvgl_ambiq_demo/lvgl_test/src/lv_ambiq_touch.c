//*****************************************************************************
//
//! @file lv_ambiq_touch.c
//!
//! @brief APIs for touch feature on LVGL.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "lvgl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "lv_ambiq_touch.h"

#include "demos/lv_demos.h"

#include "am_devices_chsc5816_ap5.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define TP_IOM_MODULE              AM_BSP_TP_IOM_MODULE
#define TP_IOM_MODE                AM_HAL_IOM_I2C_MODE

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static am_devices_tc_chsc5816_info_t g_sTouchInfo = {0};

void            *g_pCHSC5816Handle;
void            *g_pIOMCHSC5816Handle;

AM_SHARED_RW uint32_t DMATCBBuffer[1024];
am_devices_iom_chsc5816_config_t g_sI2cNBConfig =
{
    .ui32ClockFreq          = AM_HAL_IOM_400KHZ,
    .ui32NBTxnBufLength     = sizeof(DMATCBBuffer) / 4,
    .pNBTxnBuf              = DMATCBBuffer
};

// Config CQ buffer locate in SSRAM as non-cacheable.
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .sInnerAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .eDeviceAttr = 0
};
am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 0,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true
};

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define fram_iom_isr                                                          \
    am_iom_isr1(AM_BSP_TP_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr
//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
void
fram_iom_isr(void)
{
    uint32_t ui32Status;
    if (!am_hal_iom_interrupt_status_get(g_pIOMCHSC5816Handle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMCHSC5816Handle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMCHSC5816Handle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// Touch functions
//
//*****************************************************************************
static void lv_ambiq_touch_handler(void *x)
{
    am_devices_chsc5816_get_point(g_pCHSC5816Handle, &g_sTouchInfo);
}

void lv_ambiq_touch_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    /*Save the pressed coordinates and the state*/
    if(g_sTouchInfo.touch_released == true)
    {
        data->point.x = g_sTouchInfo.x0;
        data->point.y = g_sTouchInfo.y0;
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->point.x = g_sTouchInfo.x0;
        data->point.y = g_sTouchInfo.y0;
        data->state = LV_INDEV_STATE_PR;
    }

    am_util_stdio_printf("x: %d,  y: %d; state: %d\n",data->point.x,data->point.y,data->state);
}

//*****************************************************************************
//
// Init touch device
//
//*****************************************************************************
void lv_ambiq_touch_init(void)
{
    am_devices_chsc5816_init(AM_BSP_TP_IOM_MODULE, &g_sI2cNBConfig, &g_pCHSC5816Handle, &g_pIOMCHSC5816Handle, AM_BSP_GPIO_TOUCH_INT, AM_BSP_GPIO_TOUCH_RST, lv_ambiq_touch_handler, NULL);
}

//*****************************************************************************
//
// Create LVGL input device.
//
//*****************************************************************************
void lv_ambiq_touch_create(void)
{
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lv_ambiq_touch_read);
    lv_indev_set_long_press_time(indev, 2000);
}