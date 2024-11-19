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

#ifndef _BRX_ASSET_IMPORT_MESH_SCENE_H_
#define _BRX_ASSET_IMPORT_MESH_SCENE_H_ 1

#include "../include/brx_asset_import_scene.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../../McRT-Malloc/include/mcrt_string.h"

class brx_asset_import_mesh_surface final : public brx_asset_import_surface
{
    mcrt_vector<brx_asset_import_surface_vertex_position> m_vertex_positions;
    mcrt_vector<brx_asset_import_surface_vertex_varying> m_vertex_varyings;
    mcrt_vector<brx_asset_import_surface_vertex_blending> m_vertex_blendings;

    mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> m_morph_target_names;
    mcrt_vector<mcrt_vector<brx_asset_import_surface_vertex_position>> m_morph_targets_vertex_positions;
    mcrt_vector<mcrt_vector<brx_asset_import_surface_vertex_varying>> m_morph_targets_vertex_varyings;

    mcrt_vector<uint32_t> m_indices;

    bool m_is_double_sided;

    // use vector instead of string to support data binary
    mcrt_vector<uint8_t> m_emissive_image_url;
    brx_asset_import_vec3 m_emissive_factor;
    mcrt_vector<uint8_t> m_normal_image_url;
    float m_normal_scale;
    mcrt_vector<uint8_t> m_base_color_image_url;
    brx_asset_import_vec4 m_base_color_factor;
    mcrt_vector<uint8_t> m_metallic_roughness_image_url;
    float m_metallic_factor;
    float m_roughness_factor;

public:
    brx_asset_import_mesh_surface(
        mcrt_vector<brx_asset_import_surface_vertex_position> &&vertex_positions,
        mcrt_vector<brx_asset_import_surface_vertex_varying> &&vertex_varyings,
        mcrt_vector<brx_asset_import_surface_vertex_blending> &&vertex_blendings,
        mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> &&morph_target_names,
        mcrt_vector<mcrt_vector<brx_asset_import_surface_vertex_position>> &&morph_targets_vertex_positions,
        mcrt_vector<mcrt_vector<brx_asset_import_surface_vertex_varying>> &&morph_targets_vertex_varyings,
        mcrt_vector<uint32_t> &&indices,
        bool is_double_sided,
        mcrt_vector<uint8_t> &&emissive_image_url,
        brx_asset_import_vec3 const &emissive_factor,
        mcrt_vector<uint8_t> &&normal_image_url,
        float normal_scale,
        mcrt_vector<uint8_t> &&base_color_image_url,
        brx_asset_import_vec4 const &base_color_factor,
        mcrt_vector<uint8_t> &&metallic_roughness_image_url,
        float metallic_factor,
        float roughness_factor);
    ~brx_asset_import_mesh_surface();

private:
    uint32_t get_vertex_count() const override;
    brx_asset_import_surface_vertex_position const *get_vertex_positions() const override;
    brx_asset_import_surface_vertex_varying const *get_vertex_varyings() const override;
    brx_asset_import_surface_vertex_blending const *get_vertex_blendings() const override;

    uint32_t get_morph_target_count() const override;
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME get_morph_target_name(uint32_t morph_target_index) const override;
    brx_asset_import_surface_vertex_position const *get_morph_target_vertex_positions(uint32_t morph_target_index) const override;
    brx_asset_import_surface_vertex_varying const *get_morph_target_vertex_varyings(uint32_t morph_target_index) const override;

    uint32_t get_index_count() const override;
    uint32_t const *get_indices() const override;

    bool get_is_double_sided() const override;

    uint8_t const *get_emissive_image_url_data() const override;
    uint32_t get_emissive_image_url_size() const override;
    brx_asset_import_vec3 get_emissive_factor() const override;
    uint8_t const *get_normal_image_url_data() const override;
    uint32_t get_normal_image_url_size() const override;
    float get_normal_scale() const override;
    uint8_t const *get_base_color_image_url_data() const override;
    uint32_t get_base_color_image_url_size() const override;
    brx_asset_import_vec4 get_base_color_factor() const override;
    uint8_t const *get_metallic_roughness_image_url_data() const override;
    uint32_t get_metallic_roughness_image_url_size() const override;
    float get_metallic_factor() const override;
    float get_roughness_factor() const override;
};

class brx_asset_import_mesh_surface_group final : public brx_asset_import_surface_group
{
    mcrt_vector<brx_asset_import_mesh_surface> m_surfaces;

    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> m_animation_skeleton_joint_names;
    mcrt_vector<uint32_t> m_animation_skeleton_joint_parent_indices;
    mcrt_vector<brx_asset_import_rigid_transform> m_animation_skeleton_joint_transforms_bind_pose_local_space;

    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> m_animation_skeleton_joint_constraint_names;
    mcrt_vector<brx_asset_import_skeleton_joint_constraint> m_animation_skeleton_joint_constraints;
    mcrt_vector<mcrt_vector<uint32_t>> m_animation_skeleton_joint_constraints_storages;

