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

#include "brx_asset_import_model_scene.h"
#include "internal_import_scene.h"
#include <cstring>
#include <cassert>

extern "C" brx_asset_import_scene *brx_asset_import_create_scene(brx_asset_import_input_stream_factory *input_stream_factory, char const *file_name)
{
    mcrt_vector<brx_asset_import_model_surface_group> surface_groups;
    internal_import_scene(input_stream_factory, file_name, surface_groups);

    return NULL;
}

brx_asset_import_model_surface::brx_asset_import_model_surface(
    mcrt_vector<brx_asset_import_vertex_position> &&vertex_positions,
    mcrt_vector<brx_asset_import_vertex_varying> &&vertex_varyings,
    mcrt_vector<brx_asset_import_vertex_blending> &&vertex_blendings,
    mcrt_vector<mcrt_vector<brx_asset_import_vertex_position>> &&morph_targets_vertex_positions,
    mcrt_vector<mcrt_vector<brx_asset_import_vertex_varying>> &&morph_targets_vertex_varyings,
    uint32_t const *morph_target_name_indices,
    mcrt_vector<mcrt_vector<char>> &&texture_urls,
    uint32_t const *texture_name_indices)
    : m_vertex_positions(std::move(vertex_positions)),
      m_vertex_varyings(std::move(vertex_varyings)),
      m_vertex_blendings(std::move(vertex_blendings)),
      m_morph_targets_vertex_positions(std::move(morph_targets_vertex_positions)),
      m_morph_targets_vertex_varyings(std::move(morph_targets_vertex_varyings)),
      m_texture_urls(std::move(texture_urls))
{
    std::memcpy(this->m_morph_target_name_indices, morph_target_name_indices, sizeof(uint32_t) * BRX_ASSET_IMPORT_MORPH_TARGET_NAME_COUNT);
    std::memcpy(this->m_texture_name_indices, texture_name_indices, sizeof(uint32_t) * BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_BASE_COLOR);
}

brx_asset_import_model_surface::~brx_asset_import_model_surface()
{
}

brx_asset_import_vertex_blending const *brx_asset_import_model_surface::get_vertex_blending() const
{
    return (!this->m_vertex_blendings.empty()) ? this->m_vertex_blendings.data() : NULL;
}

uint32_t brx_asset_import_model_surface::get_morph_target_count() const
{
    return this->m_morph_targets_vertex_positions.size();
}

brx_asset_import_vertex_position const *brx_asset_import_model_surface::get_morph_target_vertex_positions(uint32_t morph_target_index) const
{
    return this->m_morph_targets_vertex_positions[morph_target_index].data();
}

brx_asset_import_vertex_varying const *brx_asset_import_model_surface::get_morph_target_vertex_varyings(uint32_t morph_target_index) const
{
    return this->m_morph_targets_vertex_varyings[morph_target_index].data();
}

uint32_t brx_asset_import_model_surface::get_morph_target_name_index(BRX_ASSET_IMPORT_MORPH_TARGET_NAME morph_target_name) const
{
    return this->m_morph_target_name_indices[morph_target_name];
}

uint32_t brx_asset_import_model_surface::get_texture_count() const
{
    return this->m_texture_urls.size();
}

char const *brx_asset_import_model_surface::get_texture_url(uint32_t texture_index) const
{
    return this->m_texture_urls[texture_index].data();
}

uint32_t brx_asset_import_model_surface::get_texture_index(BRX_ASSET_IMPORT_PBR_TEXTURE_NAME texture_name) const
{
    return this->m_texture_name_indices[texture_name];
}

