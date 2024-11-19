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
            void *new_unwrapped_image_base = mcrt_malloc(sizeof(brx_asset_import_albedo_image), alignof(brx_asset_import_albedo_image));
            assert(NULL != new_unwrapped_image_base);

            brx_asset_import_albedo_image *new_unwrapped_image = new (new_unwrapped_image_base) brx_asset_import_albedo_image{std::move(pixel_data), width, height};
            return new_unwrapped_image;
        }
        else
        {
            return NULL;
        }
    }
    break;
    case BRX_IMAGE_TYPE_ILLUMINANT:
    {
        mcrt_vector<uint64_t> pixel_data;
        uint32_t width;
        uint32_t height;
        if (internal_import_illumiant_image(data_base, data_size, pixel_data, &width, &height))
        {
            void *new_unwrapped_image_base = mcrt_malloc(sizeof(brx_asset_import_illumiant_image), alignof(brx_asset_import_illumiant_image));
            assert(NULL != new_unwrapped_image_base);

            brx_asset_import_illumiant_image *new_unwrapped_image = new (new_unwrapped_image_base) brx_asset_import_illumiant_image{std::move(pixel_data), width, height};
            return new_unwrapped_image;
        }
        else
        {
            return NULL;
        }
    }
    break;
    default:
    {
        return NULL;
    }
    }
}

extern "C" void brx_asset_import_destroy_image(brx_asset_import_image *wrapped_image)
{
    assert(NULL != wrapped_image);
    brx_asset_import_rgba_image *delete_unwrapped_rgba_image = static_cast<brx_asset_import_rgba_image *>(wrapped_image);
    switch (delete_unwrapped_rgba_image->get_image_type())
    {
    case BRX_IMAGE_TYPE_ALBEDO:
    {
        assert(BRX_IMAGE_TYPE_ALBEDO == delete_unwrapped_rgba_image->get_image_type());
        brx_asset_import_albedo_image *delete_unwrapped_albedo_image = static_cast<brx_asset_import_albedo_image *>(delete_unwrapped_rgba_image);
        delete_unwrapped_albedo_image->~brx_asset_import_albedo_image();
        mcrt_free(delete_unwrapped_albedo_image);
    }
    break;
    case BRX_IMAGE_TYPE_ILLUMINANT:
    {
        assert(BRX_IMAGE_TYPE_ILLUMINANT == delete_unwrapped_rgba_image->get_image_type());
        brx_asset_import_illumiant_image *delete_unwrapped_illumiant_image = static_cast<brx_asset_import_illumiant_image *>(delete_unwrapped_rgba_image);
        delete_unwrapped_illumiant_image->~brx_asset_import_illumiant_image();
        mcrt_free(delete_unwrapped_illumiant_image);
    }
    break;
    default:
    {
        assert(false);
    }
    }
}