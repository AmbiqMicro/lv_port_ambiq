//*****************************************************************************
//
//! @file am_bsp.c
//!
//! @brief Top level functions for performing board initialization.
//!
//! @addtogroup Apollo5 Engineering Board BSP Board Support Package (BSP)
//! @ingroup BSP
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

#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Configuration.
//
//*****************************************************************************
#define AM_BSP_ENABLE_ITM        0
#define AM_BSP_ITM_FREQUENCY     AM_HAL_TPIU_BAUD_1M
#define DCU_SWO                  \
    (AM_HAL_DCU_CPUTRC_DWT_SWO | AM_HAL_DCU_CPUDBG_NON_INVASIVE | AM_HAL_DCU_CPUDBG_S_NON_INVASIVE)

//*****************************************************************************
//
// LEDs
//
//*****************************************************************************
#if AM_BSP_NUM_LEDS
am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS] =
{
    {AM_BSP_GPIO_LED0, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED1, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED2, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED3, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED4, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED5, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED6, AM_DEVICES_LED_ON_LOW},
    {AM_BSP_GPIO_LED7, AM_DEVICES_LED_ON_LOW},
};
#endif

//*****************************************************************************
//
// NAND Flash configurations.
//
//*****************************************************************************

//*****************************************************************************
//
// Buttons.
//
//*****************************************************************************
#if AM_BSP_NUM_BUTTONS
am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS] =
{
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON0, AM_DEVICES_BUTTON_NORMAL_HIGH),
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON1, AM_DEVICES_BUTTON_NORMAL_HIGH),
};
#endif

#if defined (DISP_CTRL_IP)
//*****************************************************************************
//
// Display configurations.
//
//*****************************************************************************

am_devices_display_hw_config_t g_sDispCfg =
{
    //
    // Display driver IC type
    //
    .eIC = DISP_IC_RM69330,
    //
    // Display te pin
    //
    .ui16TEpin = DISPLAY_TE_PIN,
    //
    // Display resolutions
    //
    .ui16ResX = 456,
    .ui16ResY = 456,

#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)

    .eInterface = DISP_IF_QSPI,
    .ui32Module = DISPLAY_MSPI_INST,
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .bClockonD4 = false,
#endif

#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    .eInterface = DISP_IF_DSI,
    .eDsiFreq  = AM_HAL_DSI_FREQ_TRIM_X20,
    .eDbiWidth = AM_HAL_DSI_DBI_WIDTH_16,
    .ui8NumLanes = 1,
#endif

    .ui8DispMspiSelect = 2,

    .eTEType = DISP_TE_DC,
    .ui16Offset = 14,
    .bFlip = false
};
#endif // DISP_CTRL_IP

//*****************************************************************************
//
// Validation GPIO configuration
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_VALIDATION_GPIO =
{
    .GP.cfg_b.uFuncSel       = 3,
    .GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .GP.cfg_b.eGPOutCfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL
};

//*****************************************************************************
//
//  DSI VDD18 power switch.
//
//*****************************************************************************
void
am_bsp_external_vdd18_switch(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDD18_SWITCH, g_AM_BSP_GPIO_VDD18_SWITCH);
    am_hal_gpio_state_write(AM_BSP_GPIO_VDD18_SWITCH, eGpioWrType);
}

//*****************************************************************************
//
//  VDDUSB33 power switch.
//
//*****************************************************************************
void
am_bsp_external_vddusb33_switch(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDDUSB33_SWITCH, g_AM_BSP_GPIO_VDDUSB33_SWITCH);
    am_hal_gpio_state_write(AM_BSP_GPIO_VDDUSB33_SWITCH, eGpioWrType);
}

//*****************************************************************************
//
//  VDDUSB0P9 power switch.
//
//*****************************************************************************
void
am_bsp_external_vddusb0p9_switch(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDDUSB0P9_SWITCH, g_AM_BSP_GPIO_VDDUSB0P9_SWITCH);
    am_hal_gpio_state_write(AM_BSP_GPIO_VDDUSB0P9_SWITCH, eGpioWrType);
}
//*****************************************************************************
//
// Print interface tracking variable.
//
//*****************************************************************************
typedef enum
{
    AM_BSP_PRINT_IF_NONE,
    AM_BSP_PRINT_IF_SWO,
    AM_BSP_PRINT_IF_UART,
    AM_BSP_PRINT_IF_BUFFERED_UART,
    AM_BSP_PRINT_IF_MEMORY,
}
am_bsp_print_interface_e;

static am_bsp_print_interface_e g_ePrintInterface = AM_BSP_PRINT_IF_NONE;

//*****************************************************************************
//
// Default UART configuration settings.
//
//*****************************************************************************
static void *g_sCOMUART;

static const am_hal_uart_config_t g_sBspUartConfig =
{
    //
    // Standard UART settings: 115200-8-N-1
    //
    .ui32BaudRate = 115200,
    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_ONE_STOP_BIT,
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    //
    // Set TX and RX FIFOs to interrupt at half-full.
    //
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
};



//*****************************************************************************
//
// External power on.
//
// This function turns on external power switch
//
//*****************************************************************************
void
am_bsp_external_pwr_on(void)
{
} // am_bsp_external_pwr_on()

//*****************************************************************************
//
// Prepare the MCU for low power operation.
//
// This function enables several power-saving features of the MCU, and
// disables some of the less-frequently used peripherals. It also sets the
// system clock to 24 MHz.
//
//*****************************************************************************
void
am_bsp_low_power_init(void)
{
    am_util_delay_ms(2000);
#if 1 // Temporarily comment out for the initial silicon bringup

    //  am_bsp_itm_printf_disable();

    //
    // Initialize for low power in the power control block
    //
    am_hal_pwrctrl_low_power_init();
#endif

#if AM_BSP_ENABLE_SIMOBUCK
    //
    // Enable SIMOBUCK for this board
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_SIMOBUCK_INIT, NULL);
#endif

#if 0
    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();
#endif // 0
} // am_bsp_low_power_init()

//*****************************************************************************
//
// Enable the TPIU and ITM for debug printf messages.
//
// This function enables TPIU registers for debug printf messages and enables
// ITM GPIO pin to SWO mode. This function should be called after reset and
// after waking up from deep sleep.
//
//*****************************************************************************
int32_t
am_bsp_debug_printf_enable(void)
{
    switch (g_ePrintInterface)
    {
        case AM_BSP_PRINT_IF_NONE:
            // Fall on through to default to SWO
        case AM_BSP_PRINT_IF_SWO:
            return am_bsp_itm_printf_enable();

        case AM_BSP_PRINT_IF_UART:
            return am_bsp_uart_printf_enable();

        case AM_BSP_PRINT_IF_BUFFERED_UART:
            return am_bsp_buffered_uart_printf_enable(g_sCOMUART);

        default:
            break;
    }

    return -1;
} // am_bsp_debug_printf_enable()

//*****************************************************************************
//
//  Disable the TPIU and ITM for debug printf messages.
//
// This function disables TPIU registers for debug printf messages and disables
// ITM GPIO pin to SWO mode. This function should be called after reset and
// after waking up from deep sleep.
//
//*****************************************************************************
void
am_bsp_debug_printf_disable(void)
{
    switch (g_ePrintInterface)
    {
        case AM_BSP_PRINT_IF_SWO:
            am_bsp_itm_printf_disable();
            break;

        case AM_BSP_PRINT_IF_UART:
            am_bsp_uart_printf_disable();
            break;

        case AM_BSP_PRINT_IF_BUFFERED_UART:
            am_bsp_buffered_uart_printf_disable();
            break;


        default:
            break;
    }
} // am_bsp_debug_printf_disable()

