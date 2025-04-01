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

#ifndef _BRX_ASSET_IMPORT_MODEL_SCENE_H_
#define _BRX_ASSET_IMPORT_MODEL_SCENE_H_ 1

#include "../include/brx_asset_import_scene.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../../McRT-Malloc/include/mcrt_string.h"

class brx_asset_import_model_surface final : public brx_asset_import_surface
{
    mcrt_vector<brx_asset_import_vertex_position> m_vertex_positions;
    mcrt_vector<brx_asset_import_vertex_varying> m_vertex_varyings;
    mcrt_vector<brx_asset_import_vertex_blending> m_vertex_blendings;

    mcrt_vector<mcrt_vector<brx_asset_import_vertex_position>> m_morph_targets_vertex_positions;
    mcrt_vector<mcrt_vector<brx_asset_import_vertex_varying>> m_morph_targets_vertex_varyings;
    uint32_t m_morph_target_name_indices[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_COUNT];

    // use vector instead of string to support data binary
    mcrt_vector<mcrt_vector<char>> m_texture_urls;
    uint32_t m_texture_name_indices[BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_COUNT];

public:
    brx_asset_import_model_surface(
        mcrt_vector<brx_asset_import_vertex_position> &&vertex_positions,
        mcrt_vector<brx_asset_import_vertex_varying> &&vertex_varyings,
        mcrt_vector<brx_asset_import_vertex_blending> &&vertex_blendings,
        mcrt_vector<mcrt_vector<brx_asset_import_vertex_position>> &&morph_targets_vertex_positions,
        mcrt_vector<mcrt_vector<brx_asset_import_vertex_varying>> &&morph_targets_vertex_varyings,
        uint32_t const *morph_target_name_indices,
        mcrt_vector<mcrt_vector<char>> &&texture_urls,
        uint32_t const *texture_name_indices);
    ~brx_asset_import_model_surface();

private:
    uint32_t get_vertex_count() const override;
    brx_asset_import_vertex_position const *get_vertex_position() const override;
    brx_asset_import_vertex_varying const *get_vertex_varying() const override;
    brx_asset_import_vertex_blending const *get_vertex_blending() const override;

    uint32_t get_morph_target_count() const override;
    brx_asset_import_vertex_position const *get_morph_target_vertex_positions(uint32_t morph_target_index) const override;
    brx_asset_import_vertex_varying const *get_morph_target_vertex_varyings(uint32_t morph_target_index) const override;
    uint32_t get_morph_target_name_index(BRX_ASSET_IMPORT_MORPH_TARGET_NAME morph_target_name) const override;

    uint32_t get_texture_count() const override;
    char const *get_texture_url(uint32_t texture_index) const override;
    uint32_t get_texture_index(BRX_ASSET_IMPORT_PBR_TEXTURE_NAME texture_name) const override;
};

class brx_asset_import_model_skeleton_joint
{
    mcrt_string m_skeleton_joint_name;
};

class brx_asset_import_model_surface_group final : public brx_asset_import_surface_group
{
    mcrt_vector<brx_asset_import_model_surface> m_surfaces;

    mcrt_vector<uint32_t> m_skeleton_joint_parent_indices;
    mcrt_vector<brx_asset_import_rigid_transform> m_skeleton_joint_bind_pose_transforms;
    uint32_t m_skeleton_joint_name_indices[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_COUNT];

public:
    brx_asset_import_model_surface_group(
        mcrt_vector<brx_asset_import_model_surface> &&surfaces,
        mcrt_vector<uint32_t> &&skeleton_joint_parent_indices,
        mcrt_vector<brx_asset_import_rigid_transform> &&skeleton_joint_bind_pose_transforms,
        uint32_t const *skeleton_joint_name_indices);
    ~brx_asset_import_model_surface_group();

private:
    uint32_t get_surface_count() const override;
    brx_asset_import_surface const *get_surface(uint32_t surface_index) const override;

    uint32_t const get_skeleton_joint_count() const override;
    uint32_t get_skeleton_joint_parent_index(uint32_t skeleton_joint_index) const override;
    brx_asset_import_rigid_transform const *get_skeleton_joint_bind_pose_transform_local_space(uint32_t skeleton_joint_index) const override;
    uint32_t get_skeleton_joint_index(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME vrm_skeleton_joint_name) const override;
};

class brx_asset_import_model_animation final : public brx_asset_import_animation
{
    mcrt_vector<mcrt_string> m_weight_channel_names;
    mcrt_vector<float> m_weights;
    mcrt_vector<mcrt_string> m_rigid_transform_channel_names;
    mcrt_vector<brx_asset_import_rigid_transform> m_rigid_transforms;
    mcrt_vector<mcrt_string> m_ik_switch_channel_names;
    mcrt_vector<bool> m_ik_switches;

public:
    brx_asset_import_model_animation(
        mcrt_vector<mcrt_string> &&weight_channel_names,
        mcrt_vector<float> &&weights,
        mcrt_vector<mcrt_string> &&rigid_transform_channel_names,
        mcrt_vector<brx_asset_import_rigid_transform> &&rigid_transforms,
        mcrt_vector<mcrt_string> &&ik_switch_channel_names,
        mcrt_vector<bool> &&ik_switches);
    ~brx_asset_import_model_animation();

private:
    uint32_t const get_frame_count() const override;
    uint32_t const get_weight_channel_count() const override;
    char const *get_weight_channel_name(uint32_t channel_index) const override;
    float const get_weight(uint32_t frame_index, uint32_t channel_index) const override;
    uint32_t const get_rigid_transform_channel_count() const override;
    char const *get_rigid_transform_channel_name(uint32_t channel_index) const override;
    brx_asset_import_rigid_transform const *get_rigid_transform(uint32_t frame_index, uint32_t channel_index) const override;
    uint32_t const get_ik_switch_channel_count() const override;
    char const *get_ik_switch_channel_name(uint32_t channel_index) const override;
    bool get_ik_switch(uint32_t frame_index, uint32_t channel_index) const override;
};

class brx_asset_import_model_scene final : public brx_asset_import_scene
{
    mcrt_vector<brx_asset_import_model_surface_group> m_surface_groups;

public:
    brx_asset_import_model_scene(mcrt_vector<brx_asset_import_model_surface_group> &&surface_groups);
    ~brx_asset_import_model_scene();

private:
    uint32_t get_surface_group_count() const override;
    brx_asset_import_surface_group const *get_surface_group(uint32_t group_index) const override;
    uint32_t get_animation_count() const override;
    brx_asset_import_animation *get_animation(uint32_t skeleton_animation_index) const override;
};

#endif
