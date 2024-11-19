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

#include "internal_import_tga_image.h"
#include "internal_import_image.h"
#include <cstring>
#include <cassert>
#include <algorithm>

#if defined(__GNUC__)
// GCC or CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
// MSVC or CLANG-CL
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#else
#error Unknown Compiler
#endif

#if defined(__GNUC__)
// GCC or CLANG
#define internal_likely(x) __builtin_expect(!!(x), 1)
#define internal_unlikely(x) __builtin_expect(!!(x), 0)
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __ORDER_LITTLE_ENDIAN__ == __BYTE_ORDER__
static inline uint16_t internal_bswap_16(uint16_t x) { return x; }
static inline uint32_t internal_bswap_32(uint32_t x) { return x; }
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __ORDER_BIG_ENDIAN__ == __BYTE_ORDER__
static inline uint16_t internal_bswap_16(uint16_t x) { return __builtin_bswap16(x); }
static inline uint32_t internal_bswap_32(uint32_t x) { return __builtin_bswap32(x); }
#else
#error Unknown Byte Order
#endif
#elif defined(_MSC_VER)
static inline uint16_t internal_bswap_16(uint16_t x) { return x; }
static inline uint32_t internal_bswap_32(uint32_t x) { return x; }
#if defined(__clang__)
// CLANG-CL
#define internal_likely(x) __builtin_expect(!!(x), 1)
#define internal_unlikely(x) __builtin_expect(!!(x), 0)
#else
// MSVC
#define internal_likely(x) (!!(x))
#define internal_unlikely(x) (!!(x))
#endif
#else
#error Unknown Compiler
#endif

static inline bool internal_data_read_tga_header(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t *out_image_type, uint32_t *out_image_width, uint32_t *out_image_height, uint32_t *out_image_pixel_depth, bool *out_flip_x, bool *out_flip_y);

static inline bool internal_data_read_tga_uncompressed_rgba_image_data(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t image_width, uint32_t image_height, uint32_t image_pixel_depth, bool flip_x, bool flip_y, uint32_t *out_pixel_data);

static inline bool internal_data_read_uint32(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t *out_uint32);

static inline bool internal_data_read_uint16(void const *data_base, size_t data_size, size_t &inout_data_offset, uint16_t *out_uint16);

static inline bool internal_data_read_uint8(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t *out_uint8);

static inline bool internal_data_read_bytes(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t length, void *out_bytes);

extern bool internal_import_tga_image(void const *data_base, size_t data_size, mcrt_vector<uint32_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
    size_t data_offset = 0U;

    uint32_t image_type;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t image_pixel_depth;
    bool flip_x;
    bool flip_y;
    if (internal_unlikely(!internal_data_read_tga_header(data_base, data_size, data_offset, &image_type, &image_width, &image_height, &image_pixel_depth, &flip_x, &flip_y)))
    {
        return false;
    }

    if (internal_unlikely((image_width > k_max_image_width_or_height) || (image_height > k_max_image_width_or_height)))
    {
        return false;
    }

    if (internal_unlikely((image_width < 1U) || (image_height < 1U)))
    {
        return false;
    }

    {
        uint64_t const _uint64_num_pixels = static_cast<uint64_t>(image_width) * static_cast<uint64_t>(image_height);
        size_t const num_pixels = static_cast<size_t>(_uint64_num_pixels);

        if (internal_unlikely(!(num_pixels == _uint64_num_pixels)))
        {
            return false;
        }

        out_pixel_data.resize(num_pixels, static_cast<uint64_t>(0U));
    }

    if (1U == image_type)
    {
        // TODO: uncompressed color mapped image
        return false;
    }
    else if ((2U == image_type) || (3U == image_type))
    {
        // uncompressed true color image
        // uncompressed black and white image
        if (internal_unlikely(!internal_data_read_tga_uncompressed_rgba_image_data(data_base, data_size, data_offset, image_width, image_height, image_pixel_depth, flip_x, flip_y, out_pixel_data.data())))
        {
            return false;
        }
    }
    else if (9U == image_type)
    {
        // TODO: runlength encoded color mapped image
        return false;
    }
    else if ((10U == image_type) || (11U == image_type))
    {
        // TODO: runlength encoded true color image
        // TODO: runlength encoded black and white image
        return false;
    }

    (*out_width) = image_width;
    (*out_height) = image_height;

    return true;
}