//*****************************************************************************
//
//  Enable printing over ITM.
//
//*****************************************************************************
int32_t
am_bsp_itm_printf_enable(void)
{
    uint32_t ui32dcuVal;
    int32_t i32RetValue = 0;
    bool bOffCryptoOnExit = false;
    bool bOffOtpOnExit = false;

    //
    // Ensure Crypto and OTP states aren't changed by other tasks.
    //
    AM_CRITICAL_BEGIN;

    //
    // Need to make sure that SWO is enabled
    //
    {
        //
        // Ensure OTP is on before powering Crypto up.
        //
        if (PWRCTRL->DEVPWRSTATUS_b.PWRSTOTP == 0)
        {
            bOffOtpOnExit = true;
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
        }

        if (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 0)
        {
            bOffCryptoOnExit = true;
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
        }

        if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 1) && (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 1))
        {
            am_hal_dcu_get(&ui32dcuVal);

            //
            // Enable SWO
            //
// Need adjustement for rev B
            if ( ((ui32dcuVal & DCU_SWO) != DCU_SWO) &&
                 (am_hal_dcu_update(true, DCU_SWO) != AM_HAL_STATUS_SUCCESS) )
            {
                //
                // Cannot enable SWO
                //
                i32RetValue = -1;
            }
        }
        else
        {
            //
            // If DCU is not accessible, we cannot determine if ITM can be safely enabled.
            //
            i32RetValue = -1;
        }
    }

    //
    // Restore Crypto and OTP power states here and exit critial section.
    //
    if (bOffCryptoOnExit == true)
    {
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    }

    if (bOffOtpOnExit == true)
    {
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
    }

    AM_CRITICAL_END;

    //
    // If i32RetValue is not 0, return the error code directly. 
    //
    if (i32RetValue != 0)
    {
        return i32RetValue;
    }

    //
    // Enable the ITM interface and the SWO pin.
    //
    if ( am_hal_tpiu_enable(AM_HAL_TPIU_BAUD_1M) != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the TPIU enable.
        //
        while(1);
    }

    if ( am_hal_itm_enable() != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the ITM enable.
        //
        while(1);
    }

    //
    // Set the global print interface.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_SWO;

    if ( am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO) )
    {
        while (1);
    }

#if AM_BSP_ENABLE_ITM
    am_hal_tpiu_config_t TPIUcfg;

    //
    // Enable the ITM interface and the SWO pin.
    //
    am_hal_itm_enable();

    //
    // Enable the ITM and TPIU
    // Set the BAUD clock for 1M
    //
    TPIUcfg.ui32SetItmBaud = AM_HAL_TPIU_BAUD_115200;
    am_hal_tpiu_enable(&TPIUcfg);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO);
#endif

    //
    // Attach the ITM to the STDIO driver.
    //
    am_util_stdio_printf_init(am_hal_itm_print);

    return i32RetValue;
} // am_bsp_itm_printf_enable()

//*****************************************************************************
//
// Disable printing over ITM.
//
//*****************************************************************************
void
am_bsp_itm_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_SWO)
    {
        return;
    }

    //
    // Disable the ITM/TPIU
    //
    if ( am_hal_itm_disable() != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the ITM disable.
        //
        while(1);
    }

    if ( am_hal_tpiu_disable() != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the TPIU disable.
        //
        while(1);
    }

    //
    // Detach the ITM interface from the STDIO driver.
    //
    am_util_stdio_printf_init(0);

    //
    // Disconnect the SWO pin
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, am_hal_gpio_pincfg_default);

    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_itm_printf_disable()

