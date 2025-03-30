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

#include "../include/import_scene_asset.h"
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <cmath>
#include <algorithm>
#include <assert.h>
#include "brx_asset_import_model_scene.h"

extern bool internal_import_mmd_motion(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_model_surface_group> &out_surface_groups);

extern bool internal_import_mmd_model(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_model_surface_group> &out_surface_groups);

extern bool import_gltf_scene_asset(mcrt_vector<scene_mesh_data> &out_total_mesh_data, float frame_rate, brx_asset_import_input_stream_factory *input_stream_factory, char const *input_stream_name)
{
    std::vector<uint8_t> input_stream_data;
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

    mcrt_vector<brx_asset_import_model_surface_group> surface_groups;
    internal_import_mmd_motion(input_stream_data.data(), input_stream_data.size(), surface_groups);

    return true;
}