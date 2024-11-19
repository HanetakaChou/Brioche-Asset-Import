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

#ifndef _IMPORT_SCENE_ASSET_H_
#define _IMPORT_SCENE_ASSET_H_ 1

#include <stddef.h>
#include <stdint.h>
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#include <DirectXMath.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include "brx_asset_import_input_stream.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"
#include "../../McRT-Malloc/include/mcrt_string.h"

class scene_animation_joint
{
    DirectX::XMFLOAT4 m_quaternion;
    DirectX::XMFLOAT3 m_translation;

public:
    inline void init()
    {
        DirectX::XMStoreFloat4(&this->m_quaternion, DirectX::XMQuaternionIdentity());
        DirectX::XMStoreFloat3(&this->m_translation, DirectX::XMVectorZero());
    }

    inline void set_transform(DirectX::XMFLOAT4 const &quaternion, DirectX::XMFLOAT3 const &translation)
    {
        this->m_quaternion = quaternion;
        this->m_translation = translation;
    }

    inline DirectX::XMFLOAT4 get_quaternion() const
    {
        return this->m_quaternion;
    }

    inline DirectX::XMFLOAT3 get_translation() const
    {
        return this->m_translation;
    }
};

class scene_animation_pose
{
    mcrt_vector<scene_animation_joint> m_joints;

public:
    inline void init(size_t joint_count)
    {
        this->m_joints.resize(joint_count);

        for (size_t joint_index = 0; joint_index < joint_count; ++joint_index)
        {
            this->m_joints[joint_index].init();
        }
    }

    inline void set_transform(size_t joint_index, DirectX::XMFLOAT4 const &quaternion, DirectX::XMFLOAT3 const &translation)
    {
        this->m_joints[joint_index].set_transform(quaternion, translation);
    }

    inline size_t get_joint_count() const
    {
        return this->m_joints.size();
    }

    inline DirectX::XMFLOAT4 get_quaternion(size_t joint_index) const
    {
        assert(joint_index < this->m_joints.size());
        return this->m_joints[joint_index].get_quaternion();
    }

    inline DirectX::XMFLOAT3 get_translation(size_t joint_index) const
    {
        assert(joint_index < this->m_joints.size());
        return this->m_joints[joint_index].get_translation();
    }
};

// Bake Animation
// Since this demo is for rendering, we do NOT care too much about the animation. And we simply sample the poses for each frame.
class scene_animation_skeleton
{
    mcrt_vector<scene_animation_pose> m_poses;

public:
    inline void init(size_t frame_count, size_t joint_count)
    {
        this->m_poses.resize(frame_count);

        for (size_t frame_index = 0; frame_index < frame_count; ++frame_index)
        {
            this->m_poses[frame_index].init(joint_count);
        }
    }

    inline void set_transform(size_t frame_index, size_t joint_index, DirectX::XMFLOAT4 const &quaternion, DirectX::XMFLOAT3 const &translation)
    {
        this->m_poses[frame_index].set_transform(joint_index, quaternion, translation);
    }

    inline size_t get_joint_count() const
    {
        return ((this->m_poses.size() > 0) ? (this->m_poses[0].get_joint_count()) : 0);
    }

    inline scene_animation_pose const &get_pose(size_t frame_index) const
    {
        size_t const frame_count = this->m_poses.size();
        assert(frame_count > 0);

        size_t const pose_index = frame_index % frame_count;
        return this->m_poses[pose_index];
    }
};

struct scene_mesh_vertex_position_binding
{
    // R32G32B32_FLOAT
    DirectX::XMFLOAT3 m_position;
};

struct scene_mesh_vertex_varying_binding
{
    // R16G16_SNORM (octahedron map)
    uint32_t m_normal;
    // R15G15B2_SNORM (octahedron map + tangent w)
    uint32_t m_tangent;
    // R16G16_UNORM
    uint32_t m_texcoord;
};

struct scene_mesh_vertex_joint_binding
{
    // R16G16B16A16_UINT (xy)
    uint32_t m_indices_xy;
    // R16G16B16A16_UINT (wz)
    uint32_t m_indices_wz;
    // R8G8B8A8_UNORM
    uint32_t m_weights;
};

struct scene_mesh_subset_data
{
    mcrt_vector<scene_mesh_vertex_position_binding> m_vertex_position_binding;

    mcrt_vector<scene_mesh_vertex_varying_binding> m_vertex_varying_binding;

    mcrt_vector<scene_mesh_vertex_joint_binding> m_vertex_joint_binding;

    mcrt_vector<uint32_t> m_indices;

    uint32_t m_max_index;

    // we assume that the meshes with the same materials have been merged

    mcrt_string m_normal_texture_image_uri;

    float m_normal_texture_scale;

    DirectX::XMFLOAT3 m_emissive_factor;

    mcrt_string m_emissive_texture_image_uri;

    DirectX::XMFLOAT3 m_base_color_factor;

    mcrt_string m_base_color_texture_image_uri;

