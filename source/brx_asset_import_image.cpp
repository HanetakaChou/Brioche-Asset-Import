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

extern "C" brx_asset_import_image *brx_asset_import_create_image_from_memory(void const *data_base, size_t data_size)
{
    switch (internal_import_image_type(data_base, data_size))
    {
    case BRX_IMAGE_TYPE_ALBEDO:
    {
        mcrt_vector<uint32_t> pixel_data;
        uint32_t width;
        uint32_t height;
        if (internal_import_albedo_image(data_base, data_size, pixel_data, &width, &height))
        {
            void *new_unwrapped_image_base = mcrt_malloc(sizeof(brx_asset_import_rgba_image), alignof(brx_asset_import_rgba_image));
            assert(NULL != new_unwrapped_image_base);

            brx_asset_import_rgba_image *new_unwrapped_image = new (new_unwrapped_image_base) brx_asset_import_rgba_image{std::move(pixel_data), width, height};
            return new_unwrapped_image;
        }
        else
        {
            return NULL;
        }
    }
    break;
    case BRX_IMAGE_TYPE_UNKNOWN:
    default:
    {
        return NULL;
    }
    }
}

extern "C" void brx_asset_import_destroy_image(brx_asset_import_image *wrapped_image)
{
    assert(NULL != wrapped_image);
    brx_asset_import_rgba_image *delete_unwrapped_image = static_cast<brx_asset_import_rgba_image *>(wrapped_image);

    delete_unwrapped_image->~brx_asset_import_rgba_image();
    mcrt_free(delete_unwrapped_image);
}