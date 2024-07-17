//*****************************************************************************
//
//  am_bsp.c
//! @file
//!
//! @brief Top level functions for performing board initialization.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollop4p_evb_disp_shield_bsp_rev2 BSP for the Apollo4 Display Shield
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
//
//*****************************************************************************

#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Configuration.
//
//*****************************************************************************
#define AM_BSP_ITM_FREQUENCY     AM_HAL_TPIU_BAUD_1M

//*****************************************************************************
//
// LEDs
//
//*****************************************************************************
#if AM_BSP_NUM_LEDS
am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS] =
{
    {AM_BSP_GPIO_LED0, AM_DEVICES_LED_ON_LOW | AM_DEVICES_LED_POL_OPEN_DRAIN},
    {AM_BSP_GPIO_LED1, AM_DEVICES_LED_ON_LOW | AM_DEVICES_LED_POL_OPEN_DRAIN},
    {AM_BSP_GPIO_LED2, AM_DEVICES_LED_ON_LOW | AM_DEVICES_LED_POL_OPEN_DRAIN}
};
#endif

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
    .ui16ResX = 454,
    .ui16ResY = 454,

#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
    .eInterface = DISP_IF_QSPI,
    .ui32Module = DISPLAY_MSPI_INST,
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .bClockonD4 = true,
#endif

#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    .eInterface = DISP_IF_DSI,
    .eDsiFreq  = AM_HAL_DSI_FREQ_TRIM_X20,
    .eDbiWidth = AM_HAL_DSI_DBI_WIDTH_16,
    .ui8NumLanes = 1,
#endif

    .eTEType = DISP_TE_DC,
    .ui16Offset = 14,
    .bFlip = false
};
#endif // DISP_CTRL_IP

//*****************************************************************************
//
//! @brief DSI VDD18 power switch.
//!
//! @return None.
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
//! @brief VDDUSB33 power switch.
//!
//! @return None.
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
//! @brief VDDUSB0P9 power switch.
//!
//! @return None.
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
//! @brief Display device enable on the apollop4p_evb_disp_shield board.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_external_display_device_enable(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_EN, g_AM_BSP_GPIO_DISP_DEVICE_EN);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_DEVICE_EN, eGpioWrType);
}

//*****************************************************************************
//
//! @brief Accelerometer device enable on the apollop4p_evb_disp_shield board.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_external_accel_device_enable(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_DEVICE_EN, g_AM_BSP_GPIO_ACCEL_DEVICE_EN);
    am_hal_gpio_state_write(AM_BSP_GPIO_ACCEL_DEVICE_EN, eGpioWrType);
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
#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
// Default UART configuration settings if using buffers.
//
//*****************************************************************************
#define AM_BSP_UART_BUFFER_SIZE     1024
static uint8_t pui8UartTxBuffer[AM_BSP_UART_BUFFER_SIZE];
static uint8_t pui8UartRxBuffer[AM_BSP_UART_BUFFER_SIZE];
#endif // AM_BSP_DISABLE_BUFFERED_UART

//*****************************************************************************
//
//! @brief Prepare the MCU for low power operation.
//!
//! This function enables several power-saving features of the MCU, and
//! disables some of the less-frequently used peripherals.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_low_power_init(void)
{
    //
    // Disable the ITM interface.
    //
    //am_bsp_itm_printf_disable();

    //
    // Initialize for low power in the power control block
    //
    am_hal_pwrctrl_low_power_init();

#if AM_BSP_ENABLE_SIMOBUCK
    //
    // Enable SIMOBUCK for this board
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_SIMOBUCK_INIT, 0);
#endif

    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();

} // am_bsp_low_power_init()

//*****************************************************************************
//
//! @brief Enable the TPIU and ITM for debug printf messages.
//!
//! This function enables TPIU registers for debug printf messages and enables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
//!
//! @return 0 on success.
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
            return am_bsp_buffered_uart_printf_enable();

        default:
            break;
    }

    return -1;

} // am_bsp_debug_printf_enable()

