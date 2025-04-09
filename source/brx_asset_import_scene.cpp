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

#include "brx_asset_import_model_scene.h"
#include "internal_import_scene.h"
#include <cassert>

extern "C" brx_asset_import_scene *brx_asset_import_create_scene_from_input_stream(brx_asset_import_input_stream_factory *input_stream_factory, char const *input_stream_name)
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

    return brx_asset_import_create_scene_from_memory(input_stream_data.data(), input_stream_data.size());
}

extern "C" brx_asset_import_scene *brx_asset_import_create_scene_from_memory(void const *data_base, size_t data_size)
{
    void *new_unwrapped_model_scene_base = mcrt_malloc(sizeof(brx_asset_import_model_scene), alignof(brx_asset_import_model_scene));
    assert(NULL != new_unwrapped_model_scene_base);

    mcrt_vector<brx_asset_import_model_surface_group> surface_groups;
    mcrt_vector<brx_asset_import_model_animation> animations;
    if (internal_import_model_scene(data_base, data_size, surface_groups, animations))
    {
        brx_asset_import_model_scene *new_unwrapped_model_scene = new (new_unwrapped_model_scene_base) brx_asset_import_model_scene{std::move(surface_groups), std::move(animations)};
        return new_unwrapped_model_scene;
    }
    else
    {
        mcrt_free(new_unwrapped_model_scene_base);
        return NULL;
    }
}

extern "C" void brx_asset_import_destory_scene(brx_asset_import_scene *wrapped_scene)
{
    assert(NULL != wrapped_scene);
    brx_asset_import_model_scene *delete_unwrapped_scene = static_cast<brx_asset_import_model_scene *>(wrapped_scene);

    delete_unwrapped_scene->~brx_asset_import_model_scene();

    mcrt_free(delete_unwrapped_scene);
}
