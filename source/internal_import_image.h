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

#ifndef _INTERNAL_IMPORT_IMAGE_H_
#define _INTERNAL_IMPORT_IMAGE_H_ 1

#include "../../McRT-Malloc/include/mcrt_vector.h"
#include <cstddef>
#include <cstdint>

enum BRX_IMAGE_TYPE
{
    BRX_IMAGE_TYPE_UNKNOWN = 0,
    BRX_IMAGE_TYPE_ALBEDO = 1,
    BRX_IMAGE_TYPE_ILLUMINANT = 2
};

// https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-limits
// D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION 16384
static constexpr size_t const k_max_image_width_or_height = 16384U;

static constexpr int const k_albedo_image_channel_size = sizeof(uint8_t);
static constexpr int const k_albedo_image_num_channels = 4U;

static constexpr int const k_illumiant_image_channel_size = sizeof(uint16_t);
static constexpr int const k_illumiant_image_num_channels = 4U;

extern BRX_IMAGE_TYPE internal_import_image_type(void const *data_base, size_t data_size);

extern bool internal_import_albedo_image(void const *data_base, size_t data_size, mcrt_vector<uint32_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height);

extern bool internal_import_illumiant_image(void const *data_base, size_t data_size, mcrt_vector<uint64_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height);

#endif