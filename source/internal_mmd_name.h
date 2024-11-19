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

#ifndef _INTERNAL_MMD_NAME_H_
#define _INTERNAL_MMD_NAME_H_ 1

#include "../include/brx_asset_import_scene.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../../McRT-Malloc/include/mcrt_string.h"

extern void internal_fill_mmd_morph_target_name_strings(mcrt_vector<mcrt_vector<mcrt_string>> &out_mmd_morph_target_name_strings);

extern void internal_fill_mmd_skeleton_joint_name_strings(mcrt_vector<mcrt_vector<mcrt_string>> &out_mmd_skeleton_joint_name_strings);

extern void internal_fill_mmd_skeleton_joint_constraint_name_strings(mcrt_vector<mcrt_vector<mcrt_string>> &out_mmd_skeleton_joint_constraint_name_strings);

#endif
