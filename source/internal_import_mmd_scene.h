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

#ifndef _INTERNAL_IMPORT_MMD_SCENE_H_
#define _INTERNAL_IMPORT_MMD_SCENE_H_ 1

#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../include/brx_asset_import_scene.h"
#include <cstddef>
#include <cstdint>


// GLMMDModel::UpdateAnimation
// PMXModel::Update

extern bool internal_import_mmd_scene(
	void const* data_base,
	size_t data_size,
	mcrt_vector<mcrt_vector<brx_asset_import_geometry_vertex_position>> &out_morph_target_vertex_position_data,
	mcrt_vector<brx_asset_import_skeleton_animation_joint_transform> &out_inverse_bind_pose_data
);

#endif