brx_asset_import_model_surface_group::brx_asset_import_model_surface_group(
    mcrt_vector<brx_asset_import_model_surface> &&surfaces,
    mcrt_vector<uint32_t> &&skeleton_joint_parent_indices,
    mcrt_vector<brx_asset_import_rigid_transform> &&skeleton_joint_bind_pose_transforms,
    uint32_t const *skeleton_joint_name_indices)
    : m_surfaces(surfaces),
      m_skeleton_joint_parent_indices(std::move(skeleton_joint_parent_indices)),
      m_skeleton_joint_bind_pose_transforms(std::move(skeleton_joint_bind_pose_transforms))
{
    std::memcpy(this->m_skeleton_joint_name_indices, skeleton_joint_name_indices, sizeof(uint32_t) * BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_COUNT);
}

brx_asset_import_model_surface_group::~brx_asset_import_model_surface_group()
{
}

uint32_t brx_asset_import_model_surface_group::get_surface_count() const
{
    return this->m_surfaces.size();
}

brx_asset_import_surface const *brx_asset_import_model_surface_group::get_surface(uint32_t surface_index) const
{
    return &this->m_surfaces[surface_index];
}

uint32_t const brx_asset_import_model_surface_group::get_skeleton_joint_count() const
{
    return this->m_skeleton_joint_parent_indices.size();
}

uint32_t brx_asset_import_model_surface_group::get_skeleton_joint_parent_index(uint32_t skeleton_joint_index) const
{
    return this->m_skeleton_joint_parent_indices[skeleton_joint_index];
}

brx_asset_import_rigid_transform const *brx_asset_import_model_surface_group::get_skeleton_joint_bind_pose_transform_local_space(uint32_t skeleton_joint_index) const
{
    return &this->m_skeleton_joint_bind_pose_transforms[skeleton_joint_index];
}

uint32_t brx_asset_import_model_surface_group::get_skeleton_joint_index(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME skeleton_joint_name) const
{
    return this->m_skeleton_joint_name_indices[skeleton_joint_name];
}

brx_asset_import_model_animation::brx_asset_import_model_animation(
    mcrt_vector<mcrt_string> &&weight_channel_names,
    mcrt_vector<float> &&weights,
    mcrt_vector<mcrt_string> &&rigid_transform_channel_names,
    mcrt_vector<brx_asset_import_rigid_transform> &&rigid_transforms,
    mcrt_vector<mcrt_string> &&ik_switch_channel_names,
    mcrt_vector<bool> &&ik_switches)
    : m_weight_channel_names(std::move(weight_channel_names)),
      m_weights(std::move(weights)),
      m_rigid_transform_channel_names(std::move(rigid_transform_channel_names)),
      m_rigid_transforms(std::move(rigid_transforms)),
      m_ik_switch_channel_names(std::move(ik_switch_channel_names)),
      m_ik_switches(std::move(ik_switches))

{
}

brx_asset_import_model_animation::~brx_asset_import_model_animation()
{
}

