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

#ifndef _INTERNAL_MMD_VMD_H_
#define _INTERNAL_MMD_VMD_H_ 1

#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../../McRT-Malloc/include/mcrt_string.h"
#include <cstddef>
#include <cstdint>

struct mmd_vmd_vec3_t
{
    float m_x;
    float m_y;
    float m_z;
};

struct mmd_vmd_vec4_t
{
    float m_x;
    float m_y;
    float m_z;
    float m_w;
};

struct mmd_vmd_header_t
{
    mcrt_string m_name;
};

struct mmd_vmd_motion_t
{
    mcrt_string m_name;
    uint32_t m_frame_number;
    mmd_vmd_vec3_t m_translation;
    mmd_vmd_vec4_t m_rotation;
    uint8_t m_translation_x_cubic_bezier[4];
    uint8_t m_translation_y_cubic_bezier[4];
    uint8_t m_translation_z_cubic_bezier[4];
    uint8_t m_rotation_cubic_bezier[4];
};

struct mmd_vmd_morph_t
{
    mcrt_string m_name;
    uint32_t m_frame_number;
    float m_weight;
};

struct mmd_vmd_camera_t
{
    uint32_t m_frame_number;
    mmd_vmd_vec3_t m_focus_position;
    mmd_vmd_vec3_t m_rotation;
    float m_distance;
    float m_fov_angle;
    bool m_orthographic;
    uint8_t m_focus_position_x_cubic_bezier[4];
    uint8_t m_focus_position_y_cubic_bezier[4];
    uint8_t m_focus_position_z_cubic_bezier[4];
    uint8_t m_rotation_cubic_bezier[4];
    uint8_t m_distance_cubic_bezier[4];
    uint8_t m_fov_angle_cubic_bezier[4];
};

struct mmd_vmd_ik_t
{
    mcrt_string m_name;
    uint32_t m_frame_number;
    bool m_enable;
};

struct mmd_vmd_t
{
    mmd_vmd_header_t m_header;
    mcrt_vector<mmd_vmd_motion_t> m_motions;
    mcrt_vector<mmd_vmd_morph_t> m_morphs;
    mcrt_vector<mmd_vmd_camera_t> m_cameras;
    mcrt_vector<mmd_vmd_ik_t> m_iks;
};

extern bool internal_data_read_mmd_vmd(void const *data_base, size_t data_size, mmd_vmd_t *out_mmd_vmd);

#endif