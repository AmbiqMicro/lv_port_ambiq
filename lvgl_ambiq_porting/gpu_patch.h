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

/**
 * @brief Applies a two-pass separable blur (horizontal then vertical) to the corner shadow region.
 *
 * This function performs a box blur on a square region of shadow data stored in texidx1.
 * It uses texidx2 as a temporary buffer with extended borders to facilitate boundary-safe calculations.
 *
 * @param size     The size of the original square region (size x size) to be blurred.
 * @param sw       The blur strength factor. Each pixel will be averaged with its surrounding values within a ±sw range.
 * @param texidx1  Texture ID which bind to the buffer containing the original corner shadow data.
 *                 The final blurred result will also be written back into this buffer.
 * @param texidx2  Texture ID which bind to a temporary buffer with size (size + sw) x (size + sw).
 *                 This buffer is used to store horizontally blurred intermediate data and to extend borders
 *                 for safe vertical blur computation without special casing edges.
 */
void lv_ambiq_shadow_blur_corner(int32_t size, int32_t sw, nema_tex_t texidx1, nema_tex_t texidx2);

/**
 * @brief Retrieve the segment list and vertex data buffers of a VG path.
 *
 * This function extracts the number of path segments, the size of the vertex buffer,
 * and returns pointers to the internal segment list and vertex coordinate buffers
 * associated with the specified vector graphics path handle. The segment list defines
 * the sequence of VG commands, and the data buffer contains the corresponding coordinates.
 *
 * @param vg_path   The handle to the VG path object.
 * @param seg_size  Pointer to a variable where the number of path segments will be stored.
 * @param data_size Pointer to a variable where the number of floats in the vertex buffer will be stored.
 * @param seg       Pointer to a pointer where the segment list buffer address will be stored.
 * @param data      Pointer to a pointer where the vertex coordinate buffer address will be stored.
 *
 * @warning The function assumes all pointer parameters are valid and non-NULL. Undefined behavior may occur otherwise.
 */
void lv_ambiq_get_path_vbuf(NEMA_VG_PATH_HANDLE vg_path, uint32_t* seg_size, uint32_t* data_size, 
                                                         uint8_t** seg, float** data);


typedef struct {
    /** Pointer to the raw bitmap data of the glyph. This data is typically alpha-only. */
    const void* bitmap;

    /** The width of the source bitmap in pixels. */
    uint32_t bitmap_w;

    /** The height of the source bitmap in pixels. */
    uint32_t bitmap_h; 
    
    /** The X coordinate of the top-left corner where the glyph should be rendered on the destination. */
    int32_t raster_start_x;

    /** The Y coordinate of the top-left corner where the glyph should be rendered on the destination. */
    int32_t raster_start_y;

    /** The width of the destination rendering area in pixels. This may differ from bitmap_w due to clipping. */
    uint32_t raster_w;

    /** The height of the destination rendering area in pixels. This may differ from bitmap_h due to clipping. */
    uint32_t raster_h;

    /** The pixel format of the source bitmap (e.g., NEMA_A1, NEMA_A4), which defines the bits-per-pixel. */
    nema_tex_format_t nema_format;

    /** The color to be modulated with the bitmap's alpha values to render the glyph. */
    uint32_t color;

    /** Stride of the glyph. */
    uint32_t stride;

    /** The rotation angle for the glyph, often in high-precision units (e.g., 1/10th of a degree). */
    int32_t rotate_angle;

    /** The X coordinate of the rotation pivot point, relative to the glyph's bitmap. */
    int32_t pivot_x;

    /** The Y coordinate of the rotation pivot point, relative to the glyph's bitmap. */
    int32_t pivot_y;

    /** An output flag set to true if the temporary buffer was used. */
    bool temp_buffer_used;

    /** Pointer to a temporary buffer used for complex rendering scenarios. */
    void* temp_buffer;

    /** The width of the temporary buffer in pixels. */
    uint32_t temp_buffer_w;

    /** The height of the temporary buffer in pixels. */
    uint32_t temp_buffer_h;

    /** The color format of the temporary buffer (e.g., NEMA_A8). */
    nema_tex_format_t temp_buffer_cf;

    /** The stride of the temporary buffer in bytes (bytes per row). */
    uint32_t temp_buffer_stride;
} lv_ambiq_draw_bitmap_glyph_t;

/**
 * @brief Draws a bitmap glyph, handling various optimizations and fallback strategies.
 *
 * This function renders a bitmap, typically representing a font character (glyph). It employs several
 * conditional paths to optimize rendering based on the bitmap's properties, such as memory alignment,
 * size, and rotation. If the bitmap does not meet the criteria for a direct, single-pass draw,
 * it utilizes a more complex, iterative approach. This iterative method may use a temporary buffer 
 * to composite the final image, which is then blended and transformed to the destination.
 *
 * @param bitmap_glyph A pointer to a struct containing all parameters for the glyph to be drawn.
 */
void lv_ambiq_draw_bitmap_glyph(lv_ambiq_draw_bitmap_glyph_t *bitmap_glyph );

/**
 * @brief Renders a rounded-corner shadow using a radial gradient with vector graphics.
 *
 * This function draws a quarter-circle corner shadow using VG.
 * The shadow has a smooth radial gradient, the gradient smoothly fades within the sw range.
 *
 * @param size     The size of the corner region (width and height in pixels).
 * @param sw       Blur extent.
 * @param sh_buf   Destination buffer where the VG-rendered corner shadow will be stored.
 * @param paint    VG paint handle.
 * @param grad     VG gradient handle.
 */
void lv_ambiq_shadow_blur_corner_vg(float size, float sw, uint32_t* sh_buf, NEMA_VG_PAINT_HANDLE paint, NEMA_VG_GRAD_HANDLE grad);

/**
 * @brief Create a rounded corner mask for shadow rendering.
 *
 * This function generates a circular (rounded) corner mask used for
 * rendering smooth shadows. The generated mask is written into the 
 * provided buffer.
 *
 * @param size    The radius of the rounded corner.
 * @param sw      The total shadow width.
 * @param sh_buf  Pointer to the buffer where the generated mask will be stored.
 */
void lv_ambiq_create_corner_mask(uint32_t size, uint32_t sw, uint32_t* sh_buf);

/**
 * @brief Convert an L8 format image to L4 format using GPU acceleration.
 *
 * This function performs a hardware-accelerated conversion from L8 
 * format to L4 format using the NemaGFX raster engine. The input L8 image must have
 * an even width due to the 4-bit per pixel packing requirement.
 *
 * @param l8_ptr   Pointer to the input buffer containing L8 image data.
 * @param l4_ptr   Pointer to the output buffer to store the converted L4 image data.
 * @param width    Width of the input image (must be even).
 * @param height   Height of the input image.
 *
 * @return Returns 0 on successful execution. If the width is not even, returns 0 and performs no operation.
 *
 * @note This function assumes that `l8_ptr` and `l4_ptr` are valid non-null pointers,
 *       and that both the L8 input image and L4 output image share the same pixel dimensions
 *       (i.e., width × height pixels). The `l8_ptr` buffer must be at least (width × height) bytes,
 *       as each pixel is stored with 8-bit. The `l4_ptr` buffer must be at least
 *       ((width × height) / 2) bytes, since two 4-bit alpha values are packed into each byte (L4).
 *       No memory allocation or bounds checking is performed within the function.
 */
uint32_t lv_ambiq_l8_l4_convert(void* l8_ptr, void* l4_ptr, uint32_t width, uint32_t height);
#ifdef __cplusplus
}
#endif

#endif // GPU_PATCH_H