    mcrt_vector<brx_asset_import_physics_rigid_body> m_ragdoll_skeleton_rigid_bodies;
    mcrt_vector<brx_asset_import_physics_constraint> m_ragdoll_skeleton_constraints;

    mcrt_vector<brx_asset_import_ragdoll_direct_mapping> m_animation_to_ragdoll_direct_mappings;
    mcrt_vector<brx_asset_import_ragdoll_direct_mapping> m_ragdoll_to_animation_direct_mappings;

public:
    brx_asset_import_mesh_surface_group(
        mcrt_vector<brx_asset_import_mesh_surface> &&surfaces,
        mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &&animation_skeleton_joint_names,
        mcrt_vector<uint32_t> &&animation_skeleton_joint_parent_indices,
        mcrt_vector<brx_asset_import_rigid_transform> &&animation_skeleton_joint_transforms_bind_pose_local_space,
        mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> &&animation_skeleton_joint_constraint_names,
        mcrt_vector<brx_asset_import_skeleton_joint_constraint> &&animation_skeleton_joint_constraints,
        mcrt_vector<mcrt_vector<uint32_t>> &&animation_skeleton_joint_constraints_storages,
        mcrt_vector<brx_asset_import_physics_rigid_body> &&ragdoll_skeleton_rigid_bodies,
        mcrt_vector<brx_asset_import_physics_constraint> &&ragdoll_skeleton_constraints,
        mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &&animation_to_ragdoll_direct_mappings,
        mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &&ragdoll_to_animation_direct_mappings);
    ~brx_asset_import_mesh_surface_group();

private:
    uint32_t get_surface_count() const override;
    brx_asset_import_surface const *get_surface(uint32_t surface_index) const override;

    uint32_t get_animation_skeleton_joint_count() const override;
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *get_animation_skeleton_joint_names() const override;
    uint32_t const *get_animation_skeleton_joint_parent_indices() const override;
    brx_asset_import_rigid_transform const *get_animation_skeleton_joint_transforms_bind_pose_local_space() const override;

    uint32_t get_animation_skeleton_joint_constraint_count() const override;
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *get_animation_skeleton_joint_constraint_names() const override;
    brx_asset_import_skeleton_joint_constraint const *get_animation_skeleton_joint_constraints() const override;

    uint32_t get_ragdoll_skeleton_rigid_body_count() const override;
    brx_asset_import_physics_rigid_body const *get_ragdoll_skeleton_rigid_bodies() const override;

    uint32_t get_ragdoll_skeleton_constraint_count() const override;
    brx_asset_import_physics_constraint const *get_ragdoll_skeleton_constraints() const override;

    uint32_t get_animation_to_ragdoll_direct_mapping_count() const override;
    brx_asset_import_ragdoll_direct_mapping const *get_animation_to_ragdoll_direct_mappings() const override;

    uint32_t get_ragdoll_to_animation_direct_mapping_count() const override;
    brx_asset_import_ragdoll_direct_mapping const *get_ragdoll_to_animation_direct_mappings() const override;
};

class brx_asset_import_mesh_animation final : public brx_asset_import_animation
{
    mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> m_weight_channel_names;
    mcrt_vector<float> m_weights;
    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> m_rigid_transform_channel_names;
    mcrt_vector<brx_asset_import_rigid_transform> m_rigid_transforms;
    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> m_switch_channel_names;
    mcrt_vector<uint8_t> m_switches;

public:
    brx_asset_import_mesh_animation(
        mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> &&weight_channel_names,
        mcrt_vector<float> &&weights,
        mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &&rigid_transform_channel_names,
        mcrt_vector<brx_asset_import_rigid_transform> &&rigid_transforms,
        mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> &&switch_channel_names,
        mcrt_vector<uint8_t> &&switches);
    ~brx_asset_import_mesh_animation();

private:
    uint32_t const get_frame_count() const override;

    uint32_t const get_weight_channel_count() const override;
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME const *get_weight_channel_names() const override;
    float const *get_weights() const override;

    uint32_t const get_rigid_transform_channel_count() const override;
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *get_rigid_transform_channel_names() const override;
    brx_asset_import_rigid_transform const *get_rigid_transforms() const override;

    uint32_t const get_switch_channel_count() const override;
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *get_switch_channel_names() const override;
    uint8_t const *get_switches() const override;
};

class brx_asset_import_mesh_scene final : public brx_asset_import_scene
{
    mcrt_vector<brx_asset_import_mesh_surface_group> m_surface_groups;
    mcrt_vector<brx_asset_import_mesh_animation> m_animations;

public:
    brx_asset_import_mesh_scene(
        mcrt_vector<brx_asset_import_mesh_surface_group> &&surface_groups,
        mcrt_vector<brx_asset_import_mesh_animation> &&animations);
    ~brx_asset_import_mesh_scene();

private:
    uint32_t get_surface_group_count() const override;
    brx_asset_import_surface_group const *get_surface_group(uint32_t group_index) const override;
    uint32_t get_animation_count() const override;
    brx_asset_import_animation const *get_animation(uint32_t skeleton_animation_index) const override;
};

#endif