//*****************************************************************************
//
//! @brief Disable the TPIU and ITM for debug printf messages.
//!
//! This function disables TPIU registers for debug printf messages and disables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
//!
//! @return None.
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
//! @brief Enable printing over ITM.
//
//! @return 0 on success.
//
//*****************************************************************************
int32_t
am_bsp_itm_printf_enable(void)
{
    uint32_t ui32dcuVal;

    //
    // Need to make sure that SWO is enabled
    //
    {
        if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 1) && (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 1))
        {
            am_hal_dcu_get(&ui32dcuVal);
            // Enable SWO
            if ( !(ui32dcuVal & AM_HAL_DCU_CPUTRC_TPIU_SWO) &&
                 (am_hal_dcu_update(true, AM_HAL_DCU_CPUTRC_TPIU_SWO) != AM_HAL_STATUS_SUCCESS) )
            {
                // Cannot enable SWO
                return -1;
            }
        }
        else
        {
            // If DCU is not accessible, we cannot determine if ITM can be safely enabled.
            return -1;
        }
    }

    //
    // Set the global print interface.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_SWO;

    //
    // Enable the ITM interface and the SWO pin.
    //
    am_hal_itm_enable();

    am_hal_tpiu_enable(AM_BSP_ITM_FREQUENCY);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO);

    //
    // Attach the ITM to the STDIO driver.
    //
    am_util_stdio_printf_init(am_hal_itm_print);

    return 0;

} // am_bsp_itm_printf_enable()

//*****************************************************************************
//
// @brief Disable printing over ITM.
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
    am_hal_itm_not_busy();

    am_hal_gpio_state_write(AM_BSP_GPIO_ITM_SWO, AM_HAL_GPIO_OUTPUT_SET);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, am_hal_gpio_pincfg_output);

    //
    // Disable the ITM/TPIU
    //
    am_hal_itm_disable();

    //
    // Detach the ITM interface from the STDIO driver.
    //
    am_util_stdio_printf_init(0);

    //
    // Disconnect the SWO pin
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, am_hal_gpio_pincfg_disabled);
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_itm_printf_disable()

#if defined (DISP_CTRL_IP)
//*****************************************************************************
//
//! @brief Set up the display pins.
//
//*****************************************************************************
void
am_bsp_disp_pins_enable(void)
{
    am_hal_gpio_write_type_e eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_write_type_e eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)

    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_QSPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_CS,  g_AM_BSP_GPIO_DISP_QSPI_CS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D0,  g_AM_BSP_GPIO_DISP_QSPI_D0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D1,  g_AM_BSP_GPIO_DISP_QSPI_D1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D2,  g_AM_BSP_GPIO_DISP_QSPI_D2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D3,  g_AM_BSP_GPIO_DISP_QSPI_D3);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_SCK, g_AM_BSP_GPIO_DISP_QSPI_SCK);
            //
            // Set IM2 = 1, IM1 = 0 select to QSPI mode
            //
            eGpioType2 = AM_HAL_GPIO_OUTPUT_SET;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
            break;
        case DISP_IF_DSPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_CS,  g_AM_BSP_GPIO_DISP_DSPI_CS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_D0,  g_AM_BSP_GPIO_DISP_DSPI_D0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_D1,  g_AM_BSP_GPIO_DISP_DSPI_D1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_SCK, g_AM_BSP_GPIO_DISP_DSPI_SCK);
            //
            // Set IM2 = 0, IM1 = 1 select to SPI4/DSPI mode
            //
            eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_SET;
            break;
        case DISP_IF_SPI4:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_CS,  g_AM_BSP_GPIO_DISP_SPI_CS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SD,  g_AM_BSP_GPIO_DISP_SPI_SD);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_DCX, g_AM_BSP_GPIO_DISP_SPI_DCX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SCK, g_AM_BSP_GPIO_DISP_SPI_SCK);
            //
            // Set IM2 = 0, IM1 = 1 select to SPI4 mode
            //
            eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_SET;
            break;
        case DISP_IF_DSI:
            eGpioType2 = AM_HAL_GPIO_OUTPUT_CLEAR;
            eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
            break;
        default:
            break;
    }
#endif
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSI_TE,  g_AM_BSP_GPIO_DISP_DSI_TE);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL1, g_AM_BSP_GPIO_DISP_IFSEL1);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL2, g_AM_BSP_GPIO_DISP_IFSEL2);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_RST, g_AM_BSP_GPIO_DISP_DEVICE_RST);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_EN, g_AM_BSP_GPIO_DISP_DEVICE_EN);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_INT, g_AM_BSP_GPIO_TOUCH_INT);
#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)

    if ( g_sDispCfg.eDeviceConfig == AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4 )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
    }
    else if ( g_sDispCfg.eDeviceConfig == AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4 )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
    }

    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);

    if ( g_sDispCfg.bClockonD4 == true )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4_CLK, g_AM_BSP_GPIO_MSPI2_D4_CLK);
    }
    else
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
    }

    //
    // Set RES pin high
    //
    am_hal_gpio_output_set(AM_BSP_GPIO_DISP_DEVICE_RST);
    //
    // Set IM2 = 1, IM1 = 0 select to QSPI mode
    //
    eGpioType2 = AM_HAL_GPIO_OUTPUT_SET;
    eGpioType1 = AM_HAL_GPIO_OUTPUT_CLEAR;
