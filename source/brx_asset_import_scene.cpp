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

#include "brx_asset_import_mesh_scene.h"
#include "internal_import_scene.h"
#include <cassert>

extern "C" brx_asset_import_scene *brx_asset_import_create_scene_from_memory(void const *data_base, size_t data_size)
{
    mcrt_vector<brx_asset_import_mesh_surface_group> surface_groups;
    mcrt_vector<brx_asset_import_mesh_animation> animations;
    if (internal_import_model_scene(data_base, data_size, surface_groups, animations))
    {
        void *new_unwrapped_model_scene_base = mcrt_malloc(sizeof(brx_asset_import_mesh_scene), alignof(brx_asset_import_mesh_scene));
        assert(NULL != new_unwrapped_model_scene_base);

        brx_asset_import_mesh_scene *new_unwrapped_model_scene = new (new_unwrapped_model_scene_base) brx_asset_import_mesh_scene{std::move(surface_groups), std::move(animations)};
        return new_unwrapped_model_scene;
    }
    else
    {
        return NULL;
    }
}

extern "C" void brx_asset_import_destroy_scene(brx_asset_import_scene *wrapped_scene)
{
    assert(NULL != wrapped_scene);
    brx_asset_import_mesh_scene *delete_unwrapped_scene = static_cast<brx_asset_import_mesh_scene *>(wrapped_scene);

    delete_unwrapped_scene->~brx_asset_import_mesh_scene();

    mcrt_free(delete_unwrapped_scene);
}
