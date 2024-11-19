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

#include "internal_import_scene.h"
#include "internal_import_gltf_scene.h"

// the unified facade
extern bool internal_import_scene(brx_asset_import_input_stream_factory* input_stream_factory, char const* file_name, mcrt_vector<brx_asset_import_model_group>& out_groups)
{
	return internal_import_gltf_scene(input_stream_factory, file_name, out_groups);
}