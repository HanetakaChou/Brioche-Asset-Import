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

#ifndef _BRX_ASSET_IMPORT_IMAGE_H_
#define _BRX_ASSET_IMPORT_IMAGE_H_ 1

#include <cstddef>
#include <cstdint>

enum BRX_ASSET_IMPORT_IMAGE_FORMAT
{
    BRX_ASSET_IMPORT_IMAGE_FORMAT_R8G8B8A8_UNORM = 0,
    BRX_ASSET_IMPORT_IMAGE_FORMAT_R16G16B16A16_SFLOAT = 1
};

class brx_asset_import_image
{
public:
    virtual BRX_ASSET_IMPORT_IMAGE_FORMAT get_format() const = 0;
    virtual uint32_t get_width() const = 0;
    virtual uint32_t get_height() const = 0;
    virtual void const *get_pixel_data() const = 0;
};

extern "C" brx_asset_import_image *brx_asset_import_create_image_from_memory(void const *data_base, size_t data_size);
extern "C" void brx_asset_import_destroy_image(brx_asset_import_image *image);

#endif
