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

#include "internal_import_jpeg_image.h"
#include "internal_import_image.h"
#include <iostream>
#include <stdexcept>
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include "../thirdparty/libjpeg/jpeglib.h"

static void _internal_libjpeg_error_exit_callback(j_common_ptr cinfo);

extern bool internal_import_jpeg_image(void const *data_base, size_t data_size, mcrt_vector<uint32_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
    // https://chromium.googlesource.com/webm/libwebp/+/refs/heads/main/imageio/jpegdec.c
    // https://github.com/microsoft/DirectXTex/blob/main/Auxiliary/DirectXTexJPEG.cpp
    // https://github.com/mitsuba-renderer/mitsuba3/blob/master/src/core/bitmap.cpp

    jpeg_decompress_struct cinfo = {};
    bool has_error = false;
    try
    {
        jpeg_error_mgr err = {};
        jpeg_std_error(&err);
        err.error_exit = _internal_libjpeg_error_exit_callback;

        cinfo.err = &err;
        jpeg_create_decompress(&cinfo);

        jpeg_mem_src(&cinfo, reinterpret_cast<unsigned char const *>(data_base), static_cast<unsigned long>(data_size));

        int const status_jpeg_read_header = jpeg_read_header(&cinfo, TRUE);
        if (JPEG_HEADER_OK != status_jpeg_read_header)
        {
            throw std::runtime_error{"jpeg read header"};
        }

        cinfo.out_color_space = JCS_EXT_RGBA;
        cinfo.do_fancy_upsampling = TRUE;

        boolean const status_jpeg_start_decompress = jpeg_start_decompress(&cinfo);
        if (TRUE != status_jpeg_start_decompress)
        {
            throw std::runtime_error{"jpeg start decompress"};
        }

        JDIMENSION const width = cinfo.output_width;
        JDIMENSION const height = cinfo.output_height;

        if ((width > k_max_image_width_or_height) || (height > k_max_image_width_or_height))
        {
            throw std::runtime_error("Size Overflow");
        }

        if ((width < 1U) || (height < 1U))
        {
            throw std::runtime_error("Size Zero");
        }

        if (!((JCS_EXT_RGBA == cinfo.out_color_space) && (k_albedo_image_num_channels == cinfo.out_color_components) && (k_albedo_image_num_channels == cinfo.output_components)))
        {
            throw std::runtime_error("NOT RGBA8 Format");
        }

        uint64_t const _uint64_stride = static_cast<uint64_t>(k_albedo_image_channel_size) * static_cast<uint64_t>(k_albedo_image_num_channels) * static_cast<uint64_t>(width);
        uint64_t const _uint64_num_pixels = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);

        uintptr_t const stride = static_cast<uintptr_t>(_uint64_stride);
        size_t const num_pixels = static_cast<size_t>(_uint64_num_pixels);

        if (!((stride == _uint64_stride) && (num_pixels == _uint64_num_pixels)))
        {
            throw std::runtime_error("Size Overflow");
        }

        out_pixel_data.resize(num_pixels);

        JDIMENSION const batch_height = cinfo.rec_outbuf_height;

        mcrt_vector<JSAMPROW> rows(static_cast<size_t>(batch_height));

        JDIMENSION height_index = 0;
        while ((height_index < height) && (cinfo.output_scanline < cinfo.output_height))
        {
            for (JDIMENSION batch_height_index = 0; batch_height_index < batch_height; ++batch_height_index)
            {
                rows[batch_height_index] = reinterpret_cast<JSAMPROW>(reinterpret_cast<uintptr_t>(out_pixel_data.data()) + stride * (height_index + batch_height_index));
            }

            JDIMENSION const num_rows_read = jpeg_read_scanlines(&cinfo, rows.data(), batch_height);

            if (batch_height != num_rows_read)
            {
                if (!((num_rows_read >= 1) && (num_rows_read < batch_height)))
                {
                    throw std::runtime_error("jpeg read scanlines");
                }
            }

            height_index += num_rows_read;
        }

        if (TRUE != jpeg_finish_decompress(&cinfo))
        {
            throw std::runtime_error("jpeg finish decompress");
        }

        (*out_width) = width;
        (*out_height) = height;
    }
    catch (std::runtime_error exception)
    {
        std::cout << exception.what() << std::endl;

        has_error = true;
    }

    jpeg_destroy_decompress(&cinfo);

    return (!has_error);
}

static void _internal_libjpeg_error_exit_callback(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    cinfo->err->format_message(cinfo, buffer);

    throw std::runtime_error{buffer};
}
