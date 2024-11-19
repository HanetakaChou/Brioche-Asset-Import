//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "internal_import_png_image.h"
#include "internal_import_image.h"
#include <cstring>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include "../thirdparty/libpng/png.h"

static void PNGCBAPI _internal_libpng_error_callback(png_structp png_ptr, png_const_charp error_message);

static png_voidp PNGCBAPI _internal_libpng_malloc_callback(png_structp png_ptr, png_alloc_size_t size);

static void PNGCBAPI _internal_libpng_free_ptr(png_structp png_ptr, png_voidp ptr);

struct _internal_libpng_read_data_context
{
    void const *m_data_base;
    size_t m_data_size;
    size_t m_offset;
};

static void PNGCBAPI _internal_libpng_read_data_callback(png_structp png_ptr, png_bytep data, size_t length);

extern bool internal_import_png_image(void const *data_base, size_t data_size, mcrt_vector<uint32_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
    // http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
    // https://github.com/pnggroup/libpng/blob/libpng16/libpng-manual.txt
    // https://chromium.googlesource.com/webm/libwebp/+/refs/heads/main/imageio/pngdec.c
    // https://github.com/microsoft/DirectXTex/blob/main/Auxiliary/DirectXTexPNG.cpp
    // https://github.com/mitsuba-renderer/mitsuba3/blob/master/src/core/bitmap.cpp

    png_structp png_ptr = NULL;
    png_infop header_info_ptr = NULL;
    bool has_error = false;
    try
    {
        png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, _internal_libpng_error_callback, NULL, NULL, _internal_libpng_malloc_callback, _internal_libpng_free_ptr);

        if ((png_get_chunk_malloc_max(png_ptr) < data_size) && (data_size < (static_cast<uint32_t>(1U) << static_cast<uint32_t>(24U))))
        {
            png_set_chunk_malloc_max(png_ptr, data_size);
        }

        _internal_libpng_read_data_context read_data_context = {data_base, data_size, 0};

        png_set_read_fn(png_ptr, &read_data_context, _internal_libpng_read_data_callback);

        header_info_ptr = png_create_info_struct(png_ptr);
        png_read_info(png_ptr, header_info_ptr);

        png_uint_32 width;
        png_uint_32 height;
        int bit_depth;
        int color_type;
        int interlaced;

        if (!png_get_IHDR(png_ptr, header_info_ptr, &width, &height, &bit_depth, &color_type, &interlaced, NULL, NULL))
        {
            throw std::runtime_error("png get IHDR");
        }

        if (bit_depth > 8)
        {
            assert(16 == bit_depth);

            png_set_scale_16(png_ptr);
        }

        // https://github.com/pnggroup/libpng/blob/libpng16/libpng-manual.txt
        if (PNG_COLOR_TYPE_GRAY == color_type)
        {
            // 01 -> 6A: CA
            // 0  -> 6A: CA
            // 0T -> 6A: C
            // 0O -> 6A: C

            png_set_gray_to_rgb(png_ptr);

            if (!png_get_valid(png_ptr, header_info_ptr, PNG_INFO_tRNS))
            {
                png_set_add_alpha(png_ptr, 0XFFFFU, PNG_FILLER_AFTER);
            }
        }
        else if (PNG_COLOR_TYPE_GRAY_ALPHA == color_type)
        {
            // 4A -> 6A: C
            // 4O -> 6O: C

            png_set_gray_to_rgb(png_ptr);
        }
        else if (PNG_COLOR_TYPE_PALETTE == color_type)
        {
            // 31 -> 6A: PA
            // 3  -> 6A: PA
            // 3T -> 6A: P
            // 3O -> 6A: P

            png_set_expand(png_ptr);
            png_set_palette_to_rgb(png_ptr);

            if (!png_get_valid(png_ptr, header_info_ptr, PNG_INFO_tRNS))
            {
                png_set_add_alpha(png_ptr, 0XFFFFU, PNG_FILLER_AFTER);
            }
        }
        else if (PNG_COLOR_TYPE_RGB == color_type)
        {
            // 2  -> 6A: A
            // 2T -> 6A: T
            // 2O -> 6O: T

            if (!png_get_valid(png_ptr, header_info_ptr, PNG_INFO_tRNS))
            {
                png_set_add_alpha(png_ptr, 0XFFFFU, PNG_FILLER_AFTER);
            }
            else
            {
                png_set_tRNS_to_alpha(png_ptr);
            }
        }

        {
            double file_gamma = 1 / 2.2;
            double screen_gamma = 2.2;
            if (png_get_gAMA(png_ptr, header_info_ptr, &file_gamma))
            {
                png_set_gamma(png_ptr, screen_gamma, file_gamma);
            }
            else
            {
                png_set_gamma(png_ptr, screen_gamma, file_gamma);
            }
        }

        int const num_passes = png_set_interlace_handling(png_ptr);

        // perform all transforms
        png_read_update_info(png_ptr, header_info_ptr);

        if (!png_get_IHDR(png_ptr, header_info_ptr, &width, &height, &bit_depth, &color_type, &interlaced, NULL, NULL))
        {
            throw std::runtime_error("png_get_IHDR");
        }

        if ((width > k_max_image_width_or_height) || (height > k_max_image_width_or_height))
        {
            throw std::runtime_error("Size Overflow");
        }

        if ((width < 1U) || (height < 1U))
        {
            throw std::runtime_error("Size Zero");
        }

        if (!((PNG_COLOR_TYPE_RGB_ALPHA == color_type) && (k_albedo_image_num_channels == png_get_channels(png_ptr, header_info_ptr)) && ((8 * k_albedo_image_channel_size) == bit_depth)))
        {
            throw std::runtime_error("NOT RGBA8 Format");
        }

        uint64_t const _uint64_stride = static_cast<uint64_t>(k_albedo_image_channel_size) * static_cast<uint64_t>(k_albedo_image_num_channels) * static_cast<uint64_t>(width);
        uint64_t const _uint64_num_pixels = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);

        uintptr_t const stride = static_cast<uintptr_t>(_uint64_stride);
        size_t const num_pixels = static_cast<size_t>(_uint64_num_pixels);

        if (!((stride == _uint64_stride) && (num_pixels == _uint64_num_pixels)))
        {
            throw std::runtime_error("Size Arguments Overflow");
        }

        out_pixel_data.resize(num_pixels);

        for (int pass_index = 0; pass_index < num_passes; ++pass_index)
        {
            for (png_uint_32 height_index = 0U; height_index < height; ++height_index)
            {
                png_bytep row = reinterpret_cast<png_bytep>(reinterpret_cast<uintptr_t>(out_pixel_data.data()) + stride * height_index);
                png_read_rows(png_ptr, &row, NULL, 1);
            }
        }

        // we only need the header info
        // we do NOT need the end info
        // png_read_end(st, end_info);

        (*out_width) = width;
        (*out_height) = height;
    }
    catch (std::runtime_error exception)
    {
        std::cout << exception.what() << std::endl;

        has_error = true;
    }

    png_destroy_info_struct(png_ptr, &header_info_ptr);

    png_destroy_read_struct(&png_ptr, &header_info_ptr, NULL);

    return (!has_error);
}

static void PNGCBAPI _internal_libpng_error_callback(png_structp, png_const_charp error_message)
{
    // TODO: is it safe to throw exception crossing the boundary of different DLLs?
    // compile libpng into static library?
    throw std::runtime_error(error_message);
}

static png_voidp PNGCBAPI _internal_libpng_malloc_callback(png_structp, png_alloc_size_t size)
{
    return mcrt_malloc(size, 16U);
}

static void PNGCBAPI _internal_libpng_free_ptr(png_structp, png_voidp ptr)
{
    return mcrt_free(ptr);
}

static void PNGCBAPI _internal_libpng_read_data_callback(png_structp png_ptr, png_bytep data, size_t length)
{
    // pngrio.c: png_default_read_data

    if (png_ptr != NULL)
    {
        _internal_libpng_read_data_context *const read_data_context = static_cast<_internal_libpng_read_data_context *>(png_get_io_ptr(png_ptr));

        if ((read_data_context->m_offset + length) <= read_data_context->m_data_size)
        {
            std::memcpy(data, reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(read_data_context->m_data_base) + read_data_context->m_offset), length);
            read_data_context->m_offset += length;
        }
        else
        {
            throw std::runtime_error("Read Data Overflow");
        }
    }
}