static inline bool internal_data_read_tga_header(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t *out_image_type, uint32_t *out_image_width, uint32_t *out_image_height, uint32_t *out_image_pixel_depth, bool *out_flip_x, bool *out_flip_y)
{
    uint8_t id_length;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &id_length)))
    {
        return false;
    }

    uint8_t color_map_type;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &color_map_type)))
    {
        return false;
    }

    uint8_t image_type;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &image_type)))
    {
        return false;
    }
    (*out_image_type) = image_type;

    uint16_t first_entry_index;
    if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &first_entry_index)))
    {
        return false;
    }

    uint16_t color_map_length;
    if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &color_map_length)))
    {
        return false;
    }

    uint8_t color_map_entry_size;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &color_map_entry_size)))
    {
        return false;
    }

    uint16_t image_x_origin;
    if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &image_x_origin)))
    {
        return false;
    }

    uint16_t image_y_origin;
    if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &image_y_origin)))
    {
        return false;
    }

    uint16_t image_width;
    if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &image_width)))
    {
        return false;
    }
    (*out_image_width) = image_width;

    uint16_t image_height;
    if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &image_height)))
    {
        return false;
    }
    (*out_image_height) = image_height;

    uint8_t image_pixel_depth;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &image_pixel_depth)))
    {
        return false;
    }
    (*out_image_pixel_depth) = image_pixel_depth;

    uint8_t image_descriptor;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &image_descriptor)))
    {
        return false;
    }

    assert((16U != (*out_image_pixel_depth)) || ((0U == (image_descriptor & 0XFU)) || (1U == (image_descriptor & 0XFU))));
    assert((24U != (*out_image_pixel_depth)) || (0U == (image_descriptor & 0XFU)));
    assert((32U != (*out_image_pixel_depth)) || (8U == (image_descriptor & 0XFU)));

    (*out_flip_x) = (0U != (image_descriptor & 0X10U));
    (*out_flip_y) = (0U != (image_descriptor & 0X20U));

    mcrt_vector<uint8_t> image_id(static_cast<size_t>(id_length));
    if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, id_length, image_id.data())))
    {
        return false;
    }

    mcrt_vector<uint8_t> color_map_data;
    if (0U != color_map_type)
    {
        uint32_t const color_map_entry_byte_size = (color_map_entry_size >= 1U) ? (((color_map_entry_size - 1U) / 8U) + 1U) : 0U;
        uint32_t const color_map_data_size = color_map_entry_byte_size * color_map_length;
        color_map_data.resize(color_map_data_size);
        if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, color_map_data_size, color_map_data.data())))
        {
            return false;
        }
    }

    return true;
}

