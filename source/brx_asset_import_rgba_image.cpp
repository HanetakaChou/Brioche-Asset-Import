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

#include "brx_asset_import_rgba_image.h"
#include "internal_import_image.h"
#include <cassert>

brx_asset_import_rgba_image::brx_asset_import_rgba_image(mcrt_vector<mcrt_vector<uint32_t>> &&pixel_data, uint32_t width, uint32_t height) : m_pixel_data(std::move(pixel_data)), m_width(width), m_height(height)
{
}

brx_asset_import_rgba_image::~brx_asset_import_rgba_image()
{
}

BRX_ASSET_IMPORT_IMAGE_FORMAT brx_asset_import_rgba_image::get_format() const
{
    return BRX_ASSET_IMPORT_IMAGE_FORMAT_R8G8B8A8;
}

uint32_t brx_asset_import_rgba_image::get_mip_level_count() const
{
    return this->m_pixel_data.size();
}

uint32_t brx_asset_import_rgba_image::get_row_pitch(uint32_t mip_level_index) const
{
    return this->get_row_size(mip_level_index);
}

uint32_t brx_asset_import_rgba_image::get_row_size(uint32_t mip_level_index) const
{
    return (static_cast<uint32_t>(k_albedo_image_channel_size) * static_cast<uint32_t>(k_albedo_image_num_channels) * static_cast<uint32_t>(this->get_width(mip_level_index)));
}

uint32_t brx_asset_import_rgba_image::get_row_count(uint32_t mip_level_index) const
{
    return this->get_width(mip_level_index);
}

uint32_t brx_asset_import_rgba_image::get_width(uint32_t mip_level_index) const
{
    return std::max(1U, this->m_width >> mip_level_index);
}

uint32_t brx_asset_import_rgba_image::get_height(uint32_t mip_level_index) const
{
    return std::max(1U, this->m_height >> mip_level_index);
}

void const *brx_asset_import_rgba_image::get_pixel_data(uint32_t mip_level_index) const
{
    return this->m_pixel_data[mip_level_index].data();
}
