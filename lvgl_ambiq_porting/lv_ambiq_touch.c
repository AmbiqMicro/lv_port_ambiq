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
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "lvgl.h"
#include "lv_ambiq_display.h"
#include "lv_ambiq_touch.h"
#include "am_devices_chsc5816_ap5.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define TP_IOM_MODULE              2
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
    am_iom_isr1(TP_IOM_MODULE)
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
    //
    // Save the pressed coordinates and the state
    //
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
    //
    // Convert the raw touch coordinates to match the LVGL display area by applying an offset.
    // This centers the touch input within the LVGL rendering buffer, accounting for the resolution difference.
    //
    data->point.x = data->point.x - ((DISPLAY_TOUCH_RESX - LV_AMBIQ_DISPLAY_BUFFER_RESX)/2);
    data->point.y = data->point.y - ((DISPLAY_TOUCH_RESY - LV_AMBIQ_DISPLAY_BUFFER_RESY)/2);

    LV_LOG_TRACE("x: %d,  y: %d; state: %d\n",data->point.x,data->point.y,data->state);
}

//*****************************************************************************
//
// Init touch device
//
//*****************************************************************************
void lv_ambiq_touch_init(void)
{
    //
    // Init touch device.
    //
    am_devices_chsc5816_init(TP_IOM_MODULE, &g_sI2cNBConfig, &g_pCHSC5816Handle, &g_pIOMCHSC5816Handle, AM_BSP_GPIO_TOUCH_INT, AM_BSP_GPIO_TOUCH_RST, lv_ambiq_touch_handler, NULL);

    //
    // Create LVGL input device.
    //
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lv_ambiq_touch_read);
    lv_indev_set_long_press_time(indev, 2000);

    //
    // Initialize touch coordinate offset so that (0,0) corresponds to the top-left corner of the LVGL display area.
    // This compensates for any difference between the physical touch resolution and the LVGL buffer resolution.
    //
    g_sTouchInfo.x0 = (DISPLAY_TOUCH_RESX - LV_AMBIQ_DISPLAY_BUFFER_RESX)/2;
    g_sTouchInfo.y0 = (DISPLAY_TOUCH_RESY - LV_AMBIQ_DISPLAY_BUFFER_RESY)/2;
}