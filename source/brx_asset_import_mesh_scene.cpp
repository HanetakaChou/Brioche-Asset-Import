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

#include "brx_asset_import_mesh_scene.h"
#include <cstring>
#include <cassert>

brx_asset_import_mesh_surface::brx_asset_import_mesh_surface(
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
    float roughness_factor)
    : m_vertex_positions(std::move(vertex_positions)),
      m_vertex_varyings(std::move(vertex_varyings)),
      m_vertex_blendings(std::move(vertex_blendings)),
      m_morph_target_names(std::move(morph_target_names)),
      m_morph_targets_vertex_positions(std::move(morph_targets_vertex_positions)),
      m_morph_targets_vertex_varyings(std::move(morph_targets_vertex_varyings)),
      m_indices(std::move(indices)),
      m_is_double_sided(is_double_sided),
      m_emissive_image_url(std::move(emissive_image_url)),
      m_emissive_factor(emissive_factor),
      m_normal_image_url(std::move(normal_image_url)),
      m_normal_scale(normal_scale),
      m_base_color_image_url(std::move(base_color_image_url)),
      m_base_color_factor(base_color_factor),
      m_metallic_roughness_image_url(std::move(metallic_roughness_image_url)),
      m_metallic_factor(metallic_factor),
      m_roughness_factor(roughness_factor)
{
}

brx_asset_import_mesh_surface::~brx_asset_import_mesh_surface()
{
}

uint32_t brx_asset_import_mesh_surface::get_vertex_count() const
{
    uint32_t const vertex_count = this->m_vertex_positions.size();
    assert(this->m_vertex_varyings.size() == vertex_count);
    assert(this->m_vertex_blendings.empty() || this->m_vertex_blendings.size() == vertex_count);
#ifndef NDEBUG
    for (mcrt_vector<brx_asset_import_surface_vertex_position> const &morph_target_vertex_positions : this->m_morph_targets_vertex_positions)
    {
        assert(morph_target_vertex_positions.size() == vertex_count);
    }

    for (mcrt_vector<brx_asset_import_surface_vertex_varying> const &morph_target_vertex_varyings : this->m_morph_targets_vertex_varyings)
    {
        assert(morph_target_vertex_varyings.size() == vertex_count);
    }
#endif
    return vertex_count;
}

brx_asset_import_surface_vertex_position const *brx_asset_import_mesh_surface::get_vertex_positions() const
{
    return (!this->m_vertex_positions.empty()) ? this->m_vertex_positions.data() : NULL;
}

brx_asset_import_surface_vertex_varying const *brx_asset_import_mesh_surface::get_vertex_varyings() const
{
    return (!this->m_vertex_varyings.empty()) ? this->m_vertex_varyings.data() : NULL;
}

brx_asset_import_surface_vertex_blending const *brx_asset_import_mesh_surface::get_vertex_blendings() const
{
    return (!this->m_vertex_blendings.empty()) ? this->m_vertex_blendings.data() : NULL;
}

uint32_t brx_asset_import_mesh_surface::get_morph_target_count() const
{
    uint32_t const morph_target_count = this->m_morph_target_names.size();
    assert(this->m_morph_targets_vertex_positions.size() == morph_target_count);
    assert(this->m_morph_targets_vertex_varyings.size() == morph_target_count);
    return morph_target_count;
}

BRX_ASSET_IMPORT_MORPH_TARGET_NAME brx_asset_import_mesh_surface::get_morph_target_name(uint32_t morph_target_index) const
{
    return this->m_morph_target_names[morph_target_index];
}

brx_asset_import_surface_vertex_position const *brx_asset_import_mesh_surface::get_morph_target_vertex_positions(uint32_t morph_target_index) const
{
    return this->m_morph_targets_vertex_positions[morph_target_index].data();
}

brx_asset_import_surface_vertex_varying const *brx_asset_import_mesh_surface::get_morph_target_vertex_varyings(uint32_t morph_target_index) const
{
    return this->m_morph_targets_vertex_varyings[morph_target_index].data();
}

