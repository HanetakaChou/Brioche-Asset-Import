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
#include "internal_import_tga_image.h"
#include "internal_import_exr_image.h"
#include <cassert>
#include <algorithm>

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
    else if (data_size >= 4U &&
             (118 == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
             (47 == reinterpret_cast<uint8_t const *>(data_base)[1]) &&
             (49 == reinterpret_cast<uint8_t const *>(data_base)[2]) &&
             (1 == reinterpret_cast<uint8_t const *>(data_base)[3]))
    {
        // parse_header.c: read_magic_and_flags
        // 118  47  49  1
        // v    /   1   SOH
        return BRX_IMAGE_TYPE_ILLUMINANT;
    }
    else if (data_size >= 18U &&
             ((1 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (2 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (3 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (9 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (10 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (11 == reinterpret_cast<uint8_t const *>(data_base)[2])))
    {
        // TGA
        return BRX_IMAGE_TYPE_ALBEDO;
    }
    else
    {
        return BRX_IMAGE_TYPE_UNKNOWN;
    }
}

extern bool internal_import_albedo_image(void const *data_base, size_t data_size, mcrt_vector<uint32_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
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
        // webp_dec.c: ParseRIFF
        // 82 73 70 70 _ _ _ _ 87 69 66 80
        // R  I  F  F  _ _ _ _ W  E  B  P

        status_internal_import_image = internal_import_webp_image(data_base, data_size, out_pixel_data, out_width, out_height);
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

        status_internal_import_image = internal_import_png_image(data_base, data_size, out_pixel_data, out_width, out_height);
    }
    else if (data_size >= 2U &&
             (0XFFU == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
             (0XD8U == reinterpret_cast<uint8_t const *>(data_base)[1]))
    {
        // jdmarker.c: first_marker
        // 0XFF 0XD8
        // N/A  SOI

        status_internal_import_image = internal_import_jpeg_image(data_base, data_size, out_pixel_data, out_width, out_height);
    }
    else if (data_size >= 18U &&
             ((1 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (2 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (3 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (9 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (10 == reinterpret_cast<uint8_t const *>(data_base)[2]) ||
              (11 == reinterpret_cast<uint8_t const *>(data_base)[2])))
    {
        // TGA
        status_internal_import_image = internal_import_tga_image(data_base, data_size, out_pixel_data, out_width, out_height);
    }
    else
    {
        status_internal_import_image = false;
    }

    return status_internal_import_image;
}

extern bool internal_import_illumiant_image(void const *data_base, size_t data_size, mcrt_vector<uint64_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
    bool status_internal_import_image;
    if (data_size >= 4U &&
        (118 == reinterpret_cast<uint8_t const *>(data_base)[0]) &&
        (47 == reinterpret_cast<uint8_t const *>(data_base)[1]) &&
        (49 == reinterpret_cast<uint8_t const *>(data_base)[2]) &&
        (1 == reinterpret_cast<uint8_t const *>(data_base)[3]))
    {
        // parse_header.c: read_magic_and_flags
        // 118  47  49  1
        // v    /   1   SOH
        status_internal_import_image = internal_import_exr_image(data_base, data_size, out_pixel_data, out_width, out_height);
    }
    else
    {
        status_internal_import_image = false;
    }

    return status_internal_import_image;
}
