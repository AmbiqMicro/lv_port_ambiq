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
#include "nema_vg_path.h"
#include "nema_vg_paint.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Creates a gradient texture using specified color stops and colors.
 *
 * This function generates a gradient texture by interpolating between specified
 * color stops and colors. It handles cases where stops are invalid or missing
 * by adding implicit stops and colors. The gradient is rendered into a texture
 * buffer and can be used for graphical rendering.
 *
 * @param stops_count The number of color stops provided.
 * @param stops An array of float values representing the positions of the color stops
 *              in the range [0.0, 1.0].
 * @param colors An array of color_var_t structures representing the colors at each stop.
 * @param texid The texture ID where the gradient will be rendered.
 *
 * @note The function performs a dry run to validate the stops and ensure correctness
 *       before rendering the gradient. It also ensures that the first and last stops
 *       are correctly handled.
 *
 * @warning The function assumes that the input arrays (stops and colors) are valid
 *          and properly allocated. Undefined behavior may occur if invalid pointers
 *          or sizes are provided.
 */
void lv_ambiq_gradient_create(int stops_count,float *stops, color_var_t* colors, nema_tex_t texid);

/**
 * @brief Creates a dashed line texture in the specified texture ID.
 *
 * This function generates a dashed line pattern in a texture buffer. The pattern
 * is defined by the dash width, dash gap, and the RGBA color. The resulting dashed
 * line is stored in the texture identified by the given texture ID.
 *
 * @param dash_width The width of each dash segment in pixels.
 * @param dash_gap The gap between consecutive dash segments in pixels.
 * @param rgba_color The RGBA color of the dash segments, represented as a 32-bit integer.
 * @param texid The texture ID where the dashed line pattern will be created.
 *
 * @note The function assumes that the texture buffer width is sufficient to hold
 *       the dashed line pattern. The texture is modified directly.
 */
void lv_ambiq_dashline_create(uint32_t dash_width, uint32_t dash_gap, uint32_t rgba_color, nema_tex_t texid);

/**
 * @brief Retrieve the texture and optional palette object from a VG paint handle.
 *
 * This function extracts the texture object and, if applicable, the palette object
 * associated with a given vector graphics (VG) paint handle.
 *
 * @param vg_paint      The handle to the VG paint object.
 * @param img_obj       Pointer to a pointer where the texture object will be stored.
 * @param palette_obj   Pointer to a pointer where the palette object will be stored.
 *                      If the texture is not a LUT (Look-Up Table) texture, this will
 *                      be set to NULL.
 */
void lv_ambiq_get_vg_paint_tex(NEMA_VG_PAINT_HANDLE vg_paint, nema_img_obj_t ** img_obj, nema_img_obj_t ** palette_obj);

/**
 * @brief Retrieve the axis-aligned bounding box (AABB) of a vector graphics path.
 *
 * This function extracts the minimum and maximum x and y coordinates of the 
 * axis-aligned bounding box (AABB) for the given vector graphics path.
 *
 * @param vg_path A handle to the vector graphics path (NEMA_VG_PATH_HANDLE).
 * @param x_min Pointer to a float where the minimum x-coordinate of the AABB will be stored.
 * @param y_min Pointer to a float where the minimum y-coordinate of the AABB will be stored.
 * @param x_max Pointer to a float where the maximum x-coordinate of the AABB will be stored.
 * @param y_max Pointer to a float where the maximum y-coordinate of the AABB will be stored.
 */
void lv_ambiq_get_path_aabb(NEMA_VG_PATH_HANDLE vg_path, float* x_min, float* y_min, float* x_max, float* y_max);




#ifdef __cplusplus
}
#endif

#endif // GPU_PATCH_H