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

#ifndef _INTERNAL_IMPORT_VRM_MODEL_H_
#define _INTERNAL_IMPORT_VRM_MODEL_H_ 1

#include "brx_asset_import_mesh_scene.h"
#include <cstddef>
#include <cstdint>

extern bool internal_import_vrm_model(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_mesh_surface_group> &out_surface_groups);

#endif
