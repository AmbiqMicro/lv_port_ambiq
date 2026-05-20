//*****************************************************************************
//
// @file am_debug_pin.h
//
// @brief Unified header for GPIO performance profiling debug pins.
//
//*****************************************************************************

#ifndef AM_DEBUG_PIN_H
#define AM_DEBUG_PIN_H

#ifdef USE_DEBUG_PIN

#include "am_mcu_apollo.h"

// Define original debug pin mapping
#define DEBUG_PIN_1    46
#define DEBUG_PIN_2    47
#define DEBUG_PIN_3    48
#define DEBUG_PIN_4    49
#define DEBUG_PIN_5    60
#define DEBUG_PIN_6    61

// Encapsulated GPIO operations
#define AM_DEBUG_PIN_SET(pin)       am_hal_gpio_output_set(pin)
#define AM_DEBUG_PIN_CLEAR(pin)     am_hal_gpio_output_clear(pin)
#define AM_DEBUG_PIN_TOGGLE(pin)    am_hal_gpio_output_toggle(pin)

// Initialize all configured debug pins as output
static inline void am_debug_pin_init(void)
{
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_3, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_4, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_5, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_6, am_hal_gpio_pincfg_output);
}

#else

// Define empty aliases to avoid compilation overhead when profiling is disabled
#define DEBUG_PIN_1    0
#define DEBUG_PIN_2    0
#define DEBUG_PIN_3    0
#define DEBUG_PIN_4    0
#define DEBUG_PIN_5    0
#define DEBUG_PIN_6    0

#define AM_DEBUG_PIN_SET(pin)       ((void)0)
#define AM_DEBUG_PIN_CLEAR(pin)     ((void)0)
#define AM_DEBUG_PIN_TOGGLE(pin)    ((void)0)

static inline void am_debug_pin_init(void) {}

#endif

#endif // AM_DEBUG_PIN_H