#if defined (DISP_CTRL_IP)
//*****************************************************************************
//
// Set up the display pins.
//
//*****************************************************************************
void
am_bsp_disp_pins_enable(void)
{
    // Set up variables for IFSEL1 and IFSEL2
    am_hal_gpio_write_type_e eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_write_type_e eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;

    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_QSPI:
            if (g_sDispCfg.ui8DispMspiSelect == 2)
            {
                //
                //  This is the MSPI2 based QSPI Display interface.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_CS,  g_AM_BSP_GPIO_M2_DISP_QSPI_CS);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D0,  g_AM_BSP_GPIO_M2_DISP_QSPI_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D1,  g_AM_BSP_GPIO_M2_DISP_QSPI_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D2,  g_AM_BSP_GPIO_M2_DISP_QSPI_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D3,  g_AM_BSP_GPIO_M2_DISP_QSPI_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_SCK, g_AM_BSP_GPIO_M2_DISP_QSPI_SCK);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_TE,  g_AM_BSP_GPIO_M2_DISP_SPI_TE);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_RES, g_AM_BSP_GPIO_M2_DISP_SPI_RES);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_DEVICE_EN, g_AM_BSP_GPIO_M2_DISP_DEVICE_EN);
            }
            else
            {
                //
                //  This is the MSPI1 based QSPI Display interface. This will be used by default
                //  if the incorrect ui8MspiSelect byte is provided.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_CS,  g_AM_BSP_GPIO_M1_DISP_QSPI_CS);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D0,  g_AM_BSP_GPIO_M1_DISP_QSPI_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D1,  g_AM_BSP_GPIO_M1_DISP_QSPI_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D2,  g_AM_BSP_GPIO_M1_DISP_QSPI_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D3,  g_AM_BSP_GPIO_M1_DISP_QSPI_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_SCK, g_AM_BSP_GPIO_M1_DISP_QSPI_SCK);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_TE,  g_AM_BSP_GPIO_M1_DISP_SPI_TE);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_RES, g_AM_BSP_GPIO_M1_DISP_SPI_RES);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_DEVICE_EN, g_AM_BSP_GPIO_M1_DISP_DEVICE_EN);
            }

            //
            // Set interface mode to QSPI
            //
            eGpioType2 = AM_HAL_GPIO_OUTPUT_SET;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
            break;
        case DISP_IF_DSPI:
        case DISP_IF_SPI4:
            if (g_sDispCfg.ui8DispMspiSelect == 2)
            {
                //
                //  This is the MSPI2 based DSPI Display interface.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_CS,  g_AM_BSP_GPIO_M2_DISP_SPI_CS);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_SD,  g_AM_BSP_GPIO_M2_DISP_SPI_SD);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_DCX, g_AM_BSP_GPIO_M2_DISP_SPI_DCX);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_SCK, g_AM_BSP_GPIO_M2_DISP_SPI_SCK);
                //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SDI, g_AM_BSP_GPIO_DISP_SPI_SDI);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_TE,  g_AM_BSP_GPIO_M2_DISP_SPI_TE);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_RES, g_AM_BSP_GPIO_M2_DISP_SPI_RES);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_DEVICE_EN, g_AM_BSP_GPIO_M2_DISP_DEVICE_EN);
            }
            else
            {
                //
                //  This is the MSPI1 based DSPI Display interface. This will be used by default
                //  if the incorrect ui8MspiSelect byte is provided.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_CS,  g_AM_BSP_GPIO_M1_DISP_SPI_CS);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_SD,  g_AM_BSP_GPIO_M1_DISP_SPI_SD);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_DCX, g_AM_BSP_GPIO_M1_DISP_SPI_DCX);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_SCK, g_AM_BSP_GPIO_M1_DISP_SPI_SCK);
                //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SDI, g_AM_BSP_GPIO_DISP_SPI_SDI);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_TE,  g_AM_BSP_GPIO_M1_DISP_SPI_TE);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_RES, g_AM_BSP_GPIO_M1_DISP_SPI_RES);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_DEVICE_EN, g_AM_BSP_GPIO_M1_DISP_DEVICE_EN);
            }

            //
            // Set interface mode to DSPI
            //
            eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_SET;
            break;
        case DISP_IF_DSI:
            eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
            am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_TE,  g_AM_BSP_GPIO_M2_DISP_SPI_TE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_RES, g_AM_BSP_GPIO_M2_DISP_SPI_RES);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_DEVICE_EN, g_AM_BSP_GPIO_M2_DISP_DEVICE_EN);
            break;
        case DISP_IF_DPI:
            //
            // Apollo5 can support standard 16, 18 or 24 parallel data bits DPI(RGB) interface. This BSP dedicatedly support 6 parallel bits mode.
            // For more non-standard DPI interface configuration, please contact us.
            //
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_ON_OFF,  g_AM_BSP_GPIO_DISP_ON_OFF);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_COMFC,  g_AM_BSP_GPIO_DISP_COMFC);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_MODE,  g_AM_BSP_GPIO_DISP_MODE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_PCLK,  g_AM_BSP_GPIO_DISP_PCLK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VS,  g_AM_BSP_GPIO_DISP_VS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_HS,  g_AM_BSP_GPIO_DISP_HS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_COMIN,  g_AM_BSP_GPIO_DISP_COMIN);

            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_RD1,  g_AM_BSP_GPIO_DISP_RD1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_RD0,  g_AM_BSP_GPIO_DISP_RD0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GD1,  g_AM_BSP_GPIO_DISP_GD1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GD0,  g_AM_BSP_GPIO_DISP_GD0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BD1,  g_AM_BSP_GPIO_DISP_BD1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BD0,  g_AM_BSP_GPIO_DISP_BD0);
            break;
        case DISP_IF_JDI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GEN,  g_AM_BSP_GPIO_DISP_GEN);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_INTB, g_AM_BSP_GPIO_DISP_INTB);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R1,   g_AM_BSP_GPIO_DISP_R1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R2,   g_AM_BSP_GPIO_DISP_R2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G1,   g_AM_BSP_GPIO_DISP_G1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G2,   g_AM_BSP_GPIO_DISP_G2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B1,   g_AM_BSP_GPIO_DISP_B1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B2,   g_AM_BSP_GPIO_DISP_B2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GSP,  g_AM_BSP_GPIO_DISP_GSP);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BSP,  g_AM_BSP_GPIO_DISP_BSP);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GCK,  g_AM_BSP_GPIO_DISP_GCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BCK,  g_AM_BSP_GPIO_DISP_BCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VA,   g_AM_BSP_GPIO_DISP_VA);
            //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VB, g_AM_BSP_GPIO_DISP_VB);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VCOM, g_AM_BSP_GPIO_DISP_VCOM);
            break;
        case DISP_IF_DBI:
            //
            // DISP_TE, DISP_RSTn, and DISP_EN should all be matched to VDDH2 IO domain.
            //
            // This interface will need to be double-checked when the appropriate hardware
            //  is available. For now, the below control signals will be included for this case.
            //

            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_RESX,   g_AM_BSP_GPIO_DBIB_RESX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_CSX,    g_AM_BSP_GPIO_DBIB_CSX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_DCX,    g_AM_BSP_GPIO_DBIB_DCX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_WRX,    g_AM_BSP_GPIO_DBIB_WRX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_RDX,    g_AM_BSP_GPIO_DBIB_RDX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D0,     g_AM_BSP_GPIO_DBIB_D0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D1,     g_AM_BSP_GPIO_DBIB_D1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D2,     g_AM_BSP_GPIO_DBIB_D2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D3,     g_AM_BSP_GPIO_DBIB_D3);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D4,     g_AM_BSP_GPIO_DBIB_D4);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D5,     g_AM_BSP_GPIO_DBIB_D5);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D6,     g_AM_BSP_GPIO_DBIB_D6);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D7,     g_AM_BSP_GPIO_DBIB_D7);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D8,     g_AM_BSP_GPIO_DBIB_D9);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D9,     g_AM_BSP_GPIO_DBIB_D9);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D10,    g_AM_BSP_GPIO_DBIB_D10);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D11,    g_AM_BSP_GPIO_DBIB_D11);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D12,    g_AM_BSP_GPIO_DBIB_D12);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D13,    g_AM_BSP_GPIO_DBIB_D13);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D14,    g_AM_BSP_GPIO_DBIB_D14);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D15,    g_AM_BSP_GPIO_DBIB_D15);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_BL, g_AM_BSP_GPIO_BL);
            break;
        default:
            break;
    }

    // Configure interface mode selector pins for display
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL1, g_AM_BSP_GPIO_DISP_IFSEL1);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL2, g_AM_BSP_GPIO_DISP_IFSEL2);

    // Configure Touch interrupt pin
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_INT, g_AM_BSP_GPIO_TOUCH_INT);

    // Set interface mode depending on g_sDispCfg.eInterface mode
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL2, eGpioType2);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL1, eGpioType1);

    // Enable display
    if (g_sDispCfg.eInterface == DISP_IF_SPI4 || g_sDispCfg.eInterface == DISP_IF_DSPI || g_sDispCfg.eInterface == DISP_IF_QSPI  || g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        //
        // The only case where enable needs to be on the VDDH domain
        // is when the display is in QSPI/DSPI/SPI4 mode.
        //
        if (g_sDispCfg.ui8DispMspiSelect == 2)
        {
            am_hal_gpio_state_write(AM_BSP_GPIO_M2_DISP_DEVICE_EN, AM_HAL_GPIO_OUTPUT_SET);
        }
        else
        {
            //
            // else the display should use the VDDH2 domain, which most interfaces use.
            //
            am_hal_gpio_state_write(AM_BSP_GPIO_M1_DISP_DEVICE_EN, AM_HAL_GPIO_OUTPUT_SET);
        }
    }
    else if (g_sDispCfg.eInterface == DISP_IF_DPI)
    {
        //
        // Select reflective mode
        //
        am_hal_gpio_state_write(AM_BSP_GPIO_DISP_COMFC, AM_HAL_GPIO_OUTPUT_CLEAR);
    }
    else if (g_sDispCfg.eInterface == DISP_IF_DBI)
    {
        //
        // The user could adjust the blacklight brightness through a PWM signal. Setting it aways high to simplify code.
        //
        am_hal_gpio_state_write(AM_BSP_GPIO_BL, AM_HAL_GPIO_OUTPUT_SET);
    }
} // am_bsp_disp_pins_enable()