#endif
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL2, eGpioType2);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL1, eGpioType1);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_DEVICE_EN, AM_HAL_GPIO_OUTPUT_SET);
} // am_bsp_disp_pins_enable()

//*****************************************************************************
//
//! @brief Disable the display pins.
//
//*****************************************************************************
void
am_bsp_disp_pins_disable(void)
{
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_QSPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_CS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D0,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D1,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D2,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D3,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_SCK, am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_DSPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_CS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_D0,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_D1,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSPI_SCK, am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_SPI4:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_CS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SD,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_DCX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SCK, am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_DSI:
            break;
        default:
            break;
    }

#endif
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSI_TE,  am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_RST, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_EN, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL1, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL2, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_TOUCH_INT, am_hal_gpio_pincfg_disabled);
#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
    if ( g_sDispCfg.eDeviceConfig == AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4 )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
    }
    else if ( g_sDispCfg.eDeviceConfig == AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4 )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
    }

    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);

    if ( g_sDispCfg.bClockonD4 == true )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4_CLK, am_hal_gpio_pincfg_disabled);
    }
    else
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
    }
#endif
} // am_bsp_disp_pins_disable()

#endif // DISP_CTRL_IP
//*****************************************************************************
//
//! @brief Set display reset pins.
//
//*****************************************************************************
static inline void
am_bsp_disp_reset_pin_output_set(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;

    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;

    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_DEVICE_RST, eGpioWrType);
} // am_bsp_disp_reset_pin_output_set()

//*****************************************************************************
//
//! @brief Set display reset pins.
//
//*****************************************************************************
void
am_bsp_disp_reset_pins_set(void)
{
    am_bsp_disp_reset_pin_output_set(true);
} // am_bsp_disp_reset_pins_set()

//*****************************************************************************
//
//! @brief Set display reset pins.
//
//*****************************************************************************
void
am_bsp_disp_reset_pins_clear(void)
{
    am_bsp_disp_reset_pin_output_set(false);
} // am_bsp_disp_reset_pins_clear()

//*****************************************************************************
//
//! @brief Set up the SDIO's pins.
//!
//! This function configures SDIO's CMD, CLK, DAT0-7 pins
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_sdio_pins_enable(uint8_t ui8BusWidth)
{
    am_hal_gpio_state_write(AM_BSP_GPIO_SDIO_RST, AM_HAL_GPIO_OUTPUT_SET);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_CMD,  g_AM_BSP_GPIO_SDIO_CMD);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_CLK,  g_AM_BSP_GPIO_SDIO_CLK);

    switch (ui8BusWidth)
    {
        case AM_HAL_HOST_BUS_WIDTH_8:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT4, g_AM_BSP_GPIO_SDIO_DAT4);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT5, g_AM_BSP_GPIO_SDIO_DAT5);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT6, g_AM_BSP_GPIO_SDIO_DAT6);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT7, g_AM_BSP_GPIO_SDIO_DAT7);
        case AM_HAL_HOST_BUS_WIDTH_4:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT1, g_AM_BSP_GPIO_SDIO_DAT1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT2, g_AM_BSP_GPIO_SDIO_DAT2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT3, g_AM_BSP_GPIO_SDIO_DAT3);
        case AM_HAL_HOST_BUS_WIDTH_1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT0, g_AM_BSP_GPIO_SDIO_DAT0);
            break;
    }

} // am_bsp_sdio_pins_enable()


void
am_bsp_sdio_pins_disable(uint8_t ui8BusWidth)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_CMD,  am_hal_gpio_pincfg_default);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_CLK,  am_hal_gpio_pincfg_default);

    switch (ui8BusWidth)
    {
        case AM_HAL_HOST_BUS_WIDTH_8:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT4, am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT5, am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT6, am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT7, am_hal_gpio_pincfg_default);
        case AM_HAL_HOST_BUS_WIDTH_4:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT1, am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT2, am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT3, am_hal_gpio_pincfg_default);
        case AM_HAL_HOST_BUS_WIDTH_1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_DAT0, am_hal_gpio_pincfg_default);
            break;
    }
} // am_bsp_sdio_pins_disable()

