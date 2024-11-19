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

#include "internal_import_image.h"
#include "internal_import_png_image.h"
#include "internal_import_jpeg_image.h"
#include "internal_import_webp_image.h"
#include <cassert>
#include <algorithm>

#define CV_IGNORE_DEBUG_BUILD_GUARD 1
#if defined(__GNUC__)
// GCC or CLANG
#include <opencv2/opencv.hpp>
#elif defined(_MSC_VER)
#if defined(__clang__)
// CLANG-CL
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-noreturn"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdeprecated-anon-enum-enum-conversion"
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#include <opencv2/opencv.hpp>
#pragma GCC diagnostic pop
#else
// MSVC
#include <opencv2/opencv.hpp>
#endif
#else
#error Unknown Compiler
#endif

static inline bool internal_is_power_of_2(uint32_t width_or_height);
static inline void internal_fit_power_of_2(uint32_t origin_width, uint32_t origin_height, uint32_t &target_width, uint32_t &target_height);
static inline uint32_t internal_count_mips(uint32_t width, uint32_t height);

extern BRX_IMAGE_TYPE internal_import_image_type(void const *data_base, size_t data_size)
{
    if (data_size >= 12U &&
        (82U == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
        (73U == reinterpret_cast<uint8_t const *>(data_base)[1]) &&
        (70U == reinterpret_cast<uint8_t const *>(data_base)[2]) &&
        (70U == reinterpret_cast<uint8_t const *>(data_base)[3]) &&
        (87U == reinterpret_cast<uint8_t const *>(data_base)[8]) &&
        (69U == reinterpret_cast<uint8_t const *>(data_base)[9]) &&
        (66U == reinterpret_cast<uint8_t const *>(data_base)[10]) &&
        (80U == reinterpret_cast<uint8_t const *>(data_base)[11]))
    {
        // https://chromium.googlesource.com/webm/libwebp/+/refs/heads/main/imageio/image_dec.c
        // webp_dec.c: ParseRIFF
        // 82 73 70 70 _ _ _ _ 87 69 66 80
        // R  I  F  F  _ _ _ _ W  E  B  P

        return BRX_IMAGE_TYPE_ALBEDO;
    }
    else if (data_size >= 8U &&
             (137U == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
             (80U == reinterpret_cast<uint8_t const *>(data_base)[1]) &&
             (78U == reinterpret_cast<uint8_t const *>(data_base)[2]) &&
             (71U == reinterpret_cast<uint8_t const *>(data_base)[3]) &&
             (13U == reinterpret_cast<uint8_t const *>(data_base)[4]) &&
             (10U == reinterpret_cast<uint8_t const *>(data_base)[5]) &&
             (26U == reinterpret_cast<uint8_t const *>(data_base)[6]) &&
             (10U == reinterpret_cast<uint8_t const *>(data_base)[7]))
    {
        // png.c: png_sig_cmp
        // 137 80 78 71 13 10 26  10
        // N/A P  N  G  CR LF EOF LF

        return BRX_IMAGE_TYPE_ALBEDO;
    }
    else if (data_size >= 2U &&
             (0XFFU == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
             (0XD8U == reinterpret_cast<uint8_t const *>(data_base)[1]))
    {
        // jdmarker.c: first_marker
        // 0XFF 0XD8
        // N/A  SOI

        return BRX_IMAGE_TYPE_ALBEDO;
    }
    else
    {
        return BRX_IMAGE_TYPE_UNKNOWN;
    }
}

extern bool internal_import_albedo_image(void const *data_base, size_t data_size, mcrt_vector<mcrt_vector<uint32_t>> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
    mcrt_vector<uint32_t> origin_pixel_data;
    uint32_t origin_width = 0;
    uint32_t origin_height = 0;
    bool status_internal_import_image;
    if (data_size >= 12U &&
        (82U == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
        (73U == reinterpret_cast<uint8_t const *>(data_base)[1]) &&
        (70U == reinterpret_cast<uint8_t const *>(data_base)[2]) &&
        (70U == reinterpret_cast<uint8_t const *>(data_base)[3]) &&
        (87U == reinterpret_cast<uint8_t const *>(data_base)[8]) &&
        (69U == reinterpret_cast<uint8_t const *>(data_base)[9]) &&
        (66U == reinterpret_cast<uint8_t const *>(data_base)[10]) &&
        (80U == reinterpret_cast<uint8_t const *>(data_base)[11]))
    {
        // https://chromium.googlesource.com/webm/libwebp/+/refs/heads/main/imageio/image_dec.c
        // webp_dec.c: ParseRIFF
        // 82 73 70 70 _ _ _ _ 87 69 66 80
        // R  I  F  F  _ _ _ _ W  E  B  P

        status_internal_import_image = internal_import_webp_image(data_base, data_size, origin_pixel_data, &origin_width, &origin_height);
    }
    else if (data_size >= 8U &&
             (137U == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
             (80U == reinterpret_cast<uint8_t const *>(data_base)[1]) &&
             (78U == reinterpret_cast<uint8_t const *>(data_base)[2]) &&
             (71U == reinterpret_cast<uint8_t const *>(data_base)[3]) &&
             (13U == reinterpret_cast<uint8_t const *>(data_base)[4]) &&
             (10U == reinterpret_cast<uint8_t const *>(data_base)[5]) &&
             (26U == reinterpret_cast<uint8_t const *>(data_base)[6]) &&
             (10U == reinterpret_cast<uint8_t const *>(data_base)[7]))
    {
        // png.c: png_sig_cmp
        // 137 80 78 71 13 10 26  10
        // N/A P  N  G  CR LF EOF LF

        status_internal_import_image = internal_import_png_image(data_base, data_size, origin_pixel_data, &origin_width, &origin_height);
    }
    else if (data_size >= 2U &&
             (0XFFU == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
             (0XD8U == reinterpret_cast<uint8_t const *>(data_base)[1]))
    {
        // jdmarker.c: first_marker
        // 0XFF 0XD8
        // N/A  SOI

        status_internal_import_image = internal_import_jpeg_image(data_base, data_size, origin_pixel_data, &origin_width, &origin_height);
    }
    else
    {
        status_internal_import_image = false;
    }

    if (!status_internal_import_image)
    {
        return false;
    }

    uint32_t width;
    uint32_t height;
    internal_fit_power_of_2(origin_width, origin_height, width, height);
    assert((width >= 1U) && (width <= k_max_image_width_or_height) && internal_is_power_of_2(width) && (height >= 1U) && (height <= k_max_image_width_or_height) && internal_is_power_of_2(height));

    uint32_t mip_level_count = internal_count_mips(width, height);
    assert((mip_level_count >= 1U) && (mip_level_count <= k_max_image_mip_levels));

    out_pixel_data.resize(mip_level_count);

    if ((origin_width == width) && (origin_height == height))
    {
        out_pixel_data[0] = std::move(origin_pixel_data);
        (*out_width) = origin_width;
        (*out_height) = origin_height;
    }
    else
    {
        // mediapipe/framework/formats/image_frame_opencv.cc
        int const dims = 2;
        int const origin_sizes[dims] = {static_cast<int>(origin_height), static_cast<int>(origin_width)};
        int const type = CV_MAKETYPE(CV_8U, 4);
        size_t const origin_stride = static_cast<size_t>(k_albedo_image_channel_size) * static_cast<size_t>(k_albedo_image_num_channels) * static_cast<uint64_t>(origin_width);
        size_t const origin_steps[dims] = {origin_stride, 8U};

        cv::Mat origin_image(dims, origin_sizes, type, reinterpret_cast<uint8_t *>(origin_pixel_data.data()), origin_steps);
        assert(origin_image.isContinuous());

        cv::Mat target_image;
        int const interpolation = ((width <= origin_width) && (height <= origin_height)) ? cv::INTER_AREA : cv::INTER_LANCZOS4;
        cv::resize(origin_image, target_image, cv::Size(static_cast<int>(width), static_cast<int>(height)), 0.0, 0.0, interpolation);

        size_t const target_stride = static_cast<size_t>(k_albedo_image_channel_size) * static_cast<size_t>(k_albedo_image_num_channels) * static_cast<uint64_t>(width);
        assert(target_image.type() == type);
        assert(target_image.step[0] == target_stride);
        assert(width == target_image.cols);
        assert(height == target_image.rows);
        assert(target_image.isContinuous());

        out_pixel_data[0].resize(static_cast<size_t>(width) * static_cast<size_t>(height));
        size_t const total_size = sizeof(uint32_t) * out_pixel_data[0].size();
        assert((target_image.step[0] * target_image.rows) == total_size);
        std::memcpy(out_pixel_data[0].data(), target_image.data, total_size);

        (*out_width) = width;
        (*out_height) = height;
    }

    for (uint32_t mip_level_index = 1U; mip_level_index < mip_level_count; ++mip_level_index)
    {
        int const dims = 2;
        int const source_sizes[dims] = {static_cast<int>(width), static_cast<int>(height)};
        int const type = CV_MAKETYPE(CV_8U, 4);
        size_t const source_stride = static_cast<size_t>(k_albedo_image_channel_size) * static_cast<size_t>(k_albedo_image_num_channels) * static_cast<uint64_t>(width);
        size_t const source_steps[dims] = {source_stride, 8U};
        cv::Mat source_image(dims, source_sizes, type, reinterpret_cast<uint8_t *>(out_pixel_data[mip_level_index - 1U].data()), source_steps);
        assert(source_image.isContinuous());

        width = std::max(1U, width >> 1U);
        height = std::max(1U, height >> 1U);

        cv::Mat destination_image;
        cv::resize(source_image, destination_image, cv::Size(static_cast<int>(width), static_cast<int>(height)), 0.0, 0.0, cv::INTER_AREA);

        size_t const destination_stride = static_cast<size_t>(k_albedo_image_channel_size) * static_cast<size_t>(k_albedo_image_num_channels) * static_cast<uint64_t>(width);
        assert(destination_image.type() == type);
        assert(destination_image.step[0] == destination_stride);
        assert(width == destination_image.cols);
        assert(height == destination_image.rows);
        assert(destination_image.isContinuous());

        out_pixel_data[mip_level_index].resize(static_cast<size_t>(width) * static_cast<size_t>(height));
        size_t const total_size = sizeof(uint32_t) * out_pixel_data[mip_level_index].size();
        assert((destination_image.step[0] * destination_image.rows) == total_size);
        std::memcpy(out_pixel_data[mip_level_index].data(), destination_image.data, total_size);
    }

    return true;
}

static inline bool internal_is_power_of_2(uint32_t width_or_height)
{
    assert(width_or_height >= 1U);
    return (0U == (width_or_height & (width_or_height - 1U)));
}

static inline void internal_fit_power_of_2(uint32_t origin_width, uint32_t origin_height, uint32_t &target_width, uint32_t &target_height)
{
    assert((origin_width >= 1U) && (origin_width <= k_max_image_width_or_height) && (origin_height >= 1U) && (origin_height <= k_max_image_width_or_height));

    // DirectXTex/texconv.cpp: FitPowerOf2

    if (internal_is_power_of_2(origin_width) && internal_is_power_of_2(origin_height))
    {
        target_width = origin_width;
        target_height = origin_height;
    }
    else
    {
        if (origin_width > origin_height)
        {
            if (internal_is_power_of_2(origin_width))
            {
                target_width = origin_width;
            }
            else
            {
                uint32_t w;
                for (w = k_max_image_width_or_height; w > 1; w >>= 1)
                {
                    if (w <= origin_width)
                    {
                        break;
                    }
                }
                target_width = w;
            }

            {
                float const origin_aspect_ratio = static_cast<float>(origin_width) / static_cast<float>(origin_height);
                float best_score = FLT_MAX;
                for (uint32_t h = k_max_image_width_or_height; h > 0; h >>= 1)
                {
                    float const score = std::abs((static_cast<float>(target_width) / static_cast<float>(h)) - origin_aspect_ratio);
                    if (score < best_score)
                    {
                        best_score = score;
                        target_height = h;
                    }
                }
            }
        }
        else
        {
            if (internal_is_power_of_2(origin_height))
            {
                target_height = origin_height;
            }
            else
            {
                uint32_t h;
                for (h = k_max_image_width_or_height; h > 1; h >>= 1)
                {
                    if (h <= origin_height)
                    {
                        break;
                    }
                }
                target_height = h;
            }

            {
                float const rcp_origin_aspect_ratio = static_cast<float>(origin_height) / static_cast<float>(origin_width);
                float best_score = FLT_MAX;
                for (uint32_t w = k_max_image_width_or_height; w > 0; w >>= 1)
                {
                    float const score = std::abs((static_cast<float>(target_height) / static_cast<float>((w))) - rcp_origin_aspect_ratio);
                    if (score < best_score)
                    {
                        best_score = score;
                        target_width = w;
                    }
                }
            }
        }
    }
}

static inline uint32_t internal_count_mips(uint32_t width, uint32_t height)
{
    assert(width >= 1U && height >= 1U);

    // DirectXTex/DirectXTexMipmaps.cpp CountMips

    uint32_t mip_level_count = 1U;

    while (width > 1U || height > 1U)
    {
        width = std::max(1U, width >> 1U);

        height = std::max(1U, height >> 1U);

        ++mip_level_count;
    }

    return mip_level_count;
}