//*****************************************************************************
//
// Get pins number of VA or VCOM or VB
//
//*****************************************************************************
uint32_t
am_bsp_disp_jdi_timer_pins(uint8_t ui8TimerPin)
{
    uint32_t ui32TimerPins = 0xFFFFFFFF;
    if (g_sDispCfg.eInterface == DISP_IF_DPI)
    {
        //
        // Get COMIN number
        //
        ui32TimerPins = AM_BSP_GPIO_DISP_COMIN;
    }
    else
    {
        switch (ui8TimerPin)
        {
            case AM_BSP_JDI_TIMER_VA:
                ui32TimerPins = AM_BSP_GPIO_DISP_VA;
            break;
            case AM_BSP_JDI_TIMER_VCOM:
                ui32TimerPins = AM_BSP_GPIO_DISP_VCOM;
            break;
            case AM_BSP_JDI_TIMER_VB:
                ui32TimerPins = AM_BSP_GPIO_DISP_VB;
            break;
            default:
            break;
        }
    }
    return ui32TimerPins;
}

//*****************************************************************************
//
// Disable the display pins.
//
//*****************************************************************************
void
am_bsp_disp_pins_disable(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_INT, am_hal_gpio_pincfg_disabled);

    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_QSPI:
            if (g_sDispCfg.ui8DispMspiSelect == 2)
            {
                //
                //  This is the MSPI2 based QSPI Display interface.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_CS,  am_hal_gpio_pincfg_disabled);
                //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_QSPI_SCK, am_hal_gpio_pincfg_disabled);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_TE,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_RES, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_DEVICE_EN, am_hal_gpio_pincfg_disabled);
            }
            else
            {
                //
                //  This is the MSPI1 based QSPI Display interface.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_CS,  am_hal_gpio_pincfg_disabled);
                //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_QSPI_SCK, am_hal_gpio_pincfg_disabled);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_TE,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_RES, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_DEVICE_EN, am_hal_gpio_pincfg_disabled);
            }
            break;
        case DISP_IF_DSPI:
        case DISP_IF_SPI4:
            if (g_sDispCfg.ui8DispMspiSelect == 2)
            {
                //
                //  This is the MSPI2 based DSPI Display interface.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_CS,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_SD,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_DCX, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_SCK, am_hal_gpio_pincfg_disabled);
                //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SDI, am_hal_gpio_pincfg_disabled);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_TE,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_SPI_RES, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M2_DISP_DEVICE_EN, am_hal_gpio_pincfg_disabled);
            }
            else
            {
                //
                //  This is the MSPI1 based DSPI Display interface.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_CS,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_SD,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_DCX, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_SCK, am_hal_gpio_pincfg_disabled);
                //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SDI, am_hal_gpio_pincfg_disabled);

                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_TE,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_SPI_RES, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_M1_DISP_DEVICE_EN, am_hal_gpio_pincfg_disabled);
            }
            break;
        case DISP_IF_DSI:
            break;
        case DISP_IF_DPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_ON_OFF,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_COMFC,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_MODE,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_PCLK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_HS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_COMIN,  am_hal_gpio_pincfg_disabled);

            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_RD1,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_RD0,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GD1,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GD0,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BD1,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BD0,  am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_JDI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GEN, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_INTB, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R1, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R2, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G1, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G2, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B1, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B2, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GSP, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BSP, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GCK, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BCK, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VA, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VB, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VCOM, am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_DBI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_RESX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_CSX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_DCX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_WRX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_RDX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D0, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D1, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D2, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D3, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D4, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D5, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D6, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D7, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D8, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D9, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D0, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D11, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D12, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D13, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D14, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DBIB_D15, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_BL, am_hal_gpio_pincfg_disabled);
            break;
        default:
            break;
    }

    // Un-configure interface mode selector pins for display
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL1, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL2, am_hal_gpio_pincfg_disabled);
} // am_bsp_disp_pins_disable()
#endif // DISP_CTRL_IP

//*****************************************************************************
//
// Set display reset pins.
//
//*****************************************************************************

static inline void
am_bsp_disp_reset_pin_output_set(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;

    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    if (g_sDispCfg.eInterface == DISP_IF_SPI4 || g_sDispCfg.eInterface == DISP_IF_DSPI || g_sDispCfg.eInterface == DISP_IF_QSPI || g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        if (g_sDispCfg.ui8DispMspiSelect == 2)
        {
            am_hal_gpio_state_write(AM_BSP_GPIO_M2_DISP_SPI_RES, eGpioWrType);
        }
        else
        {
            am_hal_gpio_state_write(AM_BSP_GPIO_M1_DISP_SPI_RES, eGpioWrType);
        }
    }
    else if (g_sDispCfg.eInterface == DISP_IF_DBI)
    {
        am_hal_gpio_state_write(AM_BSP_GPIO_DBIB_RESX, eGpioWrType);
    }
} // am_bsp_disp_reset_pin_output_set()

//*****************************************************************************
//
//  Set display reset pins.
//
//*****************************************************************************
void
am_bsp_disp_reset_pins_set(void)
{
    am_bsp_disp_reset_pin_output_set(true);
} // am_bsp_disp_reset_pins_set()

//*****************************************************************************
//
//  Set display reset pins.
//
//*****************************************************************************
void
am_bsp_disp_reset_pins_clear(void)
{
    am_bsp_disp_reset_pin_output_set(false);
} // am_bsp_disp_reset_pins_clear()

//*****************************************************************************
//
//  Enable the external display adaptor I2C (touch,als) pins
//
//*****************************************************************************
uint32_t
am_bsp_touch_als_enable( void )
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_ALS_M0SCL, g_AM_BSP_GPIO_TOUCH_ALS_M0SCL) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_ALS_M0SDA, g_AM_BSP_GPIO_TOUCH_ALS_M0SDA) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_INT, g_AM_BSP_GPIO_TOUCH_INT) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ALS_INT, g_AM_BSP_GPIO_ALS_INT) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_RST, g_AM_BSP_GPIO_TOUCH_RST) ;

    return 0 ;
}

//*****************************************************************************
//
//  Disable the external display adaptor I2C (touch,als) pins
//
//*****************************************************************************
uint32_t
am_bsp_touch_als_disable( void )
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_ALS_M0SCL, am_hal_gpio_pincfg_disabled) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_ALS_M0SDA, am_hal_gpio_pincfg_disabled) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_INT, am_hal_gpio_pincfg_disabled) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ALS_INT, am_hal_gpio_pincfg_disabled) ;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_RST, am_hal_gpio_pincfg_disabled) ;

    return 0 ;
}