//*****************************************************************************
//
//! @brief Set up the SDIO reset pins.
//!
//! Reset SDIO device via GPIO
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_sdio_reset(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO_RST, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(AM_BSP_GPIO_SDIO_RST);
    am_hal_delay_us(10000);
    am_hal_gpio_output_set(AM_BSP_GPIO_SDIO_RST);
} // am_bsp_sdio_reset()

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
am_bsp_sd_cd_pin_enable(bool bEnable)
{
    if ( bEnable )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_CD, am_hal_gpio_pincfg_input);
        am_hal_gpio_cd_pin_config(AM_BSP_GPIO_SD_CD);
    }
    else
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_CD, am_hal_gpio_pincfg_default);
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
am_bsp_sd_wp_pin_enable(bool bEnable)
{
    if ( bEnable )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_WP, am_hal_gpio_pincfg_input);
        am_hal_gpio_wp_pin_config(AM_BSP_GPIO_SD_WP);
    }
    else
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_WP, am_hal_gpio_pincfg_default);
    }
} // am_bsp_sd_wp_pin_enable()

//*****************************************************************************
//
//! @brief External power on.
//!
//! This function turns on external power switch
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_external_pwr_on(void)
{
} // am_bsp_external_pwr_on()

//*****************************************************************************
//
//! Initialize and configure the UART
//
//! @return 0 on success.
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
//! @brief UART-based string print function.
//!
//! This function is used for printing a string via the UART, which for some
//! MCU devices may be multi-module.
//!
//! @return None.
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

#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
//! Initialize and configure the UART
//
//! @return 0 on success.
//
//*****************************************************************************
int32_t
am_bsp_buffered_uart_printf_enable(void)
{
    //
    // Initialize, power up, and configure the communication UART. Use the
    // custom configuration if it was provided. Otherwise, just use the default
    // configuration.
    //
    if (am_hal_uart_initialize(AM_BSP_UART_PRINT_INST, &g_sCOMUART) ||
        am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_uart_configure(g_sCOMUART, &g_sBspUartConfig) ||
        am_hal_uart_buffer_configure(g_sCOMUART, pui8UartTxBuffer, sizeof(pui8UartTxBuffer), pui8UartRxBuffer, sizeof(pui8UartRxBuffer)) ||

        //
        // Enable the UART pins.
        //
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX) ||
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX) )
    {
        return -1;
    }

    //
    // Register the BSP print function to the STDIO driver.
    //
    am_util_stdio_printf_init(am_bsp_uart_string_print);

    //
    // Enable the interrupts for the UART.
    //
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST));

    //
    // Set the global print interface.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_BUFFERED_UART;

    return 0;

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

    //
    // Disable the interrupts for the UART.
    //
    NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST));
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_buffered_uart_printf_disable()

//*****************************************************************************
//
// Interrupt routine for the buffered UART interface.
//
//*****************************************************************************
void
am_bsp_buffered_uart_service(void)
{
    uint32_t ui32Status;
    am_hal_uart_interrupt_status_get(g_sCOMUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_sCOMUART, ui32Status);
    am_hal_uart_interrupt_service(g_sCOMUART, ui32Status);
} // am_bsp_buffered_uart_service()
#endif // AM_BSP_DISABLE_BUFFERED_UART

//*****************************************************************************
//
//! @brief Set up the IOM pins based on mode and module.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
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
#if defined(AM_BSP_GPIO_IOM0_SCK)
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  g_AM_BSP_GPIO_IOM0_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, g_AM_BSP_GPIO_IOM0_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, g_AM_BSP_GPIO_IOM0_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   g_AM_BSP_GPIO_IOM0_CS);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCK */

#if defined(AM_BSP_GPIO_IOM1_SCK)
        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  g_AM_BSP_GPIO_IOM1_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, g_AM_BSP_GPIO_IOM1_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, g_AM_BSP_GPIO_IOM1_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   g_AM_BSP_GPIO_IOM1_CS);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCK */

#if defined(AM_BSP_GPIO_IOM2_SCK)
        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  g_AM_BSP_GPIO_IOM2_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, g_AM_BSP_GPIO_IOM2_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, g_AM_BSP_GPIO_IOM2_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   g_AM_BSP_GPIO_IOM2_CS);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCK */

#if defined(AM_BSP_GPIO_IOM3_SCK)
        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  g_AM_BSP_GPIO_IOM3_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, g_AM_BSP_GPIO_IOM3_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, g_AM_BSP_GPIO_IOM3_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   g_AM_BSP_GPIO_IOM3_CS);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCK */

#if defined(AM_BSP_GPIO_IOM4_SCK)
        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  g_AM_BSP_GPIO_IOM4_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, g_AM_BSP_GPIO_IOM4_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, g_AM_BSP_GPIO_IOM4_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   g_AM_BSP_GPIO_IOM4_CS);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCK */

#if defined(AM_BSP_GPIO_IOM5_SCK)
        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  g_AM_BSP_GPIO_IOM5_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, g_AM_BSP_GPIO_IOM5_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, g_AM_BSP_GPIO_IOM5_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   g_AM_BSP_GPIO_IOM5_CS);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCK */

#if defined(AM_BSP_GPIO_IOM6_SCK)
        case ((6 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCK,  g_AM_BSP_GPIO_IOM6_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MISO, g_AM_BSP_GPIO_IOM6_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MOSI, g_AM_BSP_GPIO_IOM6_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_CS,   g_AM_BSP_GPIO_IOM6_CS);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCK */

#if defined(AM_BSP_GPIO_IOM7_SCK)
        case ((7 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCK,  g_AM_BSP_GPIO_IOM7_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MISO, g_AM_BSP_GPIO_IOM7_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MOSI, g_AM_BSP_GPIO_IOM7_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_CS,   g_AM_BSP_GPIO_IOM7_CS);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCK */

#if defined(AM_BSP_GPIO_IOM0_SCL)
        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  g_AM_BSP_GPIO_IOM0_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  g_AM_BSP_GPIO_IOM0_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCL */

#if defined(AM_BSP_GPIO_IOM1_SCL)
        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  g_AM_BSP_GPIO_IOM1_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  g_AM_BSP_GPIO_IOM1_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCL */

#if defined(AM_BSP_GPIO_IOM2_SCL)
        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  g_AM_BSP_GPIO_IOM2_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  g_AM_BSP_GPIO_IOM2_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCL */

#if defined(AM_BSP_GPIO_IOM3_SCL)
        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  g_AM_BSP_GPIO_IOM3_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  g_AM_BSP_GPIO_IOM3_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCL */

#if defined(AM_BSP_GPIO_IOM4_SCL)
        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  g_AM_BSP_GPIO_IOM4_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  g_AM_BSP_GPIO_IOM4_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCL */

#if defined(AM_BSP_GPIO_IOM5_SCL)
        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  g_AM_BSP_GPIO_IOM5_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  g_AM_BSP_GPIO_IOM5_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCL */

#if defined(AM_BSP_GPIO_IOM6_SCL)
        case ((6 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCL,  g_AM_BSP_GPIO_IOM6_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SDA,  g_AM_BSP_GPIO_IOM6_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCL */

#if defined(AM_BSP_GPIO_IOM7_SCL)
        case ((7 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCL,  g_AM_BSP_GPIO_IOM7_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SDA,  g_AM_BSP_GPIO_IOM7_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCL */

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
#if defined(AM_BSP_GPIO_IOM0_SCK)
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCK */

#if defined(AM_BSP_GPIO_IOM1_SCK)
        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCK */

#if defined(AM_BSP_GPIO_IOM2_SCK)
        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCK */

#if defined(AM_BSP_GPIO_IOM3_SCK)
        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCK */

#if defined(AM_BSP_GPIO_IOM4_SCK)
        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCK */

#if defined(AM_BSP_GPIO_IOM5_SCK)
        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCK */

#if defined(AM_BSP_GPIO_IOM6_SCK)
        case ((6 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCK */

#if defined(AM_BSP_GPIO_IOM7_SCK)
        case ((7 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCK */

#if defined(AM_BSP_GPIO_IOM0_SCL)
        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCL */

#if defined(AM_BSP_GPIO_IOM1_SCL)
        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCL */

#if defined(AM_BSP_GPIO_IOM2_SCL)
        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCL */

#if defined(AM_BSP_GPIO_IOM3_SCL)
        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCL */

#if defined(AM_BSP_GPIO_IOM4_SCL)
        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCL */

#if defined(AM_BSP_GPIO_IOM5_SCL)
        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCL */

#if defined(AM_BSP_GPIO_IOM6_SCL)
        case ((6 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCL */

#if defined(AM_BSP_GPIO_IOM7_SCL)
        case ((7 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCL */

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
    if ( ui32Module >= AM_REG_IOSLAVE_NUM_MODULES )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | ui32IOSMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOS_USE_SPI):
#if defined(AM_BSP_GPIO_IOS_SCK)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  g_AM_BSP_GPIO_IOS_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, g_AM_BSP_GPIO_IOS_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, g_AM_BSP_GPIO_IOS_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   g_AM_BSP_GPIO_IOS_CE);
#endif /* AM_BSP_GPIO_IOS_SCK */
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
#if defined(AM_BSP_GPIO_IOS_SCL)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  g_AM_BSP_GPIO_IOS_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  g_AM_BSP_GPIO_IOS_SDA);
#endif /* AM_BSP_GPIO_IOS_SCL */
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
    if ( ui32Module >= AM_REG_IOSLAVE_NUM_MODULES )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | ui32IOSMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOS_USE_SPI):
#if defined(AM_BSP_GPIO_IOS_SCK)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   am_hal_gpio_pincfg_disabled);
#endif /* AM_BSP_GPIO_IOS_SCK */
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
#if defined(AM_BSP_GPIO_IOS_SCL)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  am_hal_gpio_pincfg_disabled);
#endif /* AM_BSP_GPIO_IOS_SCL */
            break;

        default:
            break;
    }
} // am_bsp_ios_pins_disable()

//*****************************************************************************
//
//! @brief Set up the MSPI mux switch to the external flash device.
//!
//! @return None.
//
//*****************************************************************************
static void
am_bsp_mspi1_mux_flash_enable(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_MUX_SEL, g_AM_BSP_GPIO_MSPI1_MUX_SEL);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_MUX_OE, g_AM_BSP_GPIO_MSPI1_MUX_OE);
    am_hal_gpio_state_write(AM_BSP_GPIO_MSPI1_MUX_SEL, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(AM_BSP_GPIO_MSPI1_MUX_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
} // am_bsp_mspi1_mux_flash_enable()

//*****************************************************************************
//
//! @brief Set up the MSPI mux switch to the external psram device.
//!
//! @return None.
//
//*****************************************************************************
static void
am_bsp_mspi1_mux_psram_enable(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_MUX_SEL, g_AM_BSP_GPIO_MSPI1_MUX_SEL);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_MUX_OE, g_AM_BSP_GPIO_MSPI1_MUX_OE);
    am_hal_gpio_state_write(AM_BSP_GPIO_MSPI1_MUX_SEL, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(AM_BSP_GPIO_MSPI1_MUX_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
} // am_bsp_mspi1_mux_psram_enable()


//*****************************************************************************
//
//! @brief Set up the MSPI mux switch disable.
//!
//! @return None.
//
//*****************************************************************************
static void
am_bsp_mspi1_mux_disable(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_MUX_SEL, g_AM_BSP_GPIO_MSPI1_MUX_SEL);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_MUX_OE, g_AM_BSP_GPIO_MSPI1_MUX_OE);
    am_hal_gpio_state_write(AM_BSP_GPIO_MSPI1_MUX_SEL, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(AM_BSP_GPIO_MSPI1_MUX_OE, AM_HAL_GPIO_OUTPUT_SET);
} // am_bsp_mspi1_mux_disable()


//*****************************************************************************
//
//! @brief Set up the MSPI pins based on the external flash device type.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_PSRAM_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, g_AM_BSP_GPIO_PSRAM_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  g_AM_BSP_GPIO_PSRAM_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  g_AM_BSP_GPIO_PSRAM_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, g_AM_BSP_GPIO_PSRAM_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, g_AM_BSP_GPIO_PSRAM_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  g_AM_BSP_GPIO_PSRAM_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  g_AM_BSP_GPIO_PSRAM_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  g_AM_BSP_GPIO_PSRAM_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  g_AM_BSP_GPIO_PSRAM_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, g_AM_BSP_GPIO_PSRAM_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, g_AM_BSP_GPIO_PSRAM_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  g_AM_BSP_GPIO_PSRAM_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  g_AM_BSP_GPIO_PSRAM_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  g_AM_BSP_GPIO_PSRAM_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  g_AM_BSP_GPIO_PSRAM_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D4,  g_AM_BSP_GPIO_PSRAM_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D5,  g_AM_BSP_GPIO_PSRAM_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D6,  g_AM_BSP_GPIO_PSRAM_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D7,  g_AM_BSP_GPIO_PSRAM_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, g_AM_BSP_GPIO_PSRAM_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS0DM0, g_AM_BSP_GPIO_PSRAM_DQS0DM0);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, g_AM_BSP_GPIO_PSRAM_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  g_AM_BSP_GPIO_PSRAM_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  g_AM_BSP_GPIO_PSRAM_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  g_AM_BSP_GPIO_PSRAM_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  g_AM_BSP_GPIO_PSRAM_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D4,  g_AM_BSP_GPIO_PSRAM_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D5,  g_AM_BSP_GPIO_PSRAM_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D6,  g_AM_BSP_GPIO_PSRAM_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D7,  g_AM_BSP_GPIO_PSRAM_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, g_AM_BSP_GPIO_PSRAM_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS0DM0, g_AM_BSP_GPIO_PSRAM_DQS0DM0);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_bsp_mspi1_mux_psram_enable();
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, g_AM_BSP_GPIO_PSRAM_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  g_AM_BSP_GPIO_PSRAM_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  g_AM_BSP_GPIO_PSRAM_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  g_AM_BSP_GPIO_PSRAM_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  g_AM_BSP_GPIO_PSRAM_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D4,  g_AM_BSP_GPIO_PSRAM_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D5,  g_AM_BSP_GPIO_PSRAM_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D6,  g_AM_BSP_GPIO_PSRAM_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D7,  g_AM_BSP_GPIO_PSRAM_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D8,  g_AM_BSP_GPIO_PSRAM_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D9,  g_AM_BSP_GPIO_PSRAM_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D10,  g_AM_BSP_GPIO_PSRAM_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D11,  g_AM_BSP_GPIO_PSRAM_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D12,  g_AM_BSP_GPIO_PSRAM_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D13,  g_AM_BSP_GPIO_PSRAM_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D14,  g_AM_BSP_GPIO_PSRAM_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D15,  g_AM_BSP_GPIO_PSRAM_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, g_AM_BSP_GPIO_PSRAM_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS0DM0, g_AM_BSP_GPIO_PSRAM_DQS0DM0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS1DM1, g_AM_BSP_GPIO_PSRAM_DQS1DM1);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    break;
                default:
                    break;
            }
        break;
#endif /* AM_BSP_GPIO_MSPI0_SCK */
    case 1:
#if (AM_BSP_GPIO_NOR_SCK)
        am_bsp_mspi1_mux_flash_enable();
        switch ( eMSPIDevice )
        {
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, g_AM_BSP_GPIO_NOR_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  g_AM_BSP_GPIO_NOR_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  g_AM_BSP_GPIO_NOR_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, g_AM_BSP_GPIO_NOR_SCK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_DQS, g_AM_BSP_GPIO_NOR_DQS);
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, g_AM_BSP_GPIO_NOR_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  g_AM_BSP_GPIO_NOR_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  g_AM_BSP_GPIO_NOR_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  g_AM_BSP_GPIO_NOR_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  g_AM_BSP_GPIO_NOR_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, g_AM_BSP_GPIO_NOR_SCK);
                break;
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, g_AM_BSP_GPIO_NOR_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  g_AM_BSP_GPIO_NOR_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  g_AM_BSP_GPIO_NOR_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  g_AM_BSP_GPIO_NOR_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  g_AM_BSP_GPIO_NOR_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D4,  g_AM_BSP_GPIO_NOR_D4);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D5,  g_AM_BSP_GPIO_NOR_D5);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D6,  g_AM_BSP_GPIO_NOR_D6);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D7,  g_AM_BSP_GPIO_NOR_D7);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, g_AM_BSP_GPIO_NOR_SCK);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_1_8:
            case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8:
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, g_AM_BSP_GPIO_NOR_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  g_AM_BSP_GPIO_NOR_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  g_AM_BSP_GPIO_NOR_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  g_AM_BSP_GPIO_NOR_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  g_AM_BSP_GPIO_NOR_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D4,  g_AM_BSP_GPIO_NOR_D4);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D5,  g_AM_BSP_GPIO_NOR_D5);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D6,  g_AM_BSP_GPIO_NOR_D6);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D7,  g_AM_BSP_GPIO_NOR_D7);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, g_AM_BSP_GPIO_NOR_SCK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_DQS, g_AM_BSP_GPIO_NOR_DQS);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                break;
            default:
                break;
        }
        break;
