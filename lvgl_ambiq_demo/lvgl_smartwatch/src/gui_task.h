//*****************************************************************************
//
//! @file gui_task.h
//!
//! @brief Functions and variables related to the display task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

//*****************************************************************************
//
// Debug pin.
//
//*****************************************************************************
#define DEBUG_PIN_1    86
#define DEBUG_PIN_2    87
#define DEBUG_PIN_3    80
#define DEBUG_PIN_4    81
#define DEBUG_PIN_5    82
#define DEBUG_PIN_6    83

// Dispplay
#define DISPLAY_TOUCH_RESX (468U)
#define DISPLAY_TOUCH_RESY (468U)

//Frame buffer size
#ifndef LV_AMBIQ_DISPLAY_BUFFER_RESX
    #define LV_AMBIQ_DISPLAY_BUFFER_RESX (392U)
#endif

#ifndef LV_AMBIQ_DISPLAY_BUFFER_RESY
    #define LV_AMBIQ_DISPLAY_BUFFER_RESY (392U)
#endif

//*****************************************************************************
//
// Display task handle.
//
//*****************************************************************************
extern TaskHandle_t DisplayTaskHandle;

//*****************************************************************************
//
// Display task handle.
//
//*****************************************************************************
extern SemaphoreHandle_t lvgl_mutex;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void DisplayTask(void *pvParameters);

#endif //DISPLAY_TASK_H