#if 0
//*****************************************************************************
//
// Set up the DC's dpi pins.
//
// This function configures DC's DCX, CLOCK, DATA pins
//
//*****************************************************************************
void
am_bsp_disp_dpi_pins_enable(void)
{
    // workround for CS
    GPIO->PADKEY = 0x73;
    // GPIO->PINCFG50 = 0x00000013;  // TE, N/A for DPI
    GPIO->PINCFG70 = 0x00280007;     // CS, 26=DC_DPI_DE, 27=DISP_CONT_CSX, 28=DC_SPI_CS_N, 29=DC_QSPI_CS_N
    GPIO->PINCFG79 = 0x00000005;     // VS
    GPIO->PINCFG80 = 0x00000005;     // HS
    GPIO->PINCFG81 = 0x00000005;     // DE
    GPIO->PINCFG82 = 0x00000005;     // PCLK
    GPIO->PINCFG83 = 0x00000005;     // SD
    GPIO->PINCFG89 = 0x00000005;     // CM
    GPIO->PINCFG64 = 0x00000005;     // D0
    GPIO->PINCFG65 = 0x00000005;     // D1
    GPIO->PINCFG66 = 0x00000005;     // D2
    GPIO->PINCFG67 = 0x00000005;     // D3
    GPIO->PINCFG68 = 0x00000005;     // D4
    GPIO->PINCFG69 = 0x00000005;     // D5
    GPIO->PINCFG70 = 0x00000005;     // D6
    GPIO->PINCFG71 = 0x00000005;     // D7
    GPIO->PINCFG72 = 0x00000005;     // D8
    GPIO->PINCFG73 = 0x00000005;     // D9
    GPIO->PINCFG74 = 0x00000005;     // D10
    GPIO->PINCFG75 = 0x00000005;     // D11
    GPIO->PINCFG76 = 0x00000005;     // D12
    GPIO->PINCFG77 = 0x00000005;     // D13
    GPIO->PINCFG78 = 0x00000005;     // D14
    GPIO->PINCFG37 = 0x00000005;     // D15
    GPIO->PINCFG38 = 0x00000005;     // D16
    GPIO->PINCFG39 = 0x00000005;     // D17
    GPIO->PINCFG40 = 0x00000005;     // D18
    GPIO->PINCFG41 = 0x00000005;     // D19
    GPIO->PINCFG42 = 0x00000005;     // D20
    GPIO->PINCFG43 = 0x00000005;     // D21
    GPIO->PINCFG44 = 0x00000005;     // D22
    GPIO->PINCFG45 = 0x00000005;     // D23
    GPIO->PADKEY = 0;
} // am_bsp_disp_dpi_pins_enable()
#endif

//*****************************************************************************
//
//  Reset SDIO device via GPIO
//
//*****************************************************************************
void
am_bsp_sdio_reset(uint32_t ui32Module)
{
    switch (ui32Module)
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_RST, am_hal_gpio_pincfg_output);
            am_hal_gpio_output_clear(AM_BSP_GPIO_SDIO0_RST);
            am_util_delay_ms(10);
            am_hal_gpio_output_set(AM_BSP_GPIO_SDIO0_RST);
            break;
        case 1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_RST, am_hal_gpio_pincfg_output);
            am_hal_gpio_output_clear(AM_BSP_GPIO_SDIO1_RST);
            am_util_delay_ms(10);
            am_hal_gpio_output_set(AM_BSP_GPIO_SDIO1_RST);
            break;
        default:
            break;
    }
} // am_bsp_sdio_reset()

//*****************************************************************************
//
//  Set up the SDIO's pins.
//
// This function configures SDIO's CMD, CLK, DAT0-7 pins
//
//*****************************************************************************
void
am_bsp_sdio_pins_enable(uint8_t ui8SdioNum, uint8_t ui8BusWidth)
{
    switch (ui8SdioNum)
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CMD,  g_AM_BSP_GPIO_SDIO0_CMD);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CLK,  g_AM_BSP_GPIO_SDIO0_CLK);
            am_hal_gpio_cd0_pin_config(AM_BSP_GPIO_SDIO0_CD);
            am_hal_gpio_wp0_pin_config(AM_BSP_GPIO_SDIO0_WP);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT4, g_AM_BSP_GPIO_SDIO0_DAT4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT5, g_AM_BSP_GPIO_SDIO0_DAT5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT6, g_AM_BSP_GPIO_SDIO0_DAT6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT7, g_AM_BSP_GPIO_SDIO0_DAT7);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT1, g_AM_BSP_GPIO_SDIO0_DAT1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT2, g_AM_BSP_GPIO_SDIO0_DAT2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT3, g_AM_BSP_GPIO_SDIO0_DAT3);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT0, g_AM_BSP_GPIO_SDIO0_DAT0);
                default:
                    break;
            }
            break;
        case 1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CMD,  g_AM_BSP_GPIO_SDIO1_CMD);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CLK,  g_AM_BSP_GPIO_SDIO1_CLK);
            am_hal_gpio_cd1_pin_config(AM_BSP_GPIO_SDIO1_CD);
            am_hal_gpio_wp1_pin_config(AM_BSP_GPIO_SDIO1_WP);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT4, g_AM_BSP_GPIO_SDIO1_DAT4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT5, g_AM_BSP_GPIO_SDIO1_DAT5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT6, g_AM_BSP_GPIO_SDIO1_DAT6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT7, g_AM_BSP_GPIO_SDIO1_DAT7);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT1, g_AM_BSP_GPIO_SDIO1_DAT1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT2, g_AM_BSP_GPIO_SDIO1_DAT2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT3, g_AM_BSP_GPIO_SDIO1_DAT3);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT0, g_AM_BSP_GPIO_SDIO1_DAT0);
                default:
                    break;
            }
            break;
    }
} // am_bsp_sdio_pins_enable()

//*****************************************************************************
//
//  Disable the SDIO interface
//
//*****************************************************************************
void
am_bsp_sdio_pins_disable(uint8_t ui8SdioNum, uint8_t ui8BusWidth)
{
    switch (ui8SdioNum)
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CMD,  am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CLK,  am_hal_gpio_pincfg_default);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT4, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT5, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT6, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT7, am_hal_gpio_pincfg_default);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT1, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT2, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT3, am_hal_gpio_pincfg_default);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT0, am_hal_gpio_pincfg_default);
                default:
                    break;
            }
            break;
        case 1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CMD,  am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CLK,  am_hal_gpio_pincfg_default);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT4, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT5, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT6, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT7, am_hal_gpio_pincfg_default);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT1, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT2, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT3, am_hal_gpio_pincfg_default);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT0, am_hal_gpio_pincfg_default);
                default:
                    break;
            }
            break;
    }
} // am_bsp_sdio_pins_disable()

//*****************************************************************************
//
//  Power off eMMC with setting the supply of VCCQ and VCC
//
//*****************************************************************************
void
am_bsp_emmc_power_off(uint8_t ui8SdioNum)
{
    switch (ui8SdioNum)
    {
        case 0:
            am_hal_gpio_output_clear(AM_BSP_GPIO_VCCQ_EMMC0);
            am_util_delay_ms(10);
            am_hal_gpio_output_clear(AM_BSP_GPIO_VCC_EMMC0);
            break;
        case 1:
            am_hal_gpio_output_clear(AM_BSP_GPIO_VCCQ_EMMC1);
            am_util_delay_ms(10);
            am_hal_gpio_output_clear(AM_BSP_GPIO_VCC_EMMC1);
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
//  Power on eMMC with setting the supply of VCCQ and VCC
//
//*****************************************************************************
void
am_bsp_emmc_power_on(uint8_t ui8SdioNum)
{
    switch (ui8SdioNum)
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_VCCQ_EMMC0, g_AM_BSP_GPIO_VCCQ_EMMC0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_VCC_EMMC0, g_AM_BSP_GPIO_VCC_EMMC0);

            am_hal_gpio_output_set(AM_BSP_GPIO_VCCQ_EMMC0);
            am_util_delay_ms(50);
            am_hal_gpio_output_set(AM_BSP_GPIO_VCC_EMMC0);
            break;
        case 1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_VCCQ_EMMC1, g_AM_BSP_GPIO_VCCQ_EMMC1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_VCC_EMMC1, g_AM_BSP_GPIO_VCC_EMMC1);

            am_hal_gpio_output_set(AM_BSP_GPIO_VCCQ_EMMC1);
            am_util_delay_ms(50);
            am_hal_gpio_output_set(AM_BSP_GPIO_VCC_EMMC1);
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
//! @brief Set up the SD's CD pin.
//!
//! This function configure SD's CD pin in sd card detection.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_sd_cd_pin_enable(uint8_t ui8SdioNum, bool bEnable)
{
    switch (ui8SdioNum)
    {
        case 0:
            if ( bEnable )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD0_CD, am_hal_gpio_pincfg_input);
                am_hal_gpio_cd0_pin_config(AM_BSP_GPIO_SD0_CD);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD0_CD, am_hal_gpio_pincfg_default);
            }
            break;
        case 1:
            if ( bEnable )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD1_CD, am_hal_gpio_pincfg_input);
                am_hal_gpio_cd1_pin_config(AM_BSP_GPIO_SD1_CD);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD1_CD, am_hal_gpio_pincfg_default);
            }
            break;
        default:
            break;
    }
} // am_bsp_sd_cd_pin_enable()