uint32_t brx_asset_import_mesh_surface::get_index_count() const
{
    return this->m_indices.size();
}

uint32_t const *brx_asset_import_mesh_surface::get_indices() const
{
    return (!this->m_indices.empty()) ? this->m_indices.data() : NULL;
}

bool brx_asset_import_mesh_surface::get_is_double_sided() const
{
    return this->m_is_double_sided;
}

uint8_t const *brx_asset_import_mesh_surface::get_emissive_image_url_data() const
{
    return (!this->m_emissive_image_url.empty()) ? this->m_emissive_image_url.data() : NULL;
}

uint32_t brx_asset_import_mesh_surface::get_emissive_image_url_size() const
{
    return (!this->m_emissive_image_url.empty()) ? this->m_emissive_image_url.size() : 0U;
}

brx_asset_import_vec3 brx_asset_import_mesh_surface::get_emissive_factor() const
{
    return this->m_emissive_factor;
}

uint8_t const *brx_asset_import_mesh_surface::get_normal_image_url_data() const
{
    return (!this->m_normal_image_url.empty()) ? this->m_normal_image_url.data() : NULL;
}

uint32_t brx_asset_import_mesh_surface::get_normal_image_url_size() const
{
    return (!this->m_normal_image_url.empty()) ? this->m_normal_image_url.size() : 0U;
}

float brx_asset_import_mesh_surface::get_normal_scale() const
{
    return this->m_normal_scale;
}

uint8_t const *brx_asset_import_mesh_surface::get_base_color_image_url_data() const
{
    return (!this->m_base_color_image_url.empty()) ? this->m_base_color_image_url.data() : NULL;
}

uint32_t brx_asset_import_mesh_surface::get_base_color_image_url_size() const
{
    return (!this->m_base_color_image_url.empty()) ? this->m_base_color_image_url.size() : 0U;
}

brx_asset_import_vec4 brx_asset_import_mesh_surface::get_base_color_factor() const
{
    return this->m_base_color_factor;
}

uint8_t const *brx_asset_import_mesh_surface::get_metallic_roughness_image_url_data() const
{
    return (!this->m_metallic_roughness_image_url.empty()) ? this->m_metallic_roughness_image_url.data() : NULL;
}

uint32_t brx_asset_import_mesh_surface::get_metallic_roughness_image_url_size() const
{
    return (!this->m_metallic_roughness_image_url.empty()) ? this->m_metallic_roughness_image_url.size() : 0U;
}

float brx_asset_import_mesh_surface::get_metallic_factor() const
{
    return this->m_metallic_factor;
}

float brx_asset_import_mesh_surface::get_roughness_factor() const
{
    return this->m_roughness_factor;
}

