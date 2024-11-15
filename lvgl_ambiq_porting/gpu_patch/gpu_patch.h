//*****************************************************************************
//
//! @file gpu_patch.h
//!
//! @brief Some helper functions to access the private data strutuers of private 
//!        functions of NemaSDK.
//!
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

#ifndef GPU_PATCH_H
#define GPU_PATCH_H

#include "nema_graphics.h"
#include "nema_interpolators.h"
#include "nema_graphics.h"
#include "nema_blender.h"
#include "nema_programHW.h"
#include "nema_vg_p.h"
#include "nema_vg_path.h"
#include "nema_vg_paint.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief Helper function to convert LVGL gradient descriptions to GPU commands.
//!
//! @param stops_count number of stops .
//! @param stops stops value.
//! @param colors colors in these stops
//! @param gradient_buffer buffer to hold the gradient.
//!
//!
//! @return no return.
//
//*****************************************************************************
extern void lv_ambiq_gradient_create(int stops_count,float *stops, color_var_t* colors, 
                                     nema_buffer_t* gradient_buffer);

//*****************************************************************************
//
//! @brief Helper function to got the paint texture object.
//!
//! @param vg_paint paint to read.
//! @param img_obj image object will write to this address.
//! @param palette_obj palette object will write to this address if LUT texture is used.
//!
//!
//! @return no return.
//
//*****************************************************************************
extern void lv_ambiq_get_vg_paint_tex(NEMA_VG_PAINT_HANDLE vg_paint, nema_img_obj_t ** img_obj, 
                                      nema_img_obj_t ** palette_obj);


#ifdef __cplusplus
}
#endif

#endif // GPU_PATCH_H