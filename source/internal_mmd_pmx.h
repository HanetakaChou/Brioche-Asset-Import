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

struct mmd_pmx_header_t
{
    mcrt_string m_name;
    mcrt_string m_comment;
};

struct mmd_pmx_vertex_t
{
    mmd_pmx_vec3_t m_position;
    mmd_pmx_vec3_t m_normal;
    mmd_pmx_vec2_t m_uv;
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
    bool m_is_double_sided;
    mmd_pmx_vec4_t m_diffuse;
    uint32_t m_texture_index;
    uint32_t m_face_count;
};

struct mmd_pmx_bone_t
{
    mcrt_string m_name;
    mmd_pmx_vec3_t m_translation;
    uint32_t m_parent_index;
    uint32_t m_transformation_hierarchy;
    bool m_meta_physics;
    bool m_append_rotation;
    bool m_append_translation;
    bool m_append_local;
    uint32_t m_append_parent_index;
    float m_append_rate;
    bool m_ik;
    uint32_t m_ik_end_effector_index;
    mcrt_vector<uint32_t> m_ik_link_indices;
    bool m_ik_two_links_hinge_limit_angle;
    mmd_pmx_vec3_t m_ik_two_links_hinge_limit_angle_min;
    mmd_pmx_vec3_t m_ik_two_links_hinge_limit_angle_max;
};

union mmd_pmx_morph_offset_t
{
    struct
    {
        uint32_t m_morph_index;
        float m_morph_weight;
    } m_group;

    struct
    {
        uint32_t m_vertex_index;
        mmd_pmx_vec3_t m_vertex_position;
    } m_vertex_position;

    struct
    {
        uint32_t m_vertex_index;
        mmd_pmx_vec2_t m_vertex_uv;
    } m_vertex_uv;
};

struct mmd_pmx_morph_t
{
    mcrt_string m_name;
    // 0: group
    // 1: vertex position
    // 2: vertex uv
    // 3: empty
    uint32_t m_morph_type;
    mcrt_vector<mmd_pmx_morph_offset_t> m_offsets;
};

struct mmd_pmx_rigid_body_t
{
    mcrt_string m_name;
    uint32_t m_bone_index;
    uint32_t m_collision_filter_group;
    uint32_t m_collision_filter_mask;
    uint32_t m_shape_type;
    mmd_pmx_vec3_t m_shape_size;
    mmd_pmx_vec3_t m_translation;
    mmd_pmx_vec3_t m_rotation;
    float m_mass;
    float m_linear_damping;
    float m_angular_damping;
    float m_friction;
    float m_restitution;
    uint32_t m_rigid_body_type;
};

struct mmd_pmx_constraint_t
{
    mcrt_string m_name;
    uint32_t m_rigid_body_a_index;
    uint32_t m_rigid_body_b_index;
    mmd_pmx_vec3_t m_translation;
    mmd_pmx_vec3_t m_rotation;
    mmd_pmx_vec3_t m_translation_limit_min;
    mmd_pmx_vec3_t m_translation_limit_max;
    mmd_pmx_vec3_t m_rotation_limit_min;
    mmd_pmx_vec3_t m_rotation_limit_max;
};

struct mmd_pmx_t
{
    mmd_pmx_header_t m_header;
    mcrt_vector<mmd_pmx_vertex_t> m_vertices;
    mcrt_vector<mmd_pmx_face_t> m_faces;
    mcrt_vector<mmd_pmx_texture_t> m_textures;
    mcrt_vector<mmd_pmx_material_t> m_materials;
    mcrt_vector<mmd_pmx_bone_t> m_bones;
    mcrt_vector<mmd_pmx_morph_t> m_morphs;
    mcrt_vector<mmd_pmx_rigid_body_t> m_rigid_bodies;
    mcrt_vector<mmd_pmx_constraint_t> m_constraints;
};

extern bool internal_data_read_mmd_pmx(void const *data_base, size_t data_size, mmd_pmx_t *out_mmd_pmx);

#endif