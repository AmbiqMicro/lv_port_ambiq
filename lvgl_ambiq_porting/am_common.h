//*****************************************************************************
//
//! @file am_common.h
//!
//! @brief Common initialization.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_COMMON_H
#define AM_COMMON_H

//*****************************************************************************
//
// PSRAM address.
//
//*****************************************************************************
#define MSPI_PSRAM_MODULE               0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

#define MSPI_PSRAM_SIZE                 (0x800000)

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_gpu_init(void);
extern void am_mspi_init(void);
extern void am_relocate_init_data_to_psram(void);

#endif //AM_COMMON_H