static inline bool internal_data_read_tga_uncompressed_rgba_image_data(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t image_width, uint32_t image_height, uint32_t image_pixel_depth, bool flip_x, bool flip_y, uint32_t *out_pixel_data)
{
    if (32U == image_pixel_depth)
    {
        for (uint32_t height_index = 0U; height_index < image_height; ++height_index)
        {
            for (uint32_t width_index = 0U; width_index < image_width; ++width_index)
            {
                uint32_t raw_pixel_data;
                if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &raw_pixel_data)))
                {
                    return false;
                }

                uint32_t pixel_data;
                {
                    DirectX::PackedVector::XMUBYTEN4 packed_pixel_data(raw_pixel_data);

                    uint8_t const blue = packed_pixel_data.x;
                    uint8_t const green = packed_pixel_data.y;
                    uint8_t const red = packed_pixel_data.z;
                    uint8_t const alpha = packed_pixel_data.w;

                    packed_pixel_data.x = red;
                    packed_pixel_data.y = green;
                    packed_pixel_data.z = blue;
                    packed_pixel_data.w = alpha;

                    pixel_data = packed_pixel_data.v;
                }

                uint32_t const out_height_index = (flip_y) ? height_index : (image_height - height_index - 1U);
                uint32_t const out_width_index = (!flip_x) ? width_index : (image_width - width_index - 1U);
                out_pixel_data[image_width * out_height_index + out_width_index] = pixel_data;
            }
        }
    }
    else if (24U == image_pixel_depth)
    {
        for (uint32_t height_index = 0U; height_index < image_height; ++height_index)
        {
            for (uint32_t width_index = 0U; width_index < image_width; ++width_index)
            {
                uint8_t raw_pixel_data[3];
                if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, sizeof(raw_pixel_data), raw_pixel_data)))
                {
                    return false;
                }

                uint32_t pixel_data;
                {
                    DirectX::PackedVector::XMUBYTEN4 packed_pixel_data;

                    uint8_t const blue = raw_pixel_data[0];
                    uint8_t const green = raw_pixel_data[1];
                    uint8_t const red = raw_pixel_data[2];
                    uint8_t const alpha = 255U;

                    packed_pixel_data.x = red;
                    packed_pixel_data.y = green;
                    packed_pixel_data.z = blue;
                    packed_pixel_data.w = alpha;

                    pixel_data = packed_pixel_data.v;
                }

                uint32_t const out_height_index = (flip_y) ? height_index : (image_height - height_index - 1U);
                uint32_t const out_width_index = (!flip_x) ? width_index : (image_width - width_index - 1U);
                out_pixel_data[image_width * out_height_index + out_width_index] = pixel_data;
            }
        }
    }
    else if (16U == image_pixel_depth)
    {
        for (uint32_t height_index = 0U; height_index < image_height; ++height_index)
        {
            for (uint32_t width_index = 0U; width_index < image_width; ++width_index)
            {
                uint16_t raw_pixel_data;
                if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &raw_pixel_data)))
                {
                    return false;
                }

                uint32_t pixel_data;
                {
                    DirectX::PackedVector::XMU565 packed_565_pixel_data(raw_pixel_data);

                    auto const blue = packed_565_pixel_data.x;
                    auto const green = packed_565_pixel_data.y;
                    auto const red = packed_565_pixel_data.z;
                    uint8_t const alpha = 255U;

                    DirectX::PackedVector::XMUBYTEN4 packed_8888_pixel_data;

                    packed_8888_pixel_data.x = static_cast<uint8_t>(std::min(std::max(0.0F, static_cast<float>(red) / 31.0F), 1.0F) * 255.0F);
                    packed_8888_pixel_data.y = static_cast<uint8_t>(std::min(std::max(0.0F, static_cast<float>(green) / 63.0F), 1.0F) * 255.0F);
                    packed_8888_pixel_data.z = static_cast<uint8_t>(std::min(std::max(0.0F, static_cast<float>(blue) / 31.0F), 1.0F) * 255.0F);
                    packed_8888_pixel_data.w = alpha;
                    pixel_data = packed_8888_pixel_data.v;
                }

                uint32_t const out_height_index = (flip_y) ? height_index : (image_height - height_index - 1U);
                uint32_t const out_width_index = (!flip_x) ? width_index : (image_width - width_index - 1U);
                out_pixel_data[image_width * out_height_index + out_width_index] = pixel_data;
            }
        }
    }
    else if (15U == image_pixel_depth)
    {
        for (uint32_t height_index = 0U; height_index < image_height; ++height_index)
        {
            for (uint32_t width_index = 0U; width_index < image_width; ++width_index)
            {
                uint16_t raw_pixel_data;
                if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &raw_pixel_data)))
                {
                    return false;
                }

                uint32_t pixel_data;
                {
                    DirectX::PackedVector::XMU555 packed_555_pixel_data(raw_pixel_data);

                    auto const blue = packed_555_pixel_data.x;
                    auto const green = packed_555_pixel_data.y;
                    auto const red = packed_555_pixel_data.z;
                    uint8_t const alpha = 255U;

                    DirectX::PackedVector::XMUBYTEN4 packed_8888_pixel_data;

                    packed_8888_pixel_data.x = static_cast<uint8_t>(std::min(std::max(0.0F, static_cast<float>(red) / 31.0F), 1.0F) * 255.0F);
                    packed_8888_pixel_data.y = static_cast<uint8_t>(std::min(std::max(0.0F, static_cast<float>(green) / 31.0F), 1.0F) * 255.0F);
                    packed_8888_pixel_data.z = static_cast<uint8_t>(std::min(std::max(0.0F, static_cast<float>(blue) / 31.0F), 1.0F) * 255.0F);
                    packed_8888_pixel_data.w = alpha;
                    pixel_data = packed_8888_pixel_data.v;
                }

                uint32_t const out_height_index = (flip_y) ? height_index : (image_height - height_index - 1U);
                uint32_t const out_width_index = (!flip_x) ? width_index : (image_width - width_index - 1U);
                out_pixel_data[image_width * out_height_index + out_width_index] = pixel_data;
            }
        }
    }
    else if (8U == image_pixel_depth)
    {
        for (uint32_t height_index = 0U; height_index < image_height; ++height_index)
        {
            for (uint32_t width_index = 0U; width_index < image_width; ++width_index)
            {
                uint8_t raw_pixel_data;
                if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &raw_pixel_data)))
                {
                    return false;
                }

                uint32_t pixel_data;
                {
                    uint8_t const blue = raw_pixel_data;
                    uint8_t const green = raw_pixel_data;
                    uint8_t const red = raw_pixel_data;
                    uint8_t const alpha = 255U;

                    DirectX::PackedVector::XMUBYTEN4 packed_pixel_data;

                    packed_pixel_data.x = red;
                    packed_pixel_data.y = green;
                    packed_pixel_data.z = blue;
                    packed_pixel_data.w = alpha;
                    pixel_data = packed_pixel_data.v;
                }

                uint32_t const out_height_index = (flip_y) ? height_index : (image_height - height_index - 1U);
                uint32_t const out_width_index = (!flip_x) ? width_index : (image_width - width_index - 1U);
                out_pixel_data[image_width * out_height_index + out_width_index] = pixel_data;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

static inline bool internal_data_read_uint32(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t *out_uint32)
{
    if (data_size >= (inout_data_offset + sizeof(uint32_t)))
    {
        (*out_uint32) = internal_bswap_32(*reinterpret_cast<uint32_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset));
        inout_data_offset += sizeof(uint32_t);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_uint16(void const *data_base, size_t data_size, size_t &inout_data_offset, uint16_t *out_uint16)
{
    if (data_size >= (inout_data_offset + sizeof(uint16_t)))
    {
        (*out_uint16) = internal_bswap_16(*reinterpret_cast<uint16_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset));
        inout_data_offset += sizeof(uint16_t);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_uint8(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t *out_uint8)
{
    if (data_size >= (inout_data_offset + sizeof(uint8_t)))
    {
        (*out_uint8) = (*reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset));
        inout_data_offset += sizeof(uint8_t);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_bytes(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t length, void *out_bytes)
{
    if (data_size >= (inout_data_offset + (sizeof(uint8_t) * length)))
    {
        std::memcpy(out_bytes, reinterpret_cast<void const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset), length);
        inout_data_offset += (sizeof(uint8_t) * length);
        return true;
    }
    else
    {
        return false;
    }
}