uint32_t const brx_asset_import_model_animation::get_frame_count() const
{
    if ((!this->m_weight_channel_names.empty()) && (!this->m_rigid_transform_channel_names.empty()) && (!this->m_ik_switch_channel_names.empty()))
    {
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));
        assert(0U == (this->m_ik_switches.size() % this->m_ik_switch_channel_names.size()));
        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();
        assert((this->m_weights.size() / this->m_weight_channel_names.size()) == frame_count);
        assert((this->m_ik_switches.size() / this->m_ik_switch_channel_names.size()) == frame_count);
        return frame_count;
    }
    else if ((!this->m_weight_channel_names.empty()) && (!this->m_rigid_transform_channel_names.empty()))
    {
        assert(this->m_ik_switch_channel_names.empty());
        assert(this->m_ik_switches.empty());
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));
        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();
        assert((this->m_weights.size() / this->m_weight_channel_names.size()) == frame_count);
        return frame_count;
    }
    else if ((!this->m_weight_channel_names.empty()) && (!this->m_ik_switch_channel_names.empty()))
    {
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        assert(0U == (this->m_ik_switches.size() % this->m_ik_switch_channel_names.size()));
        uint32_t const frame_count = this->m_weights.size() / this->m_weight_channel_names.size();
        assert((this->m_ik_switches.size() / this->m_ik_switch_channel_names.size()) == frame_count);
        return frame_count;
    }
    else if ((!this->m_rigid_transform_channel_names.empty()) && (!this->m_ik_switch_channel_names.empty()))
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));
        assert(0U == (this->m_ik_switches.size() % this->m_ik_switch_channel_names.size()));
        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();
        assert((this->m_ik_switches.size() / this->m_ik_switch_channel_names.size()) == frame_count);
        return frame_count;
    }
    else if (!this->m_weight_channel_names.empty())
    {
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(this->m_ik_switch_channel_names.empty());
        assert(this->m_ik_switches.empty());
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        uint32_t const frame_count = this->m_weights.size() / this->m_weight_channel_names.size();
        return frame_count;
    }
    else if (!this->m_rigid_transform_channel_names.empty())
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(this->m_ik_switch_channel_names.empty());
        assert(this->m_ik_switches.empty());
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));
        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();
        return frame_count;
    }
    else if (!this->m_ik_switch_channel_names.empty())
    {
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(this->m_ik_switch_channel_names.empty());
        assert(this->m_ik_switches.empty());
        assert(0U == (this->m_ik_switches.size() % this->m_ik_switch_channel_names.size()));
        uint32_t const frame_count = (this->m_ik_switches.size() / this->m_ik_switch_channel_names.size());
        return frame_count;
    }
    else
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(this->m_ik_switch_channel_names.empty());
        assert(this->m_ik_switches.empty());
        return 0U;
    }
}

uint32_t const brx_asset_import_model_animation::get_weight_channel_count() const
{
    return this->m_weight_channel_names.size();
}

char const *brx_asset_import_model_animation::get_weight_channel_name(uint32_t channel_index) const
{
    return this->m_weight_channel_names[channel_index].c_str();
}

float const brx_asset_import_model_animation::get_weight(uint32_t frame_index, uint32_t channel_index) const
{
    return this->m_weights[this->m_weight_channel_names.size() * frame_index + channel_index];
}

uint32_t const brx_asset_import_model_animation::get_rigid_transform_channel_count() const
{
    return this->m_rigid_transform_channel_names.size();
}

char const *brx_asset_import_model_animation::get_rigid_transform_channel_name(uint32_t channel_index) const
{
    return this->m_rigid_transform_channel_names[channel_index].c_str();
}

uint32_t const brx_asset_import_model_animation::get_ik_switch_channel_count() const
{
    return this->m_ik_switch_channel_names.size();
}

char const *brx_asset_import_model_animation::get_ik_switch_channel_name(uint32_t channel_index) const
{
    return this->m_ik_switch_channel_names[channel_index].c_str();
}

brx_asset_import_rigid_transform const *brx_asset_import_model_animation::get_rigid_transform(uint32_t frame_index, uint32_t channel_index) const
{
    return &this->m_rigid_transforms[this->m_rigid_transform_channel_names.size() * frame_index + channel_index];
}

bool brx_asset_import_model_animation::get_ik_switch(uint32_t frame_index, uint32_t channel_index) const
{
    return this->m_ik_switches[this->m_ik_switch_channel_names.size() * frame_index + channel_index];
}

brx_asset_import_model_scene::brx_asset_import_model_scene(mcrt_vector<brx_asset_import_model_surface_group> &&surface_groups) : m_surface_groups(std::move(surface_groups))
{
}

brx_asset_import_model_scene::~brx_asset_import_model_scene()
{
}

uint32_t brx_asset_import_model_scene::get_surface_group_count() const
{
    return this->m_surface_groups.size();
}

brx_asset_import_surface_group const *brx_asset_import_model_scene::get_surface_group(uint32_t group_index) const
{
    return &this->m_surface_groups[group_index];
}

uint32_t brx_asset_import_model_scene::get_animation_count() const
{
    return 0U;
}

brx_asset_import_animation *brx_asset_import_model_scene::get_animation(uint32_t skeleton_animation_index) const
{
    return NULL;
}