//*****************************************************************************
//
//! @brief Set up the SD's WP pin.
//!
//! This function configure SD's WP pin to detect sd card write protection.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_sd_wp_pin_enable(uint8_t ui8SdioNum, bool bEnable)
{
    switch (ui8SdioNum)
    {
        case 0:
            if ( bEnable )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD0_WP, am_hal_gpio_pincfg_input);
                am_hal_gpio_wp0_pin_config(AM_BSP_GPIO_SD0_WP);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD0_WP, am_hal_gpio_pincfg_default);
            }
            break;
        case 1:
            if ( bEnable )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD1_WP, am_hal_gpio_pincfg_input);
                am_hal_gpio_wp1_pin_config(AM_BSP_GPIO_SD1_WP);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_SD1_WP, am_hal_gpio_pincfg_default);
            }
            break;
        default:
            break;
    }
} // am_bsp_sd_wp_pin_enable()

//*****************************************************************************
//
// Initialize and configure the UART
//
//*****************************************************************************
int32_t
am_bsp_uart_printf_enable(void)
{
    //
    // Initialize, power up, and configure the communication UART. Use the
    // custom configuration if it was provided. Otherwise, just use the default
    // configuration.
    //
    if (am_hal_uart_initialize(AM_BSP_UART_PRINT_INST, &g_sCOMUART) ||
        am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_uart_configure(g_sCOMUART, &g_sBspUartConfig) ||

        //
        // Enable the UART pins.
        //
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX) ||
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX) )
    {
        return -1;
    }

    //
    // Enable the UART pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);

    //
    // Save the information that we're using the UART for printing.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_UART;

    //
    // Register the BSP print function to the STDIO driver.
    //
    am_util_stdio_printf_init(am_bsp_uart_string_print);

    return 0;
} // am_bsp_uart_printf_enable()

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void
am_bsp_uart_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_UART)
    {
        return;
    }

    //
    // Make sure the UART has finished sending everything it's going to send.
    //
    am_hal_uart_tx_flush(g_sCOMUART);

    //
    // Detach the UART from the stdio driver.
    //
    am_util_stdio_printf_init(0);

    //
    // Power down the UART, and surrender the handle.
    //
    am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_hal_uart_deinitialize(g_sCOMUART);

    //
    // Disable the UART pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, am_hal_gpio_pincfg_disabled);
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_uart_printf_disable()

//*****************************************************************************
//
// UART-based string print function.
//
// This function is used for printing a string via the UART, which for some
// MCU devices may be multi-module.
//
//*****************************************************************************
void
am_bsp_uart_string_print(char *pcString)
{
    uint32_t ui32StrLen = 0;
    uint32_t ui32BytesWritten = 0;

    //
    // Measure the length of the string.
    //
    while (pcString[ui32StrLen] != 0)
    {
        ui32StrLen++;
    }

    //
    // Print the string via the UART.
    //
    const am_hal_uart_transfer_t sUartWrite =
    {
        .eType = AM_HAL_UART_BLOCKING_WRITE,
        .pui8Data = (uint8_t *) pcString,
        .ui32NumBytes = ui32StrLen,
        .ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER,
        .pui32BytesTransferred = &ui32BytesWritten,
    };

    am_hal_uart_transfer(g_sCOMUART, &sUartWrite);

    if (ui32BytesWritten != ui32StrLen)
    {
        //
        // Couldn't send the whole string!!
        //
        while (1);
    }
} // am_bsp_uart_string_print()

//*****************************************************************************
//
// Initialize and configure the UART
//
//*****************************************************************************
int32_t
am_bsp_buffered_uart_printf_enable(void *pvHandle)
{
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;

    return -1;
} // am_bsp_buffered_uart_printf_enable()

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void
am_bsp_buffered_uart_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_BUFFERED_UART)
    {
        return;
    }

    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_buffered_uart_printf_disable()

//*****************************************************************************
//
// Set up the IOM pins based on mode and module.
//
// This function configures up to 10-pins for MSPI serial, dual, quad,
// dual-quad, and octal operation.
//
//*****************************************************************************
void
am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_IOM_NUM_MODULES )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | eIOMMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  g_AM_BSP_GPIO_IOM0_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, g_AM_BSP_GPIO_IOM0_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, g_AM_BSP_GPIO_IOM0_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   g_AM_BSP_GPIO_IOM0_CS);
            break;
        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  g_AM_BSP_GPIO_IOM1_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, g_AM_BSP_GPIO_IOM1_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, g_AM_BSP_GPIO_IOM1_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   g_AM_BSP_GPIO_IOM1_CS);
            break;
        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  g_AM_BSP_GPIO_IOM2_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, g_AM_BSP_GPIO_IOM2_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, g_AM_BSP_GPIO_IOM2_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   g_AM_BSP_GPIO_IOM2_CS);
            break;
        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  g_AM_BSP_GPIO_IOM3_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, g_AM_BSP_GPIO_IOM3_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, g_AM_BSP_GPIO_IOM3_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   g_AM_BSP_GPIO_IOM3_CS);
            break;
        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  g_AM_BSP_GPIO_IOM4_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, g_AM_BSP_GPIO_IOM4_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, g_AM_BSP_GPIO_IOM4_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   g_AM_BSP_GPIO_IOM4_CS);
            break;
        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  g_AM_BSP_GPIO_IOM5_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, g_AM_BSP_GPIO_IOM5_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, g_AM_BSP_GPIO_IOM5_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   g_AM_BSP_GPIO_IOM5_CS);
            break;
        case ((6 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCK,  g_AM_BSP_GPIO_IOM6_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MISO, g_AM_BSP_GPIO_IOM6_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MOSI, g_AM_BSP_GPIO_IOM6_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_CS,   g_AM_BSP_GPIO_IOM6_CS);
            break;
        case ((7 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCK,  g_AM_BSP_GPIO_IOM7_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MISO, g_AM_BSP_GPIO_IOM7_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MOSI, g_AM_BSP_GPIO_IOM7_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_CS,   g_AM_BSP_GPIO_IOM7_CS);
            break;
        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  g_AM_BSP_GPIO_IOM0_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  g_AM_BSP_GPIO_IOM0_SDA);
            break;
        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  g_AM_BSP_GPIO_IOM1_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  g_AM_BSP_GPIO_IOM1_SDA);
            break;
        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  g_AM_BSP_GPIO_IOM2_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  g_AM_BSP_GPIO_IOM2_SDA);
            break;
        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  g_AM_BSP_GPIO_IOM3_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  g_AM_BSP_GPIO_IOM3_SDA);
            break;
        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  g_AM_BSP_GPIO_IOM4_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  g_AM_BSP_GPIO_IOM4_SDA);
            break;
        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  g_AM_BSP_GPIO_IOM5_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  g_AM_BSP_GPIO_IOM5_SDA);
            break;
        case ((6 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCL,  g_AM_BSP_GPIO_IOM6_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SDA,  g_AM_BSP_GPIO_IOM6_SDA);
            break;
        case ((7 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCL,  g_AM_BSP_GPIO_IOM7_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SDA,  g_AM_BSP_GPIO_IOM7_SDA);
            break;
        default:
            break;
    }
} // am_bsp_iom_pins_enable()

