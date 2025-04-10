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

extern "C" brx_asset_import_image *brx_asset_import_create_image_from_input_stream(brx_asset_import_input_stream_factory *input_stream_factory, char const *input_stream_name)
{
    mcrt_vector<uint8_t> input_stream_data;
    {
        brx_asset_import_input_stream *input_stream;
        if (NULL == (input_stream = input_stream_factory->create_instance(input_stream_name)))
        {
            return NULL;
        }

        int64_t length;
        if (-1 == input_stream->stat_size(&length))
        {
            input_stream_factory->destory_instance(input_stream);
            return NULL;
        }

        if ((length <= 0) || (length >= static_cast<int64_t>(static_cast<int64_t>(1) << static_cast<int64_t>(24))))
        {
            input_stream_factory->destory_instance(input_stream);
            return NULL;
        }

        size_t input_stream_size = static_cast<uint32_t>(length);
        assert(static_cast<int64_t>(input_stream_size) == length);

        input_stream_data.resize(input_stream_size);

        intptr_t read_size = input_stream->read(input_stream_data.data(), input_stream_data.size());

        input_stream_factory->destory_instance(input_stream);

        if (-1 == read_size || read_size != input_stream_size)
        {
            input_stream_data.clear();
            return NULL;
        }
    }
    assert(!input_stream_data.empty());

    return brx_asset_import_create_image_from_memory(input_stream_data.data(), input_stream_data.size());
}

extern "C" brx_asset_import_image *brx_asset_import_create_image_from_memory(void const *data_base, size_t data_size)
{
    switch (internal_import_image_type(data_base, data_size))
    {
    case BRX_IMAGE_TYPE_ALBEDO:
    {
        mcrt_vector<mcrt_vector<uint32_t>> pixel_data;
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

extern "C" void brx_asset_import_destory_image(brx_asset_import_image *wrapped_image)
{
    assert(NULL != wrapped_image);
    brx_asset_import_rgba_image *delete_unwrapped_image = static_cast<brx_asset_import_rgba_image *>(wrapped_image);

    delete_unwrapped_image->~brx_asset_import_rgba_image();
    mcrt_free(delete_unwrapped_image);
}