brx_asset_import_mesh_surface_group::brx_asset_import_mesh_surface_group(
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
    mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &&ragdoll_to_animation_direct_mappings)
    : m_surfaces(std::move(surfaces)),
      m_animation_skeleton_joint_names(std::move(animation_skeleton_joint_names)),
      m_animation_skeleton_joint_parent_indices(std::move(animation_skeleton_joint_parent_indices)),
      m_animation_skeleton_joint_transforms_bind_pose_local_space(std::move(animation_skeleton_joint_transforms_bind_pose_local_space)),
      m_animation_skeleton_joint_constraint_names(std::move(animation_skeleton_joint_constraint_names)),
      m_animation_skeleton_joint_constraints(std::move(animation_skeleton_joint_constraints)),
      m_animation_skeleton_joint_constraints_storages(std::move(animation_skeleton_joint_constraints_storages)),
      m_ragdoll_skeleton_rigid_bodies(std::move(ragdoll_skeleton_rigid_bodies)),
      m_ragdoll_skeleton_constraints(std::move(ragdoll_skeleton_constraints)),
      m_animation_to_ragdoll_direct_mappings(std::move(animation_to_ragdoll_direct_mappings)),
      m_ragdoll_to_animation_direct_mappings(std::move(ragdoll_to_animation_direct_mappings))
{
    uint32_t const animation_skeleton_joint_constraint_count = this->m_animation_skeleton_joint_constraint_names.size();
    assert(this->m_animation_skeleton_joint_constraints.size() == animation_skeleton_joint_constraint_count);
    assert(this->m_animation_skeleton_joint_constraints_storages.size() == animation_skeleton_joint_constraint_count);

    for (uint32_t animation_skeleton_joint_constraint_index = 0; animation_skeleton_joint_constraint_index < animation_skeleton_joint_constraint_count; ++animation_skeleton_joint_constraint_index)
    {
        if (BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_COPY_TRANSFORM == this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_constraint_type)
        {
            assert(this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_copy_transform.m_source_weight_count == this->m_animation_skeleton_joint_constraints_storages[animation_skeleton_joint_constraint_index].size());

            assert(reinterpret_cast<uintptr_t>(this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_copy_transform.m_source_weights) == reinterpret_cast<uintptr_t>(this->m_animation_skeleton_joint_constraints_storages[animation_skeleton_joint_constraint_index].data()));
            this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_copy_transform.m_source_weights = reinterpret_cast<float *>(this->m_animation_skeleton_joint_constraints_storages[animation_skeleton_joint_constraint_index].data());
        }
        else
        {
            assert(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_INVERSE_KINEMATICS == this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_constraint_type);

            assert(this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_inverse_kinematics.m_ik_joint_count == this->m_animation_skeleton_joint_constraints_storages[animation_skeleton_joint_constraint_index].size());

            assert(reinterpret_cast<uintptr_t>(this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_inverse_kinematics.m_ik_joint_indices) == reinterpret_cast<uintptr_t>(this->m_animation_skeleton_joint_constraints_storages[animation_skeleton_joint_constraint_index].data()));
            this->m_animation_skeleton_joint_constraints[animation_skeleton_joint_constraint_index].m_inverse_kinematics.m_ik_joint_indices = this->m_animation_skeleton_joint_constraints_storages[animation_skeleton_joint_constraint_index].data();
        }
    }
}

brx_asset_import_mesh_surface_group::~brx_asset_import_mesh_surface_group()
{
}

uint32_t brx_asset_import_mesh_surface_group::get_surface_count() const
{
    return this->m_surfaces.size();
}

brx_asset_import_surface const *brx_asset_import_mesh_surface_group::get_surface(uint32_t surface_index) const
{
    return &this->m_surfaces[surface_index];
}

uint32_t brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_count() const
{
    uint32_t const animation_skeleton_joint_count = this->m_animation_skeleton_joint_names.size();
    assert(this->m_animation_skeleton_joint_parent_indices.size() == animation_skeleton_joint_count);
    assert(this->m_animation_skeleton_joint_transforms_bind_pose_local_space.size() == animation_skeleton_joint_count);
    return animation_skeleton_joint_count;
}

BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_names() const
{
    return this->m_animation_skeleton_joint_names.data();
}

uint32_t const *brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_parent_indices() const
{
    return this->m_animation_skeleton_joint_parent_indices.data();
}

brx_asset_import_rigid_transform const *brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_transforms_bind_pose_local_space() const
{
    return this->m_animation_skeleton_joint_transforms_bind_pose_local_space.data();
}

uint32_t brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_constraint_count() const
{
    uint32_t const animation_skeleton_joint_constraint_count = this->m_animation_skeleton_joint_constraint_names.size();
    assert(this->m_animation_skeleton_joint_constraints.size() == animation_skeleton_joint_constraint_count);
    assert(this->m_animation_skeleton_joint_constraints_storages.size() == animation_skeleton_joint_constraint_count);
    return animation_skeleton_joint_constraint_count;
}

BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_constraint_names() const
{
    return this->m_animation_skeleton_joint_constraint_names.data();
}