//*****************************************************************************
//
//! @brief Disable the IOM pins based on mode and module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_IOM_NUM_MODULES )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | eIOMMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((6 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((7 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_CS,   am_hal_gpio_pincfg_disabled);
            break;

        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((6 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((7 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        default:
            break;
    }
} // am_bsp_iom_pins_disable()

//*****************************************************************************
//
//! @brief Set up the IOS pins based on mode and module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= (AM_REG_IOSLAVE_NUM_MODULES + AM_REG_IOSLAVEFD_NUM_MODULES) )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | ui32IOSMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  g_AM_BSP_GPIO_IOS_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, g_AM_BSP_GPIO_IOS_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, g_AM_BSP_GPIO_IOS_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   g_AM_BSP_GPIO_IOS_CE);
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  g_AM_BSP_GPIO_IOS_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  g_AM_BSP_GPIO_IOS_SDA);
            break;

        case ((1 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_SCK,  g_AM_BSP_GPIO_IOSFD0_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_MISO, g_AM_BSP_GPIO_IOSFD0_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_MOSI, g_AM_BSP_GPIO_IOSFD0_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_CE,   g_AM_BSP_GPIO_IOSFD0_CE);
            break;

        case ((2 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_SCK,  g_AM_BSP_GPIO_IOSFD1_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_MISO, g_AM_BSP_GPIO_IOSFD1_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_MOSI, g_AM_BSP_GPIO_IOSFD1_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_CE,   g_AM_BSP_GPIO_IOSFD1_CE);
            break;

        default:
            break;
    }
} // am_bsp_ios_pins_enable()

//*****************************************************************************
//
//! @brief Disable the IOS pins based on mode and module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= (AM_REG_IOSLAVE_NUM_MODULES + AM_REG_IOSLAVEFD_NUM_MODULES) )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | ui32IOSMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   am_hal_gpio_pincfg_disabled);
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  am_hal_gpio_pincfg_disabled);
            break;

        case ((1 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD0_CE,   am_hal_gpio_pincfg_disabled);
            break;

        case ((2 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOSFD1_CE,   am_hal_gpio_pincfg_disabled);
            break;

        default:
            break;
    }
} // am_bsp_ios_pins_disable()

//*****************************************************************************
//
// Set up I2S pins based on module.
//
//*****************************************************************************
void
am_bsp_i2s_pins_enable(uint32_t ui32Module, bool bBidirectionalData)
{
    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_I2S_NUM_MODULES )
    {
        return;
    }

    switch ( ui32Module )
    {
        case 0:
            if ( bBidirectionalData )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_DATA, g_AM_BSP_GPIO_I2S0_DATA);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_CLK,  g_AM_BSP_GPIO_I2S0_CLK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_WS,   g_AM_BSP_GPIO_I2S0_WS);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_SDOUT, g_AM_BSP_GPIO_I2S0_SDOUT);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_CLK,   g_AM_BSP_GPIO_I2S0_CLK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_WS,    g_AM_BSP_GPIO_I2S0_WS);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_SDIN,  g_AM_BSP_GPIO_I2S0_SDIN);
            }
            break;
        case 1:
            if ( bBidirectionalData )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_DATA, g_AM_BSP_GPIO_I2S1_DATA);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_CLK,  g_AM_BSP_GPIO_I2S1_CLK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_WS,   g_AM_BSP_GPIO_I2S1_WS);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_SDOUT, g_AM_BSP_GPIO_I2S1_SDOUT);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_CLK,   g_AM_BSP_GPIO_I2S1_CLK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_WS,    g_AM_BSP_GPIO_I2S1_WS);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_SDIN,  g_AM_BSP_GPIO_I2S1_SDIN);
            }
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// Disable I2S pins based on module.
//
//*****************************************************************************
void
am_bsp_i2s_pins_disable(uint32_t ui32Module, bool bBidirectionalData)
{
    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_I2S_NUM_MODULES )
    {
        return;
    }

    switch ( ui32Module )
    {
        case 0:
            if ( bBidirectionalData )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_DATA, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_CLK,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_WS,   am_hal_gpio_pincfg_disabled);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_SDOUT, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_CLK,   am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_WS,    am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S0_SDIN,  am_hal_gpio_pincfg_disabled);
            }
            break;
        case 1:
            if ( bBidirectionalData )
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_DATA, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_CLK,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_WS,   am_hal_gpio_pincfg_disabled);
            }
            else
            {
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_SDOUT, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_CLK,   am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_WS,    am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_I2S1_SDIN,  am_hal_gpio_pincfg_disabled);
            }
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// Set up PDM pins based on module.
//
//*****************************************************************************
void am_bsp_pdm_pins_enable(uint32_t ui32Module)
{
    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_PDM_NUM_MODULES )
    {
        return;
    }

    switch ( ui32Module )
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_PDM0_CLK,  g_AM_BSP_GPIO_PDM0_CLK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_PDM0_DATA, g_AM_BSP_GPIO_PDM0_DATA);
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// Disable PDM pins based on module.
//
//*****************************************************************************
void am_bsp_pdm_pins_disable(uint32_t ui32Module)
{
    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_PDM_NUM_MODULES )
    {
        return;
    }

    switch ( ui32Module )
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_PDM0_CLK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_PDM0_DATA, am_hal_gpio_pincfg_disabled);
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// Set up the MSPI pins based on the external flash device type with clock
// on data pin 4.
//
//*****************************************************************************
void
am_bsp_mspi_clkond4_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    //
                    // setting clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, g_AM_BSP_GPIO_MSPI0_D4_CLK);
                    break;
                // MSPI0 only supports CE0 in Rev A silicon.
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, g_AM_BSP_GPIO_MSPI0_CE1);
                    //
                    // setting clock on data pin 4.
                    //
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, g_AM_BSP_GPIO_MSPI0_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4_CLK);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, g_AM_BSP_GPIO_MSPI2_D4_CLK);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configurations could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, g_AM_BSP_GPIO_MSPI2_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, g_AM_BSP_GPIO_MSPI3_D4_CLK);
                    break;
                // MSPI3 only supports CE0 in Rev A silicon.
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configuration could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, g_AM_BSP_GPIO_MSPI3_CE1);
                    //
                    // clock on data pin 4.
                    //
                    //am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, g_AM_BSP_GPIO_MSPI3_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_clkond4_pins_enable()

//*****************************************************************************
//
// Disable the MSPI pins based on the external flash device type with clock
// on data pin 4.
//
//*****************************************************************************
void
am_bsp_mspi_clkond4_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // setting clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // setting clock on data pin 4.
                    //
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configuration could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configuration could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_clkond4_pins_disable()

