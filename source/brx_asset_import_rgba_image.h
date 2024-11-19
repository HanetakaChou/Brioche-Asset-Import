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
#include "internal_import_image.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"

class brx_asset_import_rgba_image : public brx_asset_import_image
{
public:
    virtual BRX_IMAGE_TYPE get_image_type() const = 0;
};

class brx_asset_import_albedo_image final : public brx_asset_import_rgba_image
{
    mcrt_vector<uint32_t> m_pixel_data;
    uint32_t m_width;
    uint32_t m_height;

public:
    brx_asset_import_albedo_image(mcrt_vector<uint32_t> &&pixel_data, uint32_t width, uint32_t height);
    ~brx_asset_import_albedo_image();

private:
    BRX_ASSET_IMPORT_IMAGE_FORMAT get_format() const override;
    uint32_t get_width() const override;
    uint32_t get_height() const override;
    void const *get_pixel_data() const override;
    BRX_IMAGE_TYPE get_image_type() const override;
};

class brx_asset_import_illumiant_image final : public brx_asset_import_rgba_image
{
    mcrt_vector<uint64_t> m_pixel_data;
    uint32_t m_width;
    uint32_t m_height;

public:
    brx_asset_import_illumiant_image(mcrt_vector<uint64_t> &&pixel_data, uint32_t width, uint32_t height);
    ~brx_asset_import_illumiant_image();

private:
    BRX_ASSET_IMPORT_IMAGE_FORMAT get_format() const override;
    uint32_t get_width() const override;
    uint32_t get_height() const override;
    void const *get_pixel_data() const override;
    BRX_IMAGE_TYPE get_image_type() const override;
};

#endif