brx_asset_import_skeleton_joint_constraint const *brx_asset_import_mesh_surface_group::get_animation_skeleton_joint_constraints() const
{
    return this->m_animation_skeleton_joint_constraints.data();
}

uint32_t brx_asset_import_mesh_surface_group::get_ragdoll_skeleton_rigid_body_count() const
{
    return this->m_ragdoll_skeleton_rigid_bodies.size();
}

brx_asset_import_physics_rigid_body const *brx_asset_import_mesh_surface_group::get_ragdoll_skeleton_rigid_bodies() const
{
    return this->m_ragdoll_skeleton_rigid_bodies.data();
}

uint32_t brx_asset_import_mesh_surface_group::get_ragdoll_skeleton_constraint_count() const
{
    return this->m_ragdoll_skeleton_constraints.size();
}

brx_asset_import_physics_constraint const *brx_asset_import_mesh_surface_group::get_ragdoll_skeleton_constraints() const
{
    return this->m_ragdoll_skeleton_constraints.data();
}

uint32_t brx_asset_import_mesh_surface_group::get_animation_to_ragdoll_direct_mapping_count() const
{
    return this->m_animation_to_ragdoll_direct_mappings.size();
}

brx_asset_import_ragdoll_direct_mapping const *brx_asset_import_mesh_surface_group::get_animation_to_ragdoll_direct_mappings() const
{
    return this->m_animation_to_ragdoll_direct_mappings.data();
}

uint32_t brx_asset_import_mesh_surface_group::get_ragdoll_to_animation_direct_mapping_count() const
{
    return this->m_ragdoll_to_animation_direct_mappings.size();
}

brx_asset_import_ragdoll_direct_mapping const *brx_asset_import_mesh_surface_group::get_ragdoll_to_animation_direct_mappings() const
{
    return this->m_ragdoll_to_animation_direct_mappings.data();
}

brx_asset_import_mesh_animation::brx_asset_import_mesh_animation(
    mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> &&weight_channel_names,
    mcrt_vector<float> &&weights,
    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &&rigid_transform_channel_names,
    mcrt_vector<brx_asset_import_rigid_transform> &&rigid_transforms,
    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> &&switch_channel_names,
    mcrt_vector<uint8_t> &&switches)
    : m_weight_channel_names(std::move(weight_channel_names)),
      m_weights(std::move(weights)),
      m_rigid_transform_channel_names(std::move(rigid_transform_channel_names)),
      m_rigid_transforms(std::move(rigid_transforms)),
      m_switch_channel_names(std::move(switch_channel_names)),
      m_switches(std::move(switches))

{
}

brx_asset_import_mesh_animation::~brx_asset_import_mesh_animation()
{
}