    float m_metallic_factor;

    float m_roughness_factor;

    mcrt_string m_metallic_roughness_texture_image_uri;
};

struct scene_mesh_instance_data
{
    DirectX::XMFLOAT4X4 m_model_transform;

    scene_animation_skeleton m_animation_skeleton;
};

struct scene_mesh_data
{
    bool m_skinned;

    mcrt_vector<scene_mesh_subset_data> m_subsets;

    mcrt_vector<scene_mesh_instance_data> m_instances;
};

extern bool import_gltf_scene_asset(mcrt_vector<scene_mesh_data> &out_total_mesh_data, float frame_rate, brx_asset_import_input_stream_factory *input_stream_factory, char const *path);

// Khronos ANARI (Analytic Rendering Interface) API

// The importer will merge the meshes  
// (we should not merge emissive mesh with non-emissive mesh (due to RTXDI))
// all non-morph-target non-skin meshes into one mesh 
// all morph-target non-skin meshes into one mesh
// all non-morph-target skin meshes bound to the same skeleton into the same mesh (mesh count is the same as the skeleton)
// all morph target skin meshes bound to the same skeleton into the same mesh (mesh count is the same as the skeleton)

// The mesh subset and the material should techniquelly be separated
// But the mesh subsets, which share the same material, within the same mesh have been merged
// And there are rare cases where the mesh subsets from different meshes share the same material

class imported_scene_asset
{
public:
    virtual uint32_t mesh_count() = 0;
    virtual void get_mesh_metadata(
        uint32_t mesh_index,
        bool *out_support_morph_target,
        bool *out_support_skin,
        uint32_t *out_skin_index,
        uint32_t *out_subset_count) = 0;
    virtual void get_mesh_subset_metadata(
        uint32_t mesh_index,
        uint32_t mesh_subset_index,
        uint32_t *out_index_count,
        uint32_t *out_vertex_count,
        uint32_t *out_material_index) = 0;
    virtual void write_mesh_subset_data(
        uint32_t mesh_index,
        uint32_t mesh_subset_index,
        scene_mesh_vertex_position_binding *out_vertex_position_binding,
        scene_mesh_vertex_varying_binding *out_vertex_varying_binding,
        scene_mesh_vertex_joint_binding *out_vertex_joint_binding) = 0;
    virtual void mesh_subset_morph_target_count(
        uint32_t mesh_index,
        uint32_t mesh_subset_index) = 0;
    virtual void get_mesh_subset_morph_target_metadata(
        uint32_t mesh_index,
        uint32_t mesh_subset_index,
        uint32_t mesh_subset_morph_target_index,
        uint32_t *out_morph_target_name_length) = 0;
    virtual void write_mesh_subset_morph_target_data(
        uint32_t mesh_index,
        uint32_t mesh_subset_index,
        uint32_t mesh_subset_morph_target_index,
        scene_mesh_vertex_position_binding *out_vertex_position_binding,
        scene_mesh_vertex_varying_binding *out_vertex_varying_binding,
        scene_mesh_vertex_joint_binding *out_vertex_joint_binding,
        char *out_morph_target_name) = 0;
    virtual uint32_t skin_count() = 0;
    virtual uint32_t get_skin_metadata(
        uint32_t skin_index,
        uint32_t *out_skin_joint_count) = 0;
    virtual void write_skin_data(
        uint32_t skin_index,
        uint32_t *out_skin_joint_parent_indices) = 0;
    virtual void get_skin_joint_metadata(
        uint32_t skin_index,
        uint32_t skin_joint_index,
        uint32_t *out_skin_joint_name_length) = 0;
    virtual void write_skin_joint_data(
        uint32_t skin_index,
        uint32_t skin_joint_index,
        char *out_skin_joint_name) = 0;
    virtual uint32_t material_count() = 0;
    virtual void get_material_metadata(
        uint32_t material_index,
        float *out_normal_map_scale,
        uint32_t *out_normal_map_image_path_length,
        float out_emissive_factor[3],
        uint32_t *out_emissive_image_path_length,
        float out_base_color_factor[3],
        uint32_t *out_base_color_image_path_length,
        float *out_metallic_factor,
        float *out_roughness_factor,
        uint32_t *out_metallic_roughness_image_path_length) = 0;
    virtual void get_material_metadata(
        uint32_t material_index,
        char *out_normal_map_image_path,
        char *out_emissive_image_path,
        char *out_base_color_image_path,
        char *out_metallic_roughness_image_path) = 0;
    virtual void destory() = 0;
};

// class imported_scene_asset

class imported_animation_asset
{
public:
    virtual uint32_t morph_count() = 0;
    virtual void get_morph_metadata(
        uint32_t *out_morph_target_count
    ) = 0;
};

extern bool import_pmx_mesh_asset(mcrt_vector<scene_mesh_data> &out_total_mesh_data, brx_asset_import_input_stream_factory *input_stream_factory, char const *path);

#endif