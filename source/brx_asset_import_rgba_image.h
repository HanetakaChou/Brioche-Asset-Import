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

#ifndef _BRX_ASSET_IMPORT_RGBA_IMAGE_H_
#define _BRX_ASSET_IMPORT_RGBA_IMAGE_H_ 1

#include "../include/brx_asset_import_image.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"

class brx_asset_import_rgba_image final : public brx_asset_import_image
{
    mcrt_vector<mcrt_vector<uint32_t>> m_pixel_data;
    uint32_t m_width;
    uint32_t m_height;

public:
    brx_asset_import_rgba_image(mcrt_vector<mcrt_vector<uint32_t>> &&pixel_data, uint32_t width, uint32_t height);
    ~brx_asset_import_rgba_image();

private:
    BRX_ASSET_IMPORT_IMAGE_FORMAT get_format() const override;
    uint32_t get_mip_level_count() const override;
    uint32_t get_row_pitch(uint32_t mip_level_index) const override;
    uint32_t get_row_size(uint32_t mip_level_index) const override;
    uint32_t get_row_count(uint32_t mip_level_index) const override;
    uint32_t get_width(uint32_t mip_level_index) const override;
    uint32_t get_height(uint32_t mip_level_index) const override;
    void const *get_pixel_data(uint32_t mip_level_index) const override;
};

#endif