uint32_t const brx_asset_import_mesh_animation::get_frame_count() const
{
    if ((!this->m_weight_channel_names.empty()) && (!this->m_rigid_transform_channel_names.empty()) && (!this->m_switch_channel_names.empty()))
    {
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));
        assert(0U == (this->m_switches.size() % this->m_switch_channel_names.size()));

        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();

        assert((this->m_weights.size() / this->m_weight_channel_names.size()) == frame_count);
        assert((this->m_switches.size() / this->m_switch_channel_names.size()) == frame_count);

        return frame_count;
    }
    else if ((!this->m_weight_channel_names.empty()) && (!this->m_rigid_transform_channel_names.empty()))
    {
        assert(this->m_switch_channel_names.empty());
        assert(this->m_switches.empty());
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));

        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();

        assert((this->m_weights.size() / this->m_weight_channel_names.size()) == frame_count);

        return frame_count;
    }
    else if ((!this->m_weight_channel_names.empty()) && (!this->m_switch_channel_names.empty()))
    {
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));
        assert(0U == (this->m_switches.size() % this->m_switch_channel_names.size()));

        uint32_t const frame_count = this->m_weights.size() / this->m_weight_channel_names.size();

        assert((this->m_switches.size() / this->m_switch_channel_names.size()) == frame_count);

        return frame_count;
    }
    else if ((!this->m_rigid_transform_channel_names.empty()) && (!this->m_switch_channel_names.empty()))
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));
        assert(0U == (this->m_switches.size() % this->m_switch_channel_names.size()));

        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();

        assert((this->m_switches.size() / this->m_switch_channel_names.size()) == frame_count);

        return frame_count;
    }
    else if (!this->m_weight_channel_names.empty())
    {
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(this->m_switch_channel_names.empty());
        assert(this->m_switches.empty());
        assert(0U == (this->m_weights.size() % this->m_weight_channel_names.size()));

        uint32_t const frame_count = this->m_weights.size() / this->m_weight_channel_names.size();

        return frame_count;
    }
    else if (!this->m_rigid_transform_channel_names.empty())
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(this->m_switch_channel_names.empty());
        assert(this->m_switches.empty());
        assert(0U == (this->m_rigid_transforms.size() % this->m_rigid_transform_channel_names.size()));

        uint32_t const frame_count = this->m_rigid_transforms.size() / this->m_rigid_transform_channel_names.size();

        return frame_count;
    }
    else if (!this->m_switch_channel_names.empty())
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(0U == (this->m_switches.size() % this->m_switch_channel_names.size()));

        uint32_t const frame_count = (this->m_switches.size() / this->m_switch_channel_names.size());

        return frame_count;
    }
    else
    {
        assert(this->m_weight_channel_names.empty());
        assert(this->m_weights.empty());
        assert(this->m_rigid_transform_channel_names.empty());
        assert(this->m_rigid_transforms.empty());
        assert(this->m_switch_channel_names.empty());
        assert(this->m_switches.empty());

        return 0U;
    }
}

uint32_t const brx_asset_import_mesh_animation::get_weight_channel_count() const
{
    return this->m_weight_channel_names.size();
}

BRX_ASSET_IMPORT_MORPH_TARGET_NAME const *brx_asset_import_mesh_animation::get_weight_channel_names() const
{
    return this->m_weight_channel_names.data();
}

float const *brx_asset_import_mesh_animation::get_weights() const
{
    return this->m_weights.data();
}

uint32_t const brx_asset_import_mesh_animation::get_rigid_transform_channel_count() const
{
    return this->m_rigid_transform_channel_names.size();
}

BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *brx_asset_import_mesh_animation::get_rigid_transform_channel_names() const
{
    return this->m_rigid_transform_channel_names.data();
}

brx_asset_import_rigid_transform const *brx_asset_import_mesh_animation::get_rigid_transforms() const
{
    return this->m_rigid_transforms.data();
}

uint32_t const brx_asset_import_mesh_animation::get_switch_channel_count() const
{
    return this->m_switch_channel_names.size();
}

BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *brx_asset_import_mesh_animation::get_switch_channel_names() const
{
    return this->m_switch_channel_names.data();
}

uint8_t const *brx_asset_import_mesh_animation::get_switches() const
{
    return this->m_switches.data();
}

brx_asset_import_mesh_scene::brx_asset_import_mesh_scene(
    mcrt_vector<brx_asset_import_mesh_surface_group> &&surface_groups,
    mcrt_vector<brx_asset_import_mesh_animation> &&animations)
    : m_surface_groups(std::move(surface_groups)),
      m_animations(std::move(animations))
{
}

brx_asset_import_mesh_scene::~brx_asset_import_mesh_scene()
{
}

uint32_t brx_asset_import_mesh_scene::get_surface_group_count() const
{
    return this->m_surface_groups.size();
}

brx_asset_import_surface_group const *brx_asset_import_mesh_scene::get_surface_group(uint32_t group_index) const
{
    return &this->m_surface_groups[group_index];
}

uint32_t brx_asset_import_mesh_scene::get_animation_count() const
{
    return this->m_animations.size();
}

brx_asset_import_animation const *brx_asset_import_mesh_scene::get_animation(uint32_t skeleton_animation_index) const
{
    return &this->m_animations[skeleton_animation_index];
}
