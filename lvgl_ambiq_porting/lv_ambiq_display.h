//*****************************************************************************
//
//! @file lv_ambiq_diaplay.h
//!
//! @brief Functions and variables related to the display.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef LV_AMBIQ_DISPLAY_H
#define LV_AMBIQ_DISPLAY_H

//*****************************************************************************
//
// Frame buffer size
//
//*****************************************************************************
#ifndef LV_AMBIQ_DISPLAY_BUFFER_RESX
    #define LV_AMBIQ_DISPLAY_BUFFER_RESX (468U)
#endif

#ifndef LV_AMBIQ_DISPLAY_BUFFER_RESY
    #define LV_AMBIQ_DISPLAY_BUFFER_RESY (468U)
#endif



//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void lv_ambiq_display_init(void);

#endif //LV_AMBIQ_DISPLAY_H
