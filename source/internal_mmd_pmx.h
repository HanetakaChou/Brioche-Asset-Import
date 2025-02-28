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

#ifndef _INTERNAL_MMD_PMX_H_
#define _INTERNAL_MMD_PMX_H_ 1

#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../../McRT-Malloc/include/mcrt_string.h"
#include <cstddef>
#include <cstdint>

static constexpr uint8_t const MMD_PMX_MAX_ADDITIONAL_VEC4_COUNT = 4U;

struct mmd_pmx_header_t
{
    uint8_t m_additional_vec4_count;
    mcrt_string m_name;
    mcrt_string m_comment;
};

struct mmd_pmx_vec2_t
{
    float m_x;
    float m_y;
};

struct mmd_pmx_vec3_t
{
    float m_x;
    float m_y;
    float m_z;
};

struct mmd_pmx_vec4_t
{
    float m_x;
    float m_y;
    float m_z;
    float m_w;
};

struct mmd_pmx_vertex_t
{
    mmd_pmx_vec3_t m_position;
    mmd_pmx_vec3_t m_normal;
    mmd_pmx_vec2_t m_uv;
    mmd_pmx_vec4_t m_additional_vec4s[MMD_PMX_MAX_ADDITIONAL_VEC4_COUNT];
    uint32_t m_bone_indices[4];
    float m_bone_weights[4];
};

struct mmd_pmx_face_t
{
    uint32_t m_vertex_indices[3];
};

struct mmd_pmx_texture_t
{
    mcrt_string m_path;
};

struct mmd_pmx_material_t
{
    mcrt_string m_name;
    mmd_pmx_vec4_t m_diffuse;
    bool m_is_double_sided;
    uint32_t m_texture_index;
    uint32_t m_face_count;
};

struct mmd_pmx_bone_t
{
    mmd_pmx_vec3_t m_position;
    uint32_t m_parent_index;
    uint32_t m_transform_order;
    bool m_has_ik; // self is the target position
    bool m_has_additional_rotation;
    bool m_has_additional_translation;
    bool m_transform_after_physics;
    mcrt_vector<uint32_t> m_ik_chain_bone_indices;
};

struct mmd_pmx_t
{
    mmd_pmx_header_t m_header;
    mcrt_vector<mmd_pmx_vertex_t> m_vertices;
    mcrt_vector<mmd_pmx_face_t> m_faces;
    mcrt_vector<mmd_pmx_texture_t> m_textures;
    mcrt_vector<mmd_pmx_material_t> m_materials;
    mcrt_vector<mmd_pmx_bone_t> m_bones;
};

extern bool internal_data_read_mmd_pmx(void const *data_base, size_t data_size, mmd_pmx_t *out_mmd_pmx);

#endif