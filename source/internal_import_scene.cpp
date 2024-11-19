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
#include "internal_import_mmd_model.h"
#include "internal_import_mmd_motion.h"
#include "internal_import_vrm_model.h"

extern bool internal_import_model_scene(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_mesh_surface_group> &out_surface_groups, mcrt_vector<brx_asset_import_mesh_animation> &out_animations)
{
    bool status_internal_import_scene;
    if (data_size >= 8U && (80U == reinterpret_cast<uint8_t const *>(data_base)[0]) && (77U == reinterpret_cast<uint8_t const *>(data_base)[1]) && (88U == reinterpret_cast<uint8_t const *>(data_base)[2]) && (32U == reinterpret_cast<uint8_t const *>(data_base)[3]) && (((0U == reinterpret_cast<uint8_t const *>(data_base)[4]) && (0U == reinterpret_cast<uint8_t const *>(data_base)[5]) && (0U == reinterpret_cast<uint8_t const *>(data_base)[6]) && (64U == reinterpret_cast<uint8_t const *>(data_base)[7])) || ((102U == reinterpret_cast<uint8_t const *>(data_base)[4]) && (102U == reinterpret_cast<uint8_t const *>(data_base)[5]) && (6U == reinterpret_cast<uint8_t const *>(data_base)[6]) && (64U == reinterpret_cast<uint8_t const *>(data_base)[7]))))
    {
        // "PMX "
        // 2.0F / 2.1F
        status_internal_import_scene = internal_import_mmd_model(data_base, data_size, out_surface_groups);
    }
    else if (data_size >= 25U && (86U == reinterpret_cast<uint8_t const *>(data_base)[0]) && (111U == reinterpret_cast<uint8_t const *>(data_base)[1]) && (99U == reinterpret_cast<uint8_t const *>(data_base)[2]) && (97U == reinterpret_cast<uint8_t const *>(data_base)[3]) && (108U == reinterpret_cast<uint8_t const *>(data_base)[4]) && (111U == reinterpret_cast<uint8_t const *>(data_base)[5]) && (105U == reinterpret_cast<uint8_t const *>(data_base)[6]) && (100U == reinterpret_cast<uint8_t const *>(data_base)[7]) && (32U == reinterpret_cast<uint8_t const *>(data_base)[8]) && (77U == reinterpret_cast<uint8_t const *>(data_base)[9]) && (111U == reinterpret_cast<uint8_t const *>(data_base)[10]) && (116U == reinterpret_cast<uint8_t const *>(data_base)[11]) && (105U == reinterpret_cast<uint8_t const *>(data_base)[12]) && (111U == reinterpret_cast<uint8_t const *>(data_base)[13]) && (110U == reinterpret_cast<uint8_t const *>(data_base)[14]) && (32U == reinterpret_cast<uint8_t const *>(data_base)[15]) && (68U == reinterpret_cast<uint8_t const *>(data_base)[16]) && (97U == reinterpret_cast<uint8_t const *>(data_base)[17]) && (116U == reinterpret_cast<uint8_t const *>(data_base)[18]) && (97U == reinterpret_cast<uint8_t const *>(data_base)[19]) && (32U == reinterpret_cast<uint8_t const *>(data_base)[20]) && (48U == reinterpret_cast<uint8_t const *>(data_base)[21]) && (48U == reinterpret_cast<uint8_t const *>(data_base)[22]) && (48U == reinterpret_cast<uint8_t const *>(data_base)[23]) && (50U == reinterpret_cast<uint8_t const *>(data_base)[24]))
    {
        // "Vocaloid Motion Data 0002"
        status_internal_import_scene = internal_import_mmd_motion(data_base, data_size, out_animations);
    }
    else if (data_size >= 4U && (103U == reinterpret_cast<uint8_t const *>(data_base)[0]) && (108U == reinterpret_cast<uint8_t const *>(data_base)[1]) && (84U == reinterpret_cast<uint8_t const *>(data_base)[2]) && (70U == reinterpret_cast<uint8_t const *>(data_base)[3]))
    {
        // "glTF"
        status_internal_import_scene = internal_import_vrm_model(data_base, data_size, out_surface_groups);
    }
    else
    {
        status_internal_import_scene = false;
    }

    return status_internal_import_scene;
}