//*****************************************************************************
//
// Set up the MSPI pins based on the external flash device type.
//
// Note that Rev A silicon only supports CE0 for MSPI0 & MSPI3.
// CE1 cases for these have been commented out for ease of addition later.
//
//*****************************************************************************
void
am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  g_AM_BSP_GPIO_MSPI0_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  g_AM_BSP_GPIO_MSPI0_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  g_AM_BSP_GPIO_MSPI0_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  g_AM_BSP_GPIO_MSPI0_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  g_AM_BSP_GPIO_MSPI0_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  g_AM_BSP_GPIO_MSPI0_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  g_AM_BSP_GPIO_MSPI0_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  g_AM_BSP_GPIO_MSPI0_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, g_AM_BSP_GPIO_MSPI0_DQS1DM1);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_BSP_GPIO_MSPI0_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_BSP_GPIO_MSPI0_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_BSP_GPIO_MSPI0_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_BSP_GPIO_MSPI0_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  g_AM_BSP_GPIO_MSPI0_D8);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  g_AM_BSP_GPIO_MSPI0_D9);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  g_AM_BSP_GPIO_MSPI0_D10);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  g_AM_BSP_GPIO_MSPI0_D11);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  g_AM_BSP_GPIO_MSPI0_D12);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  g_AM_BSP_GPIO_MSPI0_D13);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  g_AM_BSP_GPIO_MSPI0_D14);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  g_AM_BSP_GPIO_MSPI0_D15);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, g_AM_BSP_GPIO_MSPI0_DQS1DM1);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_BSP_GPIO_MSPI0_D4);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_BSP_GPIO_MSPI0_D5);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_BSP_GPIO_MSPI0_D6);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_BSP_GPIO_MSPI0_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, g_AM_BSP_GPIO_MSPI0_CE1);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_1_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_BSP_GPIO_MSPI1_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_BSP_GPIO_MSPI1_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_BSP_GPIO_MSPI1_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1_1_1_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1_1_8_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_BSP_GPIO_MSPI1_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_BSP_GPIO_MSPI1_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_BSP_GPIO_MSPI1_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_1_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1_1_1_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1_1_8_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  g_AM_BSP_GPIO_MSPI3_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  g_AM_BSP_GPIO_MSPI3_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  g_AM_BSP_GPIO_MSPI3_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  g_AM_BSP_GPIO_MSPI3_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  g_AM_BSP_GPIO_MSPI3_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  g_AM_BSP_GPIO_MSPI3_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  g_AM_BSP_GPIO_MSPI3_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  g_AM_BSP_GPIO_MSPI3_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, g_AM_BSP_GPIO_MSPI3_DQS1DM1);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  g_AM_BSP_GPIO_MSPI3_D8);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  g_AM_BSP_GPIO_MSPI3_D9);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  g_AM_BSP_GPIO_MSPI3_D10);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  g_AM_BSP_GPIO_MSPI3_D11);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  g_AM_BSP_GPIO_MSPI3_D12);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  g_AM_BSP_GPIO_MSPI3_D13);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  g_AM_BSP_GPIO_MSPI3_D14);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  g_AM_BSP_GPIO_MSPI3_D15);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, g_AM_BSP_GPIO_MSPI3_DQS1DM1);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, g_AM_BSP_GPIO_MSPI3_CE1);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_pins_enable()

//*****************************************************************************
//
// Disable the MSPI pins based on the external flash device type.
//
//*****************************************************************************
void
am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK*/
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK*/
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    // am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_pins_disable()

//*****************************************************************************
//
// Return the pinnum and pincfg for the CE of MSPI requested.
//
//*****************************************************************************
void
am_bsp_mspi_ce_pincfg_get(uint32_t ui32Module,
                          am_hal_mspi_device_e eMSPIDevice,
                          uint32_t * pPinnum,
                          am_hal_gpio_pincfg_t * pPincfg)
{
    switch (ui32Module)
    {
        case PINCFG_GET_DEPRECATED:
            break;
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI0_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI0_CE0;
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    //*pPinnum = AM_BSP_GPIO_MSPI0_CE1;
                    //*pPincfg = g_AM_BSP_GPIO_MSPI0_CE1;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI1_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI1_CE0;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI2_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI2_CE0;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI3_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI3_CE0;
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    //*pPinnum = AM_BSP_GPIO_MSPI3_CE1;
                    //*pPincfg = g_AM_BSP_GPIO_MSPI3_CE1;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_ce_pincfg_get()

//*****************************************************************************
//
// Return the pinnum and pincfg for the Reset of MSPI requested.
//
//*****************************************************************************
void
am_bsp_mspi_reset_pincfg_get(uint32_t ui32Module,
                             am_hal_mspi_device_e eMSPIDevice,
                             uint32_t * pPinnum,
                             am_hal_gpio_pincfg_t * pPincfg )
{
    switch (ui32Module)
    {
        case PINCFG_GET_DEPRECATED:
            break;
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI0_RST;
                    *pPincfg = g_AM_BSP_GPIO_MSPI0_RST;
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    //*pPinnum = AM_BSP_GPIO_MSPI0_RST;
                    //*pPincfg = g_AM_BSP_GPIO_MSPI0_RST;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI1_RST;
                    *pPincfg = g_AM_BSP_GPIO_MSPI1_RST;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI2_RST;
                    *pPincfg = g_AM_BSP_GPIO_MSPI2_RST;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    *pPinnum = AM_BSP_GPIO_MSPI3_RST;
                    *pPincfg = g_AM_BSP_GPIO_MSPI3_RST;
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    // *pPinnum = AM_BSP_GPIO_MSPI3_RST;
                    // *pPincfg = g_AM_BSP_GPIO_MSPI3_RST;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
//  Clear MSPI1 and MSPI2 Loadswitch clear
//
//*****************************************************************************
void
am_bsp_mspi_loadswitch_clear(uint32_t ui32Module)
{
    switch (ui32Module)
    {
        case 1:
            am_hal_gpio_output_clear(AM_BSP_GPIO_MSPI1_LS_EN);
            break;
        case 2:
            am_hal_gpio_output_clear(AM_BSP_GPIO_MSPI2_LS_EN);
            break;
        default:
            break;
    }
} // am_bsp_mspi_loadswitch_clear()

//*****************************************************************************
//
//  Clear MSPI1 and MSPI2 Loadswitch Set
//
//*****************************************************************************
void
am_bsp_mspi_loadswitch_set(uint32_t ui32Module)
{
    switch (ui32Module)
    {
        case 1:
            am_hal_gpio_output_set(AM_BSP_GPIO_MSPI1_LS_EN);
            break;
        case 2:
            am_hal_gpio_output_set(AM_BSP_GPIO_MSPI2_LS_EN);
            break;
        default:
            break;
    }
} // am_bsp_mspi_loadswitch_set()


//*****************************************************************************
//
// Checks if itm is busy and provides a delay to flush the fifo
//
// Return: true  = ITM not busy and no timeout occurred.
//         false = Timeout occurred.
//
//*****************************************************************************
bool
am_bsp_debug_itm_printf_flush(void)
{
    bool bRet1, bRet2;
    bRet1 = am_hal_itm_print_not_busy();
    bRet2 = am_hal_itm_not_busy();

    return bRet1 & bRet2;
}