#endif /* AM_BSP_GPIO_MSPI1_SCK */
    case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
        switch ( eMSPIDevice )
        {
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                break;
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
                break;
            default:
                break;
        }
        break;
#endif /* AM_BSP_GPIO_MSPI2_SCK */
    default:
        break;
    }
} // am_bsp_mspi_pins_enable()

//*****************************************************************************
//
//! @brief Disable the MSPI pins based on the external flash device type.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_PSRAM_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS0DM0,  am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS0DM0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_PSRAM_DQS1DM1, am_hal_gpio_pincfg_disabled);
                    am_bsp_mspi1_mux_disable();
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
    case 1:
        am_bsp_mspi1_mux_disable();
#if (AM_BSP_GPIO_MSPI1_SCK)
        switch ( eMSPIDevice )
        {
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE1, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_1_8:
            case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8:
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_DQS, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_CE1, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_SCK, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_NOR_DQS, am_hal_gpio_pincfg_disabled);
                break;
            default:
                break;
        }
        break;
#endif /* AM_BSP_GPIO_MSPI1_SCK*/
    case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
        switch ( eMSPIDevice )
        {
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, am_hal_gpio_pincfg_disabled);
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, am_hal_gpio_pincfg_disabled);
                break;
            default:
                break;
        }
        break;
