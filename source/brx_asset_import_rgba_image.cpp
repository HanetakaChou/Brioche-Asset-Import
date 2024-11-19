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

brx_asset_import_albedo_image::brx_asset_import_albedo_image(mcrt_vector<uint32_t> &&pixel_data, uint32_t width, uint32_t height) : m_pixel_data(std::move(pixel_data)), m_width(width), m_height(height)
{
}

brx_asset_import_albedo_image::~brx_asset_import_albedo_image()
{
}

BRX_ASSET_IMPORT_IMAGE_FORMAT brx_asset_import_albedo_image::get_format() const
{
    return BRX_ASSET_IMPORT_IMAGE_FORMAT_R8G8B8A8_UNORM;
}

uint32_t brx_asset_import_albedo_image::get_width() const
{
    return this->m_width;
}

uint32_t brx_asset_import_albedo_image::get_height() const
{
    return this->m_height;
}

void const *brx_asset_import_albedo_image::get_pixel_data() const
{
    return this->m_pixel_data.data();
}

BRX_IMAGE_TYPE brx_asset_import_albedo_image::get_image_type() const
{
    return BRX_IMAGE_TYPE_ALBEDO;
}

brx_asset_import_illumiant_image::brx_asset_import_illumiant_image(mcrt_vector<uint64_t> &&pixel_data, uint32_t width, uint32_t height) : m_pixel_data(std::move(pixel_data)), m_width(width), m_height(height)
{
}

brx_asset_import_illumiant_image::~brx_asset_import_illumiant_image()
{
}

BRX_ASSET_IMPORT_IMAGE_FORMAT brx_asset_import_illumiant_image::get_format() const
{
    return BRX_ASSET_IMPORT_IMAGE_FORMAT_R16G16B16A16_SFLOAT;
}

uint32_t brx_asset_import_illumiant_image::get_width() const
{
    return this->m_width;
}

uint32_t brx_asset_import_illumiant_image::get_height() const
{
    return this->m_height;
}

void const *brx_asset_import_illumiant_image::get_pixel_data() const
{
    return this->m_pixel_data.data();
}

BRX_IMAGE_TYPE brx_asset_import_illumiant_image::get_image_type() const
{
    return BRX_IMAGE_TYPE_ILLUMINANT;
}