#endif /* AM_BSP_GPIO_MSPI2_SCK*/
    default:
        break;
    }
} // am_bsp_mspi_pins_disable()

//*****************************************************************************
//
//! @brief MSPI CE Pin Config Get
//!
//! This function returns the pinnum and pincfg for the CE of MSPI requested.
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
        case 0:
#if (AM_BSP_GPIO_PSRAM_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    *pPinnum = AM_BSP_GPIO_PSRAM_CE0;
                    *pPincfg = g_AM_BSP_GPIO_PSRAM_CE0;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_NOR_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    *pPinnum = AM_BSP_GPIO_NOR_CE0;
                    *pPincfg = g_AM_BSP_GPIO_NOR_CE0;
                    break;
                default:
                    break;
            }
            break;
#endif /* AM_BSP_GPIO_MSPI1_SCK*/
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
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
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    *pPinnum = AM_BSP_GPIO_MSPI2_CE1;
                    *pPincfg = g_AM_BSP_GPIO_MSPI2_CE1;
                    break;
                default:
                    break;
            }
            break;
#endif /* AM_BSP_GPIO_MSPI2_SCK*/
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
                    // *pPinnum = AM_BSP_GPIO_MSPI0_RST;
                    // *pPincfg = g_AM_BSP_GPIO_MSPI0_RST;
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
#if (AM_BSP_GPIO_NOR_SCK)
            switch (eMSPIDevice)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_1_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1_1_1_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1_1_8_8:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    *pPinnum = AM_BSP_GPIO_NOR_RST;
                    *pPincfg = g_AM_BSP_GPIO_NOR_RST;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_NOR_SCK */
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
                    // *pPinnum = AM_BSP_GPIO_MSPI2_RST;
                    // *pPincfg = g_AM_BSP_GPIO_MSPI2_RST;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// configure or deconfig adc pins
//
//*****************************************************************************
uint32_t
am_bsp_adc_pin_config( am_bsp_adp_pins_e tADCPin, bool bPinADCModeEnable )
{
    am_hal_gpio_pincfg_t tPinCfg;

    uint32_t ui32PinNUm = (uint32_t) tADCPin;
    switch (ui32PinNUm)
    {
#ifdef AM_BSP_GPIO_ADCSE0
        case AM_BSP_GPIO_ADCSE0:
            tPinCfg = g_AM_BSP_GPIO_ADCSE0;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE1
        case AM_BSP_GPIO_ADCSE1:
            tPinCfg = g_AM_BSP_GPIO_ADCSE1;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE2
        case AM_BSP_GPIO_ADCSE2:
            tPinCfg = g_AM_BSP_GPIO_ADCSE2;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE3
        case AM_BSP_GPIO_ADCSE3:
            tPinCfg = g_AM_BSP_GPIO_ADCSE3;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE4
        case AM_BSP_GPIO_ADCSE4:
            tPinCfg = g_AM_BSP_GPIO_ADCSE4;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE5
        case AM_BSP_GPIO_ADCSE5:
            tPinCfg = g_AM_BSP_GPIO_ADCSE5;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE6
        case AM_BSP_GPIO_ADCSE6:
            tPinCfg = g_AM_BSP_GPIO_ADCSE6;
            break;
#endif
#ifdef AM_BSP_GPIO_ADCSE7
        case AM_BSP_GPIO_ADCSE7:
            tPinCfg = g_AM_BSP_GPIO_ADCSE7;
            break;
#endif
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    } // switch

    //
    // pin name is valid, and at this point in the code the
    // pin is configurd for ADC. Now look at enable flag in case
    // the user actually wants to disable the adc pin.
    //
    if (!bPinADCModeEnable)
    {
        tPinCfg.GP.cfg = 0;
    }

    return am_hal_gpio_pinconfig(ui32PinNUm, tPinCfg);
}


