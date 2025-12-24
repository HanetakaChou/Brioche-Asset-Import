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

#include "internal_import_vrm_model.h"
#include "internal_mmd_name.h"
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include "../../McRT-Malloc/include/mcrt_set.h"
#include "../../McRT-Malloc/include/mcrt_map.h"
#include "../../McRT-Malloc/include/mcrt_unordered_map.h"

#if defined(__GNUC__)
// GCC or CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
// MSVC or CLANG-CL
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#else
#error Unknown Compiler
#endif
#include "../thirdparty/DirectXMesh/DirectXMesh/DirectXMesh.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <cassert>
#ifndef NDEBUG
#include <iostream>
#endif

#if defined(__GNUC__)
// GCC or CLANG
#define internal_unlikely(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
#if defined(__clang__)
// CLANG-CL
#define internal_unlikely(x) __builtin_expect(!!(x), 0)
#else
// MSVC
#define internal_unlikely(x) (!!(x))
#endif
#else
#error Unknown Compiler
#endif

#include "../thirdparty/cgltf/cgltf.h"

struct internal_cgltf_read_file_context
{
    void const *m_data_base;
    size_t m_data_size;
    size_t m_offset;
};

struct internal_vrm_vertex_t
{
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_normal;
    DirectX::XMFLOAT4 m_tangent;
    DirectX::XMFLOAT2 m_texcoord;
};

struct internal_vrm_blending_vertex_t
{
    DirectX::XMUINT4 m_indices;
    DirectX::XMFLOAT4 m_weights;
};

struct internal_vrm_morph_target_vertex_t
{
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_normal;
    DirectX::XMFLOAT4 m_tangent;
    DirectX::XMFLOAT2 m_texcoord;
};

struct internal_vrm_mesh_section_t
{
    mcrt_vector<internal_vrm_vertex_t> m_vertices;

    mcrt_vector<internal_vrm_blending_vertex_t> m_blending_vertices;

    mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<internal_vrm_morph_target_vertex_t>> m_morph_targets_vertices;

    mcrt_vector<uint32_t> m_indices;
};

struct internal_vrm_collider_t
{
    DirectX::XMFLOAT3 m_offset;
    float m_radius;
};

struct internal_vrm_collider_group_t
{
    uint32_t m_node_index;
    mcrt_vector<internal_vrm_collider_t> m_colliders;
};

struct internal_vrm_bone_group_t
{
    float m_hit_radius;
    float m_drag_force;
    mcrt_vector<uint32_t> m_bones;
    mcrt_vector<uint32_t> m_collider_groups;
};

static cgltf_result internal_cgltf_read_file(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, const char *path, cgltf_size *size, void **in_gltf_data);

static void internal_cgltf_file_release(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, void *in_gltf_data, cgltf_size size);

static void *internal_cgltf_alloc(void *, cgltf_size size);

static void internal_cgltf_free(void *, void *ptr);

static inline DirectX::XMFLOAT3 internal_transform_translation(DirectX::XMFLOAT3 const &v, bool found_vrmc_vrm, bool found_vrm);

static inline DirectX::XMFLOAT3 internal_transform_normal(DirectX::XMFLOAT3 const &v, bool found_vrmc_vrm, bool found_vrm);

static inline DirectX::XMFLOAT4 internal_transform_tangent(DirectX::XMFLOAT4 const &v, bool found_vrmc_vrm, bool found_vrm);

static inline DirectX::XMFLOAT4 internal_transform_rotation(DirectX::XMFLOAT4 const &q, bool found_vrmc_vrm, bool found_vrm);

static inline void internal_parse_vrmc_vrm_morph_target_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_map<uint32_t, float>>> &out_vrmc_vrm_nodes_expressions_morph_target_binds);

static inline void internal_parse_vrmc_vrm_skeleton_joint_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_vrmc_vrm_human_bones);

static inline void internal_parse_vrm_morph_target_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_map<uint32_t, float>>> &out_vrm_meshes_blend_shapes_morph_target_binds);

static inline void internal_parse_vrm_skeleton_joint_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_vrm_human_bones);

static inline void internal_parse_vrm_spring_bones(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_vector<internal_vrm_collider_group_t> &out_vrm_collider_groups, mcrt_vector<internal_vrm_bone_group_t> &out_vrm_bone_groups);

static inline int internal_skip_json(jsmntok_t const *const tokens, int const token_index);

static inline void internal_import_animation_skeleton(cgltf_data const *in_gltf_data, mcrt_vector<DirectX::XMFLOAT4X4> &out_gltf_node_model_space_matrices, mcrt_vector<mcrt_set<uint32_t>> &out_gltf_meshes_instance_node_indices, mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_animation_skeleton_joint_names, mcrt_vector<uint32_t> &out_animation_skeleton_joint_parent_indices, mcrt_vector<uint32_t> &out_gltf_node_to_animation_skeleton_joint_map, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_local_space_matrices, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_model_space_matrices);

static inline void internal_import_ragdoll_physics(cgltf_data const *in_gltf_data, mcrt_vector<uint32_t> const &in_animation_skeleton_joint_parent_indices, mcrt_vector<uint32_t> const &in_gltf_node_to_animation_skeleton_joint_map, mcrt_vector<DirectX::XMFLOAT4X4> const &in_animation_skeleton_bind_pose_model_space, mcrt_vector<brx_asset_import_physics_rigid_body> &out_ragdoll_skeleton_rigid_bodies, mcrt_vector<brx_asset_import_physics_constraint> &out_ragdoll_skeleton_constraints, mcrt_vector<uint32_t> &out_ragdoll_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_animation_to_ragdoll_direct_mapping, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_ragdoll_to_animation_direct_mapping);

static inline void internal_import_surface(cgltf_data const *in_gltf_data, mcrt_vector<DirectX::XMFLOAT4X4> const &in_gltf_node_model_space_matrices, mcrt_vector<mcrt_set<uint32_t>> const &in_gltf_meshes_instance_node_indices, mcrt_vector<uint32_t> const &in_gltf_node_to_animation_skeleton_joint_map, mcrt_vector<std::pair<uint32_t, internal_vrm_mesh_section_t>> &out_materials_and_mesh_sections);

static inline void internal_import_material(cgltf_material const *const in_gltf_material, bool &out_is_double_sided, mcrt_vector<uint8_t> &out_emissive_image_url, uint32_t &out_emissive_factor, mcrt_vector<uint8_t> &out_normal_image_url, uint64_t &out_normal_scale, mcrt_vector<uint8_t> &out_base_color_image_url, uint32_t &out_base_color_factor, mcrt_vector<uint8_t> &out_metallic_roughness_image_url, uint32_t &out_metallic_roughness_factor);

static inline void internal_scene_depth_first_search_traverse(cgltf_data const *in_gltf_data, void (*pfn_user_callback)(cgltf_data const *in_gltf_data, cgltf_node const *in_current_gltf_node, cgltf_node const *in_parent_gltf_node, void *user_data), void *user_data);

extern bool internal_import_vrm_model(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_mesh_surface_group> &out_surface_groups)
{
    mcrt_vector<brx_asset_import_mesh_surface> surfaces;

    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> animation_skeleton_joint_names;
    mcrt_vector<uint32_t> animation_skeleton_joint_parent_indices;
    mcrt_vector<brx_asset_import_rigid_transform> animation_skeleton_joint_transforms_bind_pose_local_space;

    mcrt_vector<brx_asset_import_physics_rigid_body> ragdoll_skeleton_rigid_bodies;
    mcrt_vector<brx_asset_import_physics_constraint> ragdoll_skeleton_constraints;

    mcrt_vector<brx_asset_import_ragdoll_direct_mapping> animation_to_ragdoll_direct_mappings;
    mcrt_vector<brx_asset_import_ragdoll_direct_mapping> ragdoll_to_animation_direct_mappings;
    {
        cgltf_data *gltf_data = NULL;
        {
            internal_cgltf_read_file_context read_file_context = {data_base, data_size, 0U};

            cgltf_options options = {};
            options.type = cgltf_file_type_glb;
            options.memory.alloc_func = internal_cgltf_alloc;
            options.memory.free_func = internal_cgltf_free;
            options.file.read = internal_cgltf_read_file;
            options.file.release = internal_cgltf_file_release;
            options.file.user_data = &read_file_context;

            cgltf_result result_parse_file = cgltf_parse_file(&options, NULL, &gltf_data);
            if (internal_unlikely(cgltf_result_success != result_parse_file))
            {
                return false;
            }

            cgltf_result result_load_buffers = cgltf_load_buffers(&options, gltf_data, NULL);
            if (internal_unlikely(cgltf_result_success != result_load_buffers))
            {
                cgltf_free(gltf_data);
                return false;
            }
        }

        mcrt_vector<DirectX::XMFLOAT4X4> gltf_node_model_space_matrices;
        mcrt_vector<mcrt_set<uint32_t>> gltf_meshes_instance_node_indices;
        mcrt_vector<uint32_t> gltf_node_to_animation_skeleton_joint_map;
        mcrt_vector<DirectX::XMFLOAT4X4> animation_skeleton_bind_pose_local_space_matrices;
        mcrt_vector<DirectX::XMFLOAT4X4> animation_skeleton_bind_pose_model_space_matrices;
        internal_import_animation_skeleton(gltf_data, gltf_node_model_space_matrices, gltf_meshes_instance_node_indices, animation_skeleton_joint_names, animation_skeleton_joint_parent_indices, gltf_node_to_animation_skeleton_joint_map, animation_skeleton_bind_pose_local_space_matrices, animation_skeleton_bind_pose_model_space_matrices);

        assert(animation_skeleton_joint_transforms_bind_pose_local_space.empty());
        {
            uint32_t const animation_skeleton_joint_count = animation_skeleton_bind_pose_local_space_matrices.size();

            animation_skeleton_joint_transforms_bind_pose_local_space.resize(animation_skeleton_joint_count);

            for (uint32_t animation_skeleton_joint_index = 0U; animation_skeleton_joint_index < animation_skeleton_joint_count; ++animation_skeleton_joint_index)
            {
                DirectX::XMVECTOR simd_animation_skeleton_joint_transform_bind_pose_local_space_translation;
                DirectX::XMVECTOR simd_animation_skeleton_joint_transform_bind_pose_local_space_scale;
                DirectX::XMVECTOR simd_animation_skeleton_joint_transform_bind_pose_local_space_rotation;
                bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&simd_animation_skeleton_joint_transform_bind_pose_local_space_scale, &simd_animation_skeleton_joint_transform_bind_pose_local_space_rotation, &simd_animation_skeleton_joint_transform_bind_pose_local_space_translation, DirectX::XMLoadFloat4x4(&animation_skeleton_bind_pose_local_space_matrices[animation_skeleton_joint_index]));
                assert(directx_xm_matrix_decompose);

                constexpr float const INTERNAL_SCALE_EPSILON = 9E-5F;
                assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(simd_animation_skeleton_joint_transform_bind_pose_local_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));

                DirectX::XMFLOAT3 animation_skeleton_joint_transform_bind_pose_local_space_translation;
                DirectX::XMStoreFloat3(&animation_skeleton_joint_transform_bind_pose_local_space_translation, simd_animation_skeleton_joint_transform_bind_pose_local_space_translation);

                DirectX::XMFLOAT4 animation_skeleton_joint_transform_bind_pose_local_space_rotation;
                DirectX::XMStoreFloat4(&animation_skeleton_joint_transform_bind_pose_local_space_rotation, simd_animation_skeleton_joint_transform_bind_pose_local_space_rotation);

                animation_skeleton_joint_transforms_bind_pose_local_space[animation_skeleton_joint_index] = brx_asset_import_rigid_transform{{animation_skeleton_joint_transform_bind_pose_local_space_rotation.x, animation_skeleton_joint_transform_bind_pose_local_space_rotation.y, animation_skeleton_joint_transform_bind_pose_local_space_rotation.z, animation_skeleton_joint_transform_bind_pose_local_space_rotation.w}, {animation_skeleton_joint_transform_bind_pose_local_space_translation.x, animation_skeleton_joint_transform_bind_pose_local_space_translation.y, animation_skeleton_joint_transform_bind_pose_local_space_translation.z}};
            }
        }

        mcrt_vector<uint32_t> ragdoll_skeleton_joint_parent_indices;
        internal_import_ragdoll_physics(gltf_data, animation_skeleton_joint_parent_indices, gltf_node_to_animation_skeleton_joint_map, animation_skeleton_bind_pose_model_space_matrices, ragdoll_skeleton_rigid_bodies, ragdoll_skeleton_constraints, ragdoll_skeleton_joint_parent_indices, animation_to_ragdoll_direct_mappings, ragdoll_to_animation_direct_mappings);

        mcrt_vector<std::pair<uint32_t, internal_vrm_mesh_section_t>> materials_and_mesh_sections;
        internal_import_surface(gltf_data, gltf_node_model_space_matrices, gltf_meshes_instance_node_indices, gltf_node_to_animation_skeleton_joint_map, materials_and_mesh_sections);

        {
            surfaces.reserve(materials_and_mesh_sections.size());

            for (auto const &material_and_mesh_section : materials_and_mesh_sections)
            {
                mcrt_vector<brx_asset_import_surface_vertex_position> vertex_positions;
                mcrt_vector<brx_asset_import_surface_vertex_varying> vertex_varyings;
                mcrt_vector<brx_asset_import_surface_vertex_blending> vertex_blendings;
                mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> mesh_section_morph_target_names;
                mcrt_vector<mcrt_vector<brx_asset_import_surface_vertex_position>> morph_targets_vertex_positions;
                mcrt_vector<mcrt_vector<brx_asset_import_surface_vertex_varying>> morph_targets_vertex_varyings;
                mcrt_vector<uint32_t> indices;
                bool is_double_sided;
                mcrt_vector<uint8_t> emissive_image_url;
                brx_asset_import_vec3 emissive_factor;
                mcrt_vector<uint8_t> normal_image_url;
                float normal_scale;
                mcrt_vector<uint8_t> base_color_image_url;
                brx_asset_import_vec4 base_color_factor;
                mcrt_vector<uint8_t> metallic_roughness_image_url;
                float metallic_factor;
                float roughness_factor;
                {
                    auto const &mesh_section = material_and_mesh_section.second;

                    uint32_t const vertex_count = mesh_section.m_vertices.size();

                    assert(vertex_positions.empty());
                    vertex_positions.resize(vertex_count);

                    assert(vertex_varyings.empty());
                    vertex_varyings.resize(vertex_count);

                    for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
                    {
                        vertex_positions[vertex_index].m_position[0] = mesh_section.m_vertices[vertex_index].m_position.x;
                        vertex_positions[vertex_index].m_position[1] = mesh_section.m_vertices[vertex_index].m_position.y;
                        vertex_positions[vertex_index].m_position[2] = mesh_section.m_vertices[vertex_index].m_position.z;

                        vertex_varyings[vertex_index].m_normal[0] = mesh_section.m_vertices[vertex_index].m_normal.x;
                        vertex_varyings[vertex_index].m_normal[1] = mesh_section.m_vertices[vertex_index].m_normal.y;
                        vertex_varyings[vertex_index].m_normal[2] = mesh_section.m_vertices[vertex_index].m_normal.z;

                        vertex_varyings[vertex_index].m_tangent[0] = mesh_section.m_vertices[vertex_index].m_tangent.x;
                        vertex_varyings[vertex_index].m_tangent[1] = mesh_section.m_vertices[vertex_index].m_tangent.y;
                        vertex_varyings[vertex_index].m_tangent[2] = mesh_section.m_vertices[vertex_index].m_tangent.z;
                        vertex_varyings[vertex_index].m_tangent[3] = mesh_section.m_vertices[vertex_index].m_tangent.w;

                        vertex_varyings[vertex_index].m_texcoord[0] = mesh_section.m_vertices[vertex_index].m_texcoord.x;
                        vertex_varyings[vertex_index].m_texcoord[1] = mesh_section.m_vertices[vertex_index].m_texcoord.y;
                    }

                    if (!mesh_section.m_blending_vertices.empty())
                    {
                        assert(mesh_section.m_blending_vertices.size() == vertex_count);

                        assert(vertex_blendings.empty());
                        vertex_blendings.resize(vertex_count);

                        for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
                        {
                            vertex_blendings[vertex_index].m_indices[0] = mesh_section.m_blending_vertices[vertex_index].m_indices.x;
                            vertex_blendings[vertex_index].m_indices[1] = mesh_section.m_blending_vertices[vertex_index].m_indices.y;
                            vertex_blendings[vertex_index].m_indices[2] = mesh_section.m_blending_vertices[vertex_index].m_indices.z;
                            vertex_blendings[vertex_index].m_indices[3] = mesh_section.m_blending_vertices[vertex_index].m_indices.w;

                            vertex_blendings[vertex_index].m_weights[0] = mesh_section.m_blending_vertices[vertex_index].m_weights.x;
                            vertex_blendings[vertex_index].m_weights[1] = mesh_section.m_blending_vertices[vertex_index].m_weights.y;
                            vertex_blendings[vertex_index].m_weights[2] = mesh_section.m_blending_vertices[vertex_index].m_weights.z;
                            vertex_blendings[vertex_index].m_weights[3] = mesh_section.m_blending_vertices[vertex_index].m_weights.w;
                        }
                    }
                    else
                    {
                        assert(vertex_blendings.empty());
                    }

                    if (!mesh_section.m_morph_targets_vertices.empty())
                    {
                        uint32_t const morph_target_count = mesh_section.m_morph_targets_vertices.size();

                        mesh_section_morph_target_names.reserve(morph_target_count);

                        morph_targets_vertex_positions.reserve(morph_target_count);

                        morph_targets_vertex_varyings.reserve(morph_target_count);

                        for (auto const &morph_target_name_and_vertices : mesh_section.m_morph_targets_vertices)
                        {
                            BRX_ASSET_IMPORT_MORPH_TARGET_NAME const morph_target_name = morph_target_name_and_vertices.first;

                            mesh_section_morph_target_names.push_back(morph_target_name);

                            auto const &morph_target_vertices = morph_target_name_and_vertices.second;
                            assert(morph_target_vertices.size() == vertex_count);

                            mcrt_vector<brx_asset_import_surface_vertex_position> morph_target_vertex_positions(static_cast<size_t>(vertex_count));

                            mcrt_vector<brx_asset_import_surface_vertex_varying> morph_target_vertex_varyings(static_cast<size_t>(vertex_count));
                            ;

                            for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
                            {
                                morph_target_vertex_positions[vertex_index].m_position[0] = morph_target_vertices[vertex_index].m_position.x;
                                morph_target_vertex_positions[vertex_index].m_position[1] = morph_target_vertices[vertex_index].m_position.y;
                                morph_target_vertex_positions[vertex_index].m_position[2] = morph_target_vertices[vertex_index].m_position.z;

                                morph_target_vertex_varyings[vertex_index].m_normal[0] = morph_target_vertices[vertex_index].m_normal.x;
                                morph_target_vertex_varyings[vertex_index].m_normal[1] = morph_target_vertices[vertex_index].m_normal.y;
                                morph_target_vertex_varyings[vertex_index].m_normal[2] = morph_target_vertices[vertex_index].m_normal.z;

                                morph_target_vertex_varyings[vertex_index].m_tangent[0] = morph_target_vertices[vertex_index].m_tangent.x;
                                morph_target_vertex_varyings[vertex_index].m_tangent[1] = morph_target_vertices[vertex_index].m_tangent.y;
                                morph_target_vertex_varyings[vertex_index].m_tangent[2] = morph_target_vertices[vertex_index].m_tangent.z;
                                morph_target_vertex_varyings[vertex_index].m_tangent[3] = morph_target_vertices[vertex_index].m_tangent.w;

                                morph_target_vertex_varyings[vertex_index].m_texcoord[0] = morph_target_vertices[vertex_index].m_texcoord.x;
                                morph_target_vertex_varyings[vertex_index].m_texcoord[1] = morph_target_vertices[vertex_index].m_texcoord.y;
                            }

                            morph_targets_vertex_positions.push_back(std::move(morph_target_vertex_positions));

                            morph_targets_vertex_varyings.push_back(std::move(morph_target_vertex_varyings));
                        }

                        assert(mesh_section_morph_target_names.size() == morph_target_count);
                        assert(morph_targets_vertex_positions.size() == morph_target_count);
                        assert(morph_targets_vertex_varyings.size() == morph_target_count);
                    }
                    else
                    {
                        assert(mesh_section_morph_target_names.empty());
                        assert(morph_targets_vertex_positions.empty());
                        assert(morph_targets_vertex_varyings.empty());
                    }

                    uint32_t const index_count = mesh_section.m_indices.size();

                    assert(indices.empty());
                    indices.resize(index_count);

                    for (uint32_t index_index = 0U; index_index < index_count; ++index_index)
                    {
                        indices[index_index] = mesh_section.m_indices[index_index];
                    }

                    uint32_t const gltf_material_index = material_and_mesh_section.first;
                    assert(gltf_material_index < gltf_data->materials_count);

                    cgltf_material const *const gltf_material = gltf_data->materials + gltf_material_index;

                    uint32_t packed_emissive_factor_uint32;
                    uint64_t packed_normal_scale_uint64;
                    uint32_t packed_base_color_factor_uint32;
                    uint32_t packed_metallic_roughness_factor_uint32;
                    internal_import_material(gltf_material, is_double_sided, emissive_image_url, packed_emissive_factor_uint32, normal_image_url, packed_normal_scale_uint64, base_color_image_url, packed_base_color_factor_uint32, metallic_roughness_image_url, packed_metallic_roughness_factor_uint32);

                    DirectX::XMFLOAT3 unpacked_emissive_factor;
                    DirectX::PackedVector::XMUBYTEN4 packed_emissive_factor(packed_emissive_factor_uint32);
                    DirectX::XMStoreFloat3(&unpacked_emissive_factor, DirectX::PackedVector::XMLoadUByteN4(&packed_emissive_factor));

                    emissive_factor.m_x = unpacked_emissive_factor.x;
                    emissive_factor.m_y = unpacked_emissive_factor.y;
                    emissive_factor.m_z = unpacked_emissive_factor.z;

                    DirectX::XMFLOAT2 unpacked_normal_scale;
                    DirectX::PackedVector::XMUSHORTN4 packed_normal_scale(packed_normal_scale_uint64);
                    DirectX::XMStoreFloat2(&unpacked_normal_scale, DirectX::PackedVector::XMLoadUShortN4(&packed_normal_scale));

                    normal_scale = unpacked_normal_scale.x;

                    DirectX::XMFLOAT4 unpacked_base_color_factor;
                    DirectX::PackedVector::XMUBYTEN4 packed_base_color_factor(packed_base_color_factor_uint32);
                    DirectX::XMStoreFloat4(&unpacked_base_color_factor, DirectX::PackedVector::XMLoadUByteN4(&packed_base_color_factor));

                    base_color_factor.m_x = unpacked_base_color_factor.x;
                    base_color_factor.m_y = unpacked_base_color_factor.y;
                    base_color_factor.m_z = unpacked_base_color_factor.z;
                    base_color_factor.m_w = unpacked_base_color_factor.w;

                    DirectX::XMFLOAT3 unpacked_metallic_roughness_factor;
                    DirectX::PackedVector::XMUBYTEN4 packed_metallic_roughness_factor(packed_metallic_roughness_factor_uint32);
                    DirectX::XMStoreFloat3(&unpacked_metallic_roughness_factor, DirectX::PackedVector::XMLoadUByteN4(&packed_metallic_roughness_factor));

                    roughness_factor = unpacked_metallic_roughness_factor.y;
                    metallic_factor = unpacked_metallic_roughness_factor.z;
                }

                surfaces.emplace_back(std::move(vertex_positions), std::move(vertex_varyings), std::move(vertex_blendings), std::move(mesh_section_morph_target_names), std::move(morph_targets_vertex_positions), std::move(morph_targets_vertex_varyings), std::move(indices), is_double_sided, std::move(emissive_image_url), emissive_factor, std::move(normal_image_url), normal_scale, std::move(base_color_image_url), base_color_factor, std::move(metallic_roughness_image_url), metallic_factor, roughness_factor);
            }

            assert(surfaces.size() == materials_and_mesh_sections.size());
        }

        cgltf_free(gltf_data);
    }

    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> animation_skeleton_joint_constraint_names;
    mcrt_vector<brx_asset_import_skeleton_joint_constraint> animation_skeleton_joint_constraints;
    mcrt_vector<mcrt_vector<uint32_t>> animation_skeleton_joint_constraints_storages;

    assert(out_surface_groups.empty());
    out_surface_groups = {};

    out_surface_groups.reserve(1U);

    out_surface_groups.emplace_back(std::move(surfaces), std::move(animation_skeleton_joint_names), std::move(animation_skeleton_joint_parent_indices), std::move(animation_skeleton_joint_transforms_bind_pose_local_space), std::move(animation_skeleton_joint_constraint_names), std::move(animation_skeleton_joint_constraints), std::move(animation_skeleton_joint_constraints_storages), std::move(ragdoll_skeleton_rigid_bodies), std::move(ragdoll_skeleton_constraints), std::move(animation_to_ragdoll_direct_mappings), std::move(ragdoll_to_animation_direct_mappings));

    return true;
}

static inline void internal_parse_vrmc_vrm_morph_target_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_map<uint32_t, float>>> &out_vrmc_vrm_nodes_expressions_morph_target_binds)
{
    assert(out_vrmc_vrm_nodes_expressions_morph_target_binds.empty());

    mcrt_unordered_map<mcrt_string, mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>> vrmc_vrm_expression_name_strings;
    {
        // https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.expressions.schema.json
        vrmc_vrm_expression_name_strings["happy"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_HAPPY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_HAPPY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY};
        vrmc_vrm_expression_name_strings["angry"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_ANGRY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY};
        vrmc_vrm_expression_name_strings["sad"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SAD, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SAD, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SAD};
        vrmc_vrm_expression_name_strings["surprised"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SURPRISED, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SURPRISED, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED};
        vrmc_vrm_expression_name_strings["aa"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A};
        vrmc_vrm_expression_name_strings["ih"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_I};
        vrmc_vrm_expression_name_strings["ou"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_U};
        vrmc_vrm_expression_name_strings["ee"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_E};
        vrmc_vrm_expression_name_strings["oh"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_O};
        vrmc_vrm_expression_name_strings["blink"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK};
        vrmc_vrm_expression_name_strings["blinkLeft"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L};
        vrmc_vrm_expression_name_strings["blinkRight"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R};
    }

    int token_index = 0;
    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
    {
        assert(false);
        return;
    }

    int const vrmc_vrm_property_count = tokens[token_index].size;
    ++token_index;
    assert(token_index <= token_count);

    for (int vrmc_vrm_property_index = 0; vrmc_vrm_property_index < vrmc_vrm_property_count; ++vrmc_vrm_property_index)
    {
        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
        {
            assert(false);
            return;
        }

        char const *const vrmc_vrm_property_name_string = json + tokens[token_index].start;
        size_t const vrmc_vrm_property_name_length = tokens[token_index].end - tokens[token_index].start;
        ++token_index;
        assert(token_index <= token_count);

        if ((11U == vrmc_vrm_property_name_length) && (0 == std::strncmp("expressions", vrmc_vrm_property_name_string, 11U)))
        {
            if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
            {
                assert(false);
                return;
            }

            int const vrmc_vrm_expression_property_count = tokens[token_index].size;
            ++token_index;
            assert(token_index <= token_count);

            for (int vrmc_vrm_expression_property_index = 0; vrmc_vrm_expression_property_index < vrmc_vrm_expression_property_count; ++vrmc_vrm_expression_property_index)
            {
                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                {
                    assert(false);
                    return;
                }

                char const *const vrmc_vrm_expression_property_name_string = json + tokens[token_index].start;
                size_t const vrmc_vrm_expression_property_name_length = tokens[token_index].end - tokens[token_index].start;
                ++token_index;
                assert(token_index <= token_count);

                if ((6U == vrmc_vrm_expression_property_name_length) && (0 == std::strncmp("preset", vrmc_vrm_expression_property_name_string, 6U)))
                {
                    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                    {
                        assert(false);
                        return;
                    }

                    int const vrmc_vrm_expression_preset_property_count = tokens[token_index].size;
                    ++token_index;
                    assert(token_index <= token_count);

                    for (int vrmc_vrm_expression_preset_property_index = 0; vrmc_vrm_expression_preset_property_index < vrmc_vrm_expression_preset_property_count; ++vrmc_vrm_expression_preset_property_index)
                    {
                        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                        {
                            assert(false);
                            return;
                        }

                        char const *const vrmc_vrm_expression_preset_property_name_string = json + tokens[token_index].start;
                        size_t const vrmc_vrm_expression_preset_property_name_length = tokens[token_index].end - tokens[token_index].start;
                        ++token_index;
                        assert(token_index <= token_count);

                        mcrt_string vrmc_vrm_expression_preset_property_name(vrmc_vrm_expression_preset_property_name_string, vrmc_vrm_expression_preset_property_name_length);

                        int const vrmc_vrm_expression_preset_property_value_property_count = tokens[token_index].size;
                        ++token_index;
                        assert(token_index <= token_count);

                        for (int vrmc_vrm_expression_preset_property_value_property_index = 0; vrmc_vrm_expression_preset_property_value_property_index < vrmc_vrm_expression_preset_property_value_property_count; ++vrmc_vrm_expression_preset_property_value_property_index)
                        {
                            if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                            {
                                assert(false);
                                return;
                            }

                            char const *const vrmc_vrm_expression_preset_property_value_property_name_string = json + tokens[token_index].start;
                            size_t const vrmc_vrm_expression_preset_property_value_property_name_length = tokens[token_index].end - tokens[token_index].start;
                            ++token_index;
                            assert(token_index <= token_count);

                            if ((16U == vrmc_vrm_expression_preset_property_value_property_name_length) && (0 == std::strncmp("morphTargetBinds", vrmc_vrm_expression_preset_property_value_property_name_string, 16U)))
                            {
                                if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                int const vrmc_vrm_expression_preset_morph_target_bind_element_count = tokens[token_index].size;
                                ++token_index;
                                assert(token_index <= token_count);

                                for (int vrmc_vrm_expression_preset_morph_target_bind_element_index = 0; vrmc_vrm_expression_preset_morph_target_bind_element_index < vrmc_vrm_expression_preset_morph_target_bind_element_count; ++vrmc_vrm_expression_preset_morph_target_bind_element_index)
                                {
                                    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                                    {
                                        assert(false);
                                        return;
                                    }

                                    int vrmc_vrm_expression_preset_morph_target_bind_element_property_count = tokens[token_index].size;
                                    ++token_index;
                                    assert(token_index <= token_count);

                                    bool found_vrmc_vrm_expression_preset_morph_target_bind_index = false;
                                    unsigned long long vrmc_vrm_expression_preset_morph_target_bind_index = -1;

                                    bool found_vrmc_vrm_expression_preset_morph_target_bind_node = false;
                                    unsigned long long vrmc_vrm_expression_preset_morph_target_bind_node = -1;

                                    bool found_vrmc_vrm_expression_preset_morph_target_bind_weight = false;
                                    float vrmc_vrm_expression_preset_morph_target_bind_weight = -1.0F;

                                    for (int vrmc_vrm_expression_preset_morph_target_bind_element_property_index = 0; vrmc_vrm_expression_preset_morph_target_bind_element_property_index < vrmc_vrm_expression_preset_morph_target_bind_element_property_count; ++vrmc_vrm_expression_preset_morph_target_bind_element_property_index)
                                    {
                                        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                        {
                                            assert(false);
                                            return;
                                        }

                                        char const *const vrmc_vrm_expression_preset_morph_target_bind_element_property_name_string = json + tokens[token_index].start;
                                        size_t const vrmc_vrm_expression_preset_morph_target_bind_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                                        ++token_index;
                                        assert(token_index <= token_count);

                                        if ((5U == vrmc_vrm_expression_preset_morph_target_bind_element_property_name_length) && (0 == std::strncmp("index", vrmc_vrm_expression_preset_morph_target_bind_element_property_name_string, 5U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrmc_vrm_expression_preset_morph_target_bind_element_property_value_string = json + tokens[token_index].start;
                                            size_t const vrmc_vrm_expression_preset_morph_target_bind_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrmc_vrm_expression_preset_morph_target_bind_element_property_value(vrmc_vrm_expression_preset_morph_target_bind_element_property_value_string, vrmc_vrm_expression_preset_morph_target_bind_element_property_value_length);

                                            assert(!found_vrmc_vrm_expression_preset_morph_target_bind_index);
                                            found_vrmc_vrm_expression_preset_morph_target_bind_index = true;

                                            assert(-1 == vrmc_vrm_expression_preset_morph_target_bind_index);
                                            vrmc_vrm_expression_preset_morph_target_bind_index = std::strtoull(vrmc_vrm_expression_preset_morph_target_bind_element_property_value.c_str(), NULL, 10);
                                        }
                                        else if ((4U == vrmc_vrm_expression_preset_morph_target_bind_element_property_name_length) && (0 == std::strncmp("node", vrmc_vrm_expression_preset_morph_target_bind_element_property_name_string, 4U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrmc_vrm_expression_preset_morph_target_bind_element_property_value_string = json + tokens[token_index].start;
                                            size_t const vrmc_vrm_expression_preset_morph_target_bind_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrmc_vrm_expression_preset_morph_target_bind_element_property_value(vrmc_vrm_expression_preset_morph_target_bind_element_property_value_string, vrmc_vrm_expression_preset_morph_target_bind_element_property_value_length);

                                            assert(!found_vrmc_vrm_expression_preset_morph_target_bind_node);
                                            found_vrmc_vrm_expression_preset_morph_target_bind_node = true;

                                            assert(-1 == vrmc_vrm_expression_preset_morph_target_bind_node);
                                            vrmc_vrm_expression_preset_morph_target_bind_node = std::strtoull(vrmc_vrm_expression_preset_morph_target_bind_element_property_value.c_str(), NULL, 10);
                                        }
                                        else if ((6U == vrmc_vrm_expression_preset_morph_target_bind_element_property_name_length) && (0 == std::strncmp("weight", vrmc_vrm_expression_preset_morph_target_bind_element_property_name_string, 6U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrmc_vrm_expression_preset_morph_target_bind_element_property_value_string = json + tokens[token_index].start;
                                            size_t const vrmc_vrm_expression_preset_morph_target_bind_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrmc_vrm_expression_preset_morph_target_bind_element_property_value(vrmc_vrm_expression_preset_morph_target_bind_element_property_value_string, vrmc_vrm_expression_preset_morph_target_bind_element_property_value_length);

                                            assert(!found_vrmc_vrm_expression_preset_morph_target_bind_weight);
                                            found_vrmc_vrm_expression_preset_morph_target_bind_weight = true;

                                            assert(-1.0F == vrmc_vrm_expression_preset_morph_target_bind_weight);
                                            vrmc_vrm_expression_preset_morph_target_bind_weight = std::strtof(vrmc_vrm_expression_preset_morph_target_bind_element_property_value.c_str(), NULL);
                                        }
                                        else
                                        {
                                            assert(false);
                                            token_index = internal_skip_json(tokens, token_index);
                                            assert(token_index <= token_count);
                                        }
                                    }

                                    if (found_vrmc_vrm_expression_preset_morph_target_bind_index && found_vrmc_vrm_expression_preset_morph_target_bind_node && found_vrmc_vrm_expression_preset_morph_target_bind_weight)
                                    {
                                        auto found_vrmc_vrm_expression_names = vrmc_vrm_expression_name_strings.find(vrmc_vrm_expression_preset_property_name);
                                        if (vrmc_vrm_expression_name_strings.end() != found_vrmc_vrm_expression_names)
                                        {
                                            assert(vrmc_vrm_expression_preset_morph_target_bind_index < static_cast<unsigned long long>(UINT32_MAX));
                                            uint32_t const vrmc_vrm_expression_preset_morph_target_bind_index_uint32 = static_cast<uint32_t>(vrmc_vrm_expression_preset_morph_target_bind_index);

                                            assert(vrmc_vrm_expression_preset_morph_target_bind_node < static_cast<unsigned long long>(UINT32_MAX));
                                            uint32_t const vrmc_vrm_expression_preset_morph_target_bind_node_index_uint32 = static_cast<uint32_t>(vrmc_vrm_expression_preset_morph_target_bind_node);

                                            auto &vrmc_vrm_node_expressions_morph_target_binds = out_vrmc_vrm_nodes_expressions_morph_target_binds[vrmc_vrm_expression_preset_morph_target_bind_node_index_uint32];

                                            for (auto const &vrmc_vrm_expression_name : found_vrmc_vrm_expression_names->second)
                                            {
                                                assert(found_vrmc_vrm_expression_names->second.size() >= 1U);
                                                float const weight = vrmc_vrm_expression_preset_morph_target_bind_weight / static_cast<float>(found_vrmc_vrm_expression_names->second.size());

                                                auto &vrmc_vrm_node_expression_morph_target_binds = vrmc_vrm_node_expressions_morph_target_binds[vrmc_vrm_expression_name];

                                                auto found_vrmc_vrm_node_expression_morph_target_bind = vrmc_vrm_node_expression_morph_target_binds.find(vrmc_vrm_expression_preset_morph_target_bind_index_uint32);
                                                if (vrmc_vrm_node_expression_morph_target_binds.end() == found_vrmc_vrm_node_expression_morph_target_bind)
                                                {
                                                    vrmc_vrm_node_expression_morph_target_binds.emplace_hint(found_vrmc_vrm_node_expression_morph_target_bind, vrmc_vrm_expression_preset_morph_target_bind_index_uint32, weight);
                                                }
                                                else
                                                {
                                                    found_vrmc_vrm_node_expression_morph_target_bind->second += weight;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            assert(false);
                                        }
                                    }
                                    else
                                    {
                                        assert(false);
                                    }
                                }
                            }
                            else
                            {
                                token_index = internal_skip_json(tokens, token_index);
                                assert(token_index <= token_count);
                            }
                        }
                    }
                }
                else
                {
                    token_index = internal_skip_json(tokens, token_index);
                    assert(token_index <= token_count);
                }
            }
        }
        else
        {
            token_index = internal_skip_json(tokens, token_index);
            assert(token_index <= token_count);
        }

        if (internal_unlikely(token_index < 0))
        {
            assert(false);
            break;
        }
    }

    assert(token_index == token_count);
}

static inline void internal_parse_vrmc_vrm_skeleton_joint_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_vrmc_vrm_human_bones)
{
    assert(out_vrmc_vrm_human_bones.empty());

    mcrt_unordered_map<mcrt_string, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> vrmc_vrm_skeleton_joint_name_strings;
    {
        // https://github.com/saturday06/VRM-Addon-for-Blender/blob/main/src/io_scene_vrm/common/human_bone_mapper/mmd_mapping.py
        // https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.humanoid.humanBones.schema.json
        vrmc_vrm_skeleton_joint_name_strings["hips"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CENTER;
        vrmc_vrm_skeleton_joint_name_strings["spine"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY;
        vrmc_vrm_skeleton_joint_name_strings["chest"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY_2;
        vrmc_vrm_skeleton_joint_name_strings["neck"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_NECK;
        vrmc_vrm_skeleton_joint_name_strings["head"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_HEAD;
        vrmc_vrm_skeleton_joint_name_strings["leftEye"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE;
        vrmc_vrm_skeleton_joint_name_strings["rightEye"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE;
        vrmc_vrm_skeleton_joint_name_strings["leftUpperLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LEG;
        vrmc_vrm_skeleton_joint_name_strings["leftLowerLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_KNEE;
        vrmc_vrm_skeleton_joint_name_strings["leftFoot"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE;
        vrmc_vrm_skeleton_joint_name_strings["leftToes"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP;
        vrmc_vrm_skeleton_joint_name_strings["rightUpperLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LEG;
        vrmc_vrm_skeleton_joint_name_strings["rightLowerLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_KNEE;
        vrmc_vrm_skeleton_joint_name_strings["rightFoot"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE;
        vrmc_vrm_skeleton_joint_name_strings["rightToes"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP;
        vrmc_vrm_skeleton_joint_name_strings["leftShoulder"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER;
        vrmc_vrm_skeleton_joint_name_strings["leftUpperArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM;
        vrmc_vrm_skeleton_joint_name_strings["leftLowerArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ELBOW;
        vrmc_vrm_skeleton_joint_name_strings["leftHand"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_WRIST;
        vrmc_vrm_skeleton_joint_name_strings["rightShoulder"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER;
        vrmc_vrm_skeleton_joint_name_strings["rightUpperArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM;
        vrmc_vrm_skeleton_joint_name_strings["rightLowerArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ELBOW;
        vrmc_vrm_skeleton_joint_name_strings["rightHand"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_WRIST;
        vrmc_vrm_skeleton_joint_name_strings["leftThumbMetacarpal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_0;
        vrmc_vrm_skeleton_joint_name_strings["leftThumbProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_1;
        vrmc_vrm_skeleton_joint_name_strings["leftThumbDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_2;
        vrmc_vrm_skeleton_joint_name_strings["leftIndexProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["leftIndexIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["leftIndexDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["leftMiddleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["leftMiddleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["leftMiddleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["leftRingProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["leftRingIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["leftRingDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["leftLittleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["leftLittleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["leftLittleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["rightThumbMetacarpal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_0;
        vrmc_vrm_skeleton_joint_name_strings["rightThumbProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_1;
        vrmc_vrm_skeleton_joint_name_strings["rightThumbDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_2;
        vrmc_vrm_skeleton_joint_name_strings["rightIndexProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["rightIndexIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["rightIndexDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["rightMiddleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["rightMiddleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["rightMiddleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["rightRingProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["rightRingIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["rightRingDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_3;
        vrmc_vrm_skeleton_joint_name_strings["rightLittleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_1;
        vrmc_vrm_skeleton_joint_name_strings["rightLittleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_2;
        vrmc_vrm_skeleton_joint_name_strings["rightLittleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_3;
    }

    int token_index = 0;
    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
    {
        assert(false);
        return;
    }

    int const vrmc_vrm_property_count = tokens[token_index].size;
    ++token_index;
    assert(token_index <= token_count);

    for (int vrmc_vrm_property_index = 0; vrmc_vrm_property_index < vrmc_vrm_property_count; ++vrmc_vrm_property_index)
    {
        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
        {
            assert(false);
            return;
        }

        char const *const vrmc_vrm_property_name_string = json + tokens[token_index].start;
        size_t const vrmc_vrm_property_name_length = tokens[token_index].end - tokens[token_index].start;
        ++token_index;
        assert(token_index <= token_count);

        if ((8U == vrmc_vrm_property_name_length) && (0 == std::strncmp("humanoid", vrmc_vrm_property_name_string, 8U)))
        {
            if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
            {
                assert(false);
                return;
            }

            int const vrmc_vrm_humanoid_property_count = tokens[token_index].size;
            ++token_index;
            assert(token_index <= token_count);

            for (int vrmc_vrm_humanoid_property_index = 0; vrmc_vrm_humanoid_property_index < vrmc_vrm_humanoid_property_count; ++vrmc_vrm_humanoid_property_index)
            {
                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                {
                    assert(false);
                    return;
                }

                char const *const vrmc_vrm_humanoid_property_name_string = json + tokens[token_index].start;
                size_t const vrmc_vrm_humanoid_property_name_length = tokens[token_index].end - tokens[token_index].start;
                ++token_index;
                assert(token_index <= token_count);

                if ((10U == vrmc_vrm_humanoid_property_name_length) && (0 == std::strncmp("humanBones", vrmc_vrm_humanoid_property_name_string, 10U)))
                {
                    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                    {
                        assert(false);
                        return;
                    }

                    int const vrmc_vrm_human_bone_property_count = tokens[token_index].size;
                    ++token_index;
                    assert(token_index <= token_count);

                    for (int vrmc_vrm_human_bone_property_index = 0; vrmc_vrm_human_bone_property_index < vrmc_vrm_human_bone_property_count; ++vrmc_vrm_human_bone_property_index)
                    {
                        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                        {
                            assert(false);
                            return;
                        }

                        char const *const vrmc_vrm_human_bone_property_name_string = json + tokens[token_index].start;
                        size_t const vrmc_vrm_human_bone_property_name_length = tokens[token_index].end - tokens[token_index].start;
                        ++token_index;
                        assert(token_index <= token_count);

                        mcrt_string vrmc_vrm_human_bone_property_name(vrmc_vrm_human_bone_property_name_string, vrmc_vrm_human_bone_property_name_length);

                        int const vrmc_vrm_human_bone_property_value_property_count = tokens[token_index].size;
                        ++token_index;
                        assert(token_index <= token_count);

                        bool found_vrmc_vrm_human_bone_node = false;
                        unsigned long long vrmc_vrm_human_bone_node = -1;

                        for (int vrmc_vrm_human_bone_property_value_property_index = 0; vrmc_vrm_human_bone_property_value_property_index < vrmc_vrm_human_bone_property_value_property_count; ++vrmc_vrm_human_bone_property_value_property_index)
                        {
                            if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                            {
                                assert(false);
                                return;
                            }

                            char const *const vrmc_vrm_human_bone_property_value_property_name_string = json + tokens[token_index].start;
                            size_t const vrmc_vrm_human_bone_property_value_property_name_length = tokens[token_index].end - tokens[token_index].start;
                            ++token_index;
                            assert(token_index <= token_count);

                            if ((4U == vrmc_vrm_human_bone_property_value_property_name_length) && (0 == std::strncmp("node", vrmc_vrm_human_bone_property_value_property_name_string, 4U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_human_bone_element_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_human_bone_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_human_bone_element_property_value(vrm_human_bone_element_property_value_string, vrm_human_bone_element_property_value_length);

                                assert(!found_vrmc_vrm_human_bone_node);
                                found_vrmc_vrm_human_bone_node = true;

                                assert(-1 == vrmc_vrm_human_bone_node);
                                vrmc_vrm_human_bone_node = std::strtoull(vrm_human_bone_element_property_value.c_str(), NULL, 10);
                            }
                            else
                            {
                                assert(false);
                                token_index = internal_skip_json(tokens, token_index);
                                assert(token_index <= token_count);
                            }
                        }

                        if (found_vrmc_vrm_human_bone_node)
                        {
                            auto found_vrmc_vrm_skeleton_joint_name = vrmc_vrm_skeleton_joint_name_strings.find(vrmc_vrm_human_bone_property_name);
                            if (vrmc_vrm_skeleton_joint_name_strings.end() != found_vrmc_vrm_skeleton_joint_name)
                            {
                                assert(vrmc_vrm_human_bone_node <= static_cast<unsigned long long>(UINT32_MAX));
                                uint32_t const vrmc_vrm_human_bone_node_index_uint32 = static_cast<uint32_t>(vrmc_vrm_human_bone_node);

                                assert(out_vrmc_vrm_human_bones.end() == out_vrmc_vrm_human_bones.find(vrmc_vrm_human_bone_node_index_uint32));
                                out_vrmc_vrm_human_bones[vrmc_vrm_human_bone_node_index_uint32] = found_vrmc_vrm_skeleton_joint_name->second;
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                }
                else
                {
                    token_index = internal_skip_json(tokens, token_index);
                    assert(token_index <= token_count);
                }
            }
        }
        else
        {
            token_index = internal_skip_json(tokens, token_index);
            assert(token_index <= token_count);
        }

        if (internal_unlikely(token_index < 0))
        {
            assert(false);
            break;
        }
    }

    assert(token_index == token_count);
}

static inline void internal_parse_vrm_morph_target_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_map<uint32_t, float>>> &out_vrm_meshes_blend_shapes_morph_target_binds)
{
    mcrt_unordered_map<mcrt_string, mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>> vrm_blend_shape_name_strings;
    {
        // https://github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.group.schema.json
        vrm_blend_shape_name_strings["a"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A};
        vrm_blend_shape_name_strings["i"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_I};
        vrm_blend_shape_name_strings["u"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_U};
        vrm_blend_shape_name_strings["e"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_E};
        vrm_blend_shape_name_strings["o"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_O};
        vrm_blend_shape_name_strings["blink"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK};
        vrm_blend_shape_name_strings["joy"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_HAPPY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_HAPPY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY};
        vrm_blend_shape_name_strings["angry"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_ANGRY, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY};
        vrm_blend_shape_name_strings["sorrow"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SAD, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SAD, BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SAD};
        vrm_blend_shape_name_strings["blink_l"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L};
        vrm_blend_shape_name_strings["blink_r"] = mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>{BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R};
    }

    int token_index = 0;
    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
    {
        assert(false);
        return;
    }

    int const vrm_property_count = tokens[token_index].size;
    ++token_index;
    assert(token_index <= token_count);

    for (int vrm_property_index = 0; vrm_property_index < vrm_property_count; ++vrm_property_index)
    {
        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
        {
            assert(false);
            return;
        }

        char const *const vrm_property_name_string = json + tokens[token_index].start;
        size_t const vrm_property_name_length = tokens[token_index].end - tokens[token_index].start;
        ++token_index;
        assert(token_index <= token_count);

        if ((16U == vrm_property_name_length) && (0 == std::strncmp("blendShapeMaster", vrm_property_name_string, 16U)))
        {
            if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
            {
                assert(false);
                return;
            }

            int const vrm_blend_shape_master_property_count = tokens[token_index].size;
            ++token_index;
            assert(token_index <= token_count);

            for (int vrm_blend_shape_master_property_index = 0; vrm_blend_shape_master_property_index < vrm_blend_shape_master_property_count; ++vrm_blend_shape_master_property_index)
            {
                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                {
                    assert(false);
                    return;
                }

                char const *const vrm_blend_shape_master_property_name_string = json + tokens[token_index].start;
                size_t const vrm_blend_shape_master_property_name_length = tokens[token_index].end - tokens[token_index].start;
                ++token_index;
                assert(token_index <= token_count);

                if ((16U == vrm_blend_shape_master_property_name_length) && (0 == std::strncmp("blendShapeGroups", vrm_blend_shape_master_property_name_string, 16U)))
                {
                    if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                    {
                        assert(false);
                        return;
                    }

                    int const vrm_blend_shape_group_element_count = tokens[token_index].size;
                    ++token_index;
                    assert(token_index <= token_count);

                    for (int vrm_blend_shape_group_element_index = 0; vrm_blend_shape_group_element_index < vrm_blend_shape_group_element_count; ++vrm_blend_shape_group_element_index)
                    {
                        struct internal_vrm_blend_shape_bind
                        {
                            uint32_t m_index;
                            uint32_t m_mesh;
                            float m_weight;
                        };

                        mcrt_vector<internal_vrm_blend_shape_bind> vrm_blend_shape_group_element_binds;

                        mcrt_string vrm_blend_shape_group_element_preset_name;

                        if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                        {
                            assert(false);
                            return;
                        }

                        int const vrm_blend_shape_group_element_property_count = tokens[token_index].size;
                        ++token_index;
                        assert(token_index <= token_count);

                        for (int vrm_blend_shape_group_element_property_index = 0; vrm_blend_shape_group_element_property_index < vrm_blend_shape_group_element_property_count; ++vrm_blend_shape_group_element_property_index)
                        {
                            if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                            {
                                assert(false);
                                return;
                            }

                            char const *const vrm_blend_shape_group_element_property_name_string = json + tokens[token_index].start;
                            size_t const vrm_blend_shape_group_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                            ++token_index;
                            assert(token_index <= token_count);

                            if ((5U == vrm_blend_shape_group_element_property_name_length) && (0 == std::strncmp("binds", vrm_blend_shape_group_element_property_name_string, 5U)))
                            {
                                if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                int const vrm_blend_shape_group_element_bind_element_count = tokens[token_index].size;
                                ++token_index;
                                assert(token_index <= token_count);

                                for (int vrm_blend_shape_group_element_bind_element_index = 0; vrm_blend_shape_group_element_bind_element_index < vrm_blend_shape_group_element_bind_element_count; ++vrm_blend_shape_group_element_bind_element_index)
                                {
                                    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                                    {
                                        assert(false);
                                        return;
                                    }

                                    int const vrm_blend_shape_group_element_bind_element_property_count = tokens[token_index].size;
                                    ++token_index;
                                    assert(token_index <= token_count);

                                    bool found_vrm_blend_shape_bind_index = false;
                                    unsigned long long vrm_blend_shape_bind_index = -1;

                                    bool found_vrm_blend_shape_bind_mesh = false;
                                    unsigned long long vrm_blend_shape_bind_mesh = -1;

                                    bool found_vrm_blend_shape_bind_weight = false;
                                    float vrm_blend_shape_bind_weight = -1.0F;

                                    for (int vrm_blend_shape_group_element_bind_element_property_index = 0; vrm_blend_shape_group_element_bind_element_property_index < vrm_blend_shape_group_element_bind_element_property_count; ++vrm_blend_shape_group_element_bind_element_property_index)
                                    {
                                        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                        {
                                            assert(false);
                                            return;
                                        }

                                        char const *const vrm_blend_shape_group_element_bind_element_property_name_string = json + tokens[token_index].start;
                                        size_t const vrm_blend_shape_group_element_bind_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                                        ++token_index;
                                        assert(token_index <= token_count);

                                        if ((5U == vrm_blend_shape_group_element_bind_element_property_name_length) && (0 == std::strncmp("index", vrm_blend_shape_group_element_bind_element_property_name_string, 5U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrm_blend_shape_group_element_bind_element_property_value_string = json + tokens[token_index].start;
                                            size_t const vrm_blend_shape_group_element_bind_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrm_blend_shape_group_element_bind_element_property_value(vrm_blend_shape_group_element_bind_element_property_value_string, vrm_blend_shape_group_element_bind_element_property_value_length);

                                            assert(!found_vrm_blend_shape_bind_index);
                                            found_vrm_blend_shape_bind_index = true;

                                            assert(-1 == vrm_blend_shape_bind_index);
                                            vrm_blend_shape_bind_index = std::strtoull(vrm_blend_shape_group_element_bind_element_property_value.c_str(), NULL, 10);
                                        }
                                        else if ((4U == vrm_blend_shape_group_element_bind_element_property_name_length) && (0 == std::strncmp("mesh", vrm_blend_shape_group_element_bind_element_property_name_string, 4U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrm_blend_shape_group_element_bind_element_property_value_string = json + tokens[token_index].start;
                                            size_t const vrm_blend_shape_group_element_bind_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrm_blend_shape_group_element_bind_element_property_value(vrm_blend_shape_group_element_bind_element_property_value_string, vrm_blend_shape_group_element_bind_element_property_value_length);

                                            assert(!found_vrm_blend_shape_bind_mesh);
                                            found_vrm_blend_shape_bind_mesh = true;

                                            assert(-1 == vrm_blend_shape_bind_mesh);
                                            vrm_blend_shape_bind_mesh = std::strtoull(vrm_blend_shape_group_element_bind_element_property_value.c_str(), NULL, 10);
                                        }
                                        else if ((6U == vrm_blend_shape_group_element_bind_element_property_name_length) && (0 == std::strncmp("weight", vrm_blend_shape_group_element_bind_element_property_name_string, 6U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrm_blend_shape_group_element_bind_element_property_value_string = json + tokens[token_index].start;
                                            size_t const vrm_blend_shape_group_element_bind_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrm_blend_shape_group_element_bind_element_property_value(vrm_blend_shape_group_element_bind_element_property_value_string, vrm_blend_shape_group_element_bind_element_property_value_length);

                                            assert(!found_vrm_blend_shape_bind_weight);
                                            found_vrm_blend_shape_bind_weight = true;

                                            assert(-1.0F == vrm_blend_shape_bind_weight);
                                            vrm_blend_shape_bind_weight = std::strtof(vrm_blend_shape_group_element_bind_element_property_value.c_str(), NULL);
                                        }
                                        else
                                        {
                                            assert(false);
                                            token_index = internal_skip_json(tokens, token_index);
                                            assert(token_index <= token_count);
                                        }
                                    }

                                    if (found_vrm_blend_shape_bind_index && found_vrm_blend_shape_bind_mesh && found_vrm_blend_shape_bind_weight)
                                    {
                                        assert(vrm_blend_shape_bind_index <= static_cast<unsigned long long>(UINT32_MAX));
                                        uint32_t const vrm_blend_shape_bind_index_uint32 = static_cast<uint32_t>(vrm_blend_shape_bind_index);

                                        assert(vrm_blend_shape_bind_mesh <= static_cast<unsigned long long>(UINT32_MAX));
                                        uint32_t const vrm_blend_shape_bind_mesh_index_uint32 = static_cast<uint32_t>(vrm_blend_shape_bind_mesh);

                                        assert(vrm_blend_shape_bind_weight >= 0.0F);
                                        assert(vrm_blend_shape_bind_weight <= 100.0F);
                                        float const vrm_blend_shape_bind_normalized_weight = vrm_blend_shape_bind_weight / 100.0F;

                                        vrm_blend_shape_group_element_binds.push_back(internal_vrm_blend_shape_bind{vrm_blend_shape_bind_index_uint32, vrm_blend_shape_bind_mesh_index_uint32, vrm_blend_shape_bind_normalized_weight});
                                    }
                                    else
                                    {
                                        assert(false);
                                    }
                                }
                            }
                            else if ((10U == vrm_blend_shape_group_element_property_name_length) && (0 == std::strncmp("presetName", vrm_blend_shape_group_element_property_name_string, 10U)))
                            {
                                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_blend_shape_group_element_preset_name_string = json + tokens[token_index].start;
                                size_t const vrm_blend_shape_group_element_preset_name_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                vrm_blend_shape_group_element_preset_name = mcrt_string(vrm_blend_shape_group_element_preset_name_string, vrm_blend_shape_group_element_preset_name_length);
                            }
                            else
                            {
                                token_index = internal_skip_json(tokens, token_index);
                                assert(token_index <= token_count);
                            }
                        }

                        auto found_vrm_blend_shape_names = vrm_blend_shape_name_strings.find(vrm_blend_shape_group_element_preset_name);
                        if (vrm_blend_shape_name_strings.end() != found_vrm_blend_shape_names)
                        {
                            for (internal_vrm_blend_shape_bind const &vrm_blend_shape_group_element_bind : vrm_blend_shape_group_element_binds)
                            {
                                auto &vrm_mesh_blend_shapes_morph_target_binds = out_vrm_meshes_blend_shapes_morph_target_binds[vrm_blend_shape_group_element_bind.m_mesh];

                                for (auto const &vrm_blend_shape_name : found_vrm_blend_shape_names->second)
                                {
                                    assert(found_vrm_blend_shape_names->second.size() >= 1U);
                                    float const weight = vrm_blend_shape_group_element_bind.m_weight / static_cast<float>(found_vrm_blend_shape_names->second.size());

                                    auto &vrm_mesh_blend_shape_morph_target_binds = vrm_mesh_blend_shapes_morph_target_binds[vrm_blend_shape_name];

                                    auto found_vrm_mesh_blend_shape_morph_target_bind = vrm_mesh_blend_shape_morph_target_binds.find(vrm_blend_shape_group_element_bind.m_index);
                                    if (vrm_mesh_blend_shape_morph_target_binds.end() == found_vrm_mesh_blend_shape_morph_target_bind)
                                    {
                                        vrm_mesh_blend_shape_morph_target_binds.emplace_hint(found_vrm_mesh_blend_shape_morph_target_bind, vrm_blend_shape_group_element_bind.m_index, weight);
                                    }
                                    else
                                    {
                                        found_vrm_mesh_blend_shape_morph_target_bind->second += weight;
                                    }
                                }
                            }
                        }
                        else
                        {
                            assert((0 == vrm_blend_shape_group_element_preset_name.compare("unknown")) || (0 == vrm_blend_shape_group_element_preset_name.compare("fun")) || vrm_blend_shape_group_element_binds.empty());
                        }
                    }
                }
                else
                {
                    assert(false);
                    token_index = internal_skip_json(tokens, token_index);
                    assert(token_index <= token_count);
                }

                if (internal_unlikely(token_index < 0))
                {
                    assert(false);
                    break;
                }
            }
        }
        else
        {
            token_index = internal_skip_json(tokens, token_index);
            assert(token_index <= token_count);
        }

        if (internal_unlikely(token_index < 0))
        {
            assert(false);
            break;
        }
    }

    assert(token_index == token_count);
}

static inline void internal_parse_vrm_skeleton_joint_names(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_map<uint32_t, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_vrm_human_bones)
{
    mcrt_unordered_map<mcrt_string, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> vrm_skeleton_joint_name_strings;
    {
        // https://github.com/saturday06/VRM-Addon-for-Blender/blob/main/src/io_scene_vrm/common/human_bone_mapper/mmd_mapping.py
        // https://github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.bone.schema.json
        vrm_skeleton_joint_name_strings["hips"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CENTER;
        vrm_skeleton_joint_name_strings["leftUpperLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LEG;
        vrm_skeleton_joint_name_strings["rightUpperLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LEG;
        vrm_skeleton_joint_name_strings["leftLowerLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_KNEE;
        vrm_skeleton_joint_name_strings["rightLowerLeg"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_KNEE;
        vrm_skeleton_joint_name_strings["leftFoot"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE;
        vrm_skeleton_joint_name_strings["rightFoot"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE;
        vrm_skeleton_joint_name_strings["spine"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY;
        vrm_skeleton_joint_name_strings["chest"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY_2;
        vrm_skeleton_joint_name_strings["neck"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_NECK;
        vrm_skeleton_joint_name_strings["head"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_HEAD;
        vrm_skeleton_joint_name_strings["leftShoulder"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER;
        vrm_skeleton_joint_name_strings["rightShoulder"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER;
        vrm_skeleton_joint_name_strings["leftUpperArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM;
        vrm_skeleton_joint_name_strings["rightUpperArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM;
        vrm_skeleton_joint_name_strings["leftLowerArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ELBOW;
        vrm_skeleton_joint_name_strings["rightLowerArm"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ELBOW;
        vrm_skeleton_joint_name_strings["leftHand"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_WRIST;
        vrm_skeleton_joint_name_strings["rightHand"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_WRIST;
        vrm_skeleton_joint_name_strings["leftToes"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP;
        vrm_skeleton_joint_name_strings["rightToes"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP;
        vrm_skeleton_joint_name_strings["leftEye"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE;
        vrm_skeleton_joint_name_strings["rightEye"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE;
        vrm_skeleton_joint_name_strings["leftThumbProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_0;
        vrm_skeleton_joint_name_strings["leftThumbIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_1;
        vrm_skeleton_joint_name_strings["leftThumbDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_2;
        vrm_skeleton_joint_name_strings["leftIndexProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_1;
        vrm_skeleton_joint_name_strings["leftIndexIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_2;
        vrm_skeleton_joint_name_strings["leftIndexDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_3;
        vrm_skeleton_joint_name_strings["leftMiddleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_1;
        vrm_skeleton_joint_name_strings["leftMiddleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_2;
        vrm_skeleton_joint_name_strings["leftMiddleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_3;
        vrm_skeleton_joint_name_strings["leftRingProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_1;
        vrm_skeleton_joint_name_strings["leftRingIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_2;
        vrm_skeleton_joint_name_strings["leftRingDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_3;
        vrm_skeleton_joint_name_strings["leftLittleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_1;
        vrm_skeleton_joint_name_strings["leftLittleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_2;
        vrm_skeleton_joint_name_strings["leftLittleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_3;
        vrm_skeleton_joint_name_strings["rightThumbProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_0;
        vrm_skeleton_joint_name_strings["rightThumbIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_1;
        vrm_skeleton_joint_name_strings["rightThumbDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_2;
        vrm_skeleton_joint_name_strings["rightIndexProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_1;
        vrm_skeleton_joint_name_strings["rightIndexIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_2;
        vrm_skeleton_joint_name_strings["rightIndexDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_3;
        vrm_skeleton_joint_name_strings["rightMiddleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_1;
        vrm_skeleton_joint_name_strings["rightMiddleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_2;
        vrm_skeleton_joint_name_strings["rightMiddleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_3;
        vrm_skeleton_joint_name_strings["rightRingProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_1;
        vrm_skeleton_joint_name_strings["rightRingIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_2;
        vrm_skeleton_joint_name_strings["rightRingDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_3;
        vrm_skeleton_joint_name_strings["rightLittleProximal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_1;
        vrm_skeleton_joint_name_strings["rightLittleIntermediate"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_2;
        vrm_skeleton_joint_name_strings["rightLittleDistal"] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_3;
    }

    int token_index = 0;
    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
    {
        assert(false);
        return;
    }

    int const vrm_property_count = tokens[token_index].size;
    ++token_index;
    assert(token_index <= token_count);

    for (int vrm_property_index = 0; vrm_property_index < vrm_property_count; ++vrm_property_index)
    {
        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
        {
            assert(false);
            return;
        }

        char const *const vrm_property_name_string = json + tokens[token_index].start;
        size_t const vrm_property_name_length = tokens[token_index].end - tokens[token_index].start;
        ++token_index;
        assert(token_index <= token_count);

        if ((8U == vrm_property_name_length) && (0 == std::strncmp("humanoid", vrm_property_name_string, 8U)))
        {
            if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
            {
                assert(false);
                return;
            }

            int const vrm_humanoid_property_count = tokens[token_index].size;
            ++token_index;
            assert(token_index <= token_count);

            for (int vrm_humanoid_property_index = 0; vrm_humanoid_property_index < vrm_humanoid_property_count; ++vrm_humanoid_property_index)
            {
                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                {
                    assert(false);
                    return;
                }

                char const *const vrm_humanoid_property_name_string = json + tokens[token_index].start;
                size_t const vrm_humanoid_property_name_length = tokens[token_index].end - tokens[token_index].start;
                ++token_index;
                assert(token_index <= token_count);

                if ((10U == vrm_humanoid_property_name_length) && (0 == std::strncmp("humanBones", vrm_humanoid_property_name_string, 10U)))
                {
                    if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                    {
                        assert(false);
                        return;
                    }

                    int const vrm_human_bone_element_count = tokens[token_index].size;
                    ++token_index;
                    assert(token_index <= token_count);

                    for (int vrm_human_bone_element_index = 0; vrm_human_bone_element_index < vrm_human_bone_element_count; ++vrm_human_bone_element_index)
                    {
                        if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                        {
                            assert(false);
                            return;
                        }

                        int const vrm_human_bone_element_property_count = tokens[token_index].size;
                        ++token_index;
                        assert(token_index <= token_count);

                        bool found_vrm_human_bone_bone = false;
                        mcrt_string vrm_human_bone_bone;

                        bool found_vrm_human_bone_node = false;
                        unsigned long long vrm_human_bone_node = -1;

                        for (int vrm_human_bone_element_property_index = 0; vrm_human_bone_element_property_index < vrm_human_bone_element_property_count; ++vrm_human_bone_element_property_index)
                        {
                            if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                            {
                                assert(false);
                                return;
                            }

                            char const *const vrm_human_bone_element_property_name_string = json + tokens[token_index].start;
                            size_t const vrm_human_bone_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                            ++token_index;
                            assert(token_index <= token_count);

                            if ((4U == vrm_human_bone_element_property_name_length) && (0 == std::strncmp("bone", vrm_human_bone_element_property_name_string, 4U)))
                            {
                                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_human_bone_element_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_human_bone_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                assert(!found_vrm_human_bone_bone);
                                found_vrm_human_bone_bone = true;

                                assert(vrm_human_bone_bone.empty());
                                vrm_human_bone_bone = mcrt_string(vrm_human_bone_element_property_value_string, vrm_human_bone_element_property_value_length);
                            }
                            else if ((4U == vrm_human_bone_element_property_name_length) && (0 == std::strncmp("node", vrm_human_bone_element_property_name_string, 4U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_human_bone_element_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_human_bone_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_human_bone_element_property_value(vrm_human_bone_element_property_value_string, vrm_human_bone_element_property_value_length);

                                assert(!found_vrm_human_bone_node);
                                found_vrm_human_bone_node = true;

                                assert(-1 == vrm_human_bone_node);
                                vrm_human_bone_node = std::strtoull(vrm_human_bone_element_property_value.c_str(), NULL, 10);
                            }
                            else
                            {
                                token_index = internal_skip_json(tokens, token_index);
                                assert(token_index <= token_count);
                            }
                        }

                        if (found_vrm_human_bone_bone && found_vrm_human_bone_node)
                        {
                            auto found_vrm_skeleton_joint_name = vrm_skeleton_joint_name_strings.find(vrm_human_bone_bone);
                            if (vrm_skeleton_joint_name_strings.end() != found_vrm_skeleton_joint_name)
                            {
                                assert(vrm_human_bone_node <= static_cast<unsigned long long>(UINT32_MAX));
                                uint32_t const vrm_human_bone_node_index_uint32 = static_cast<uint32_t>(vrm_human_bone_node);

                                assert(out_vrm_human_bones.end() == out_vrm_human_bones.find(vrm_human_bone_node_index_uint32));
                                out_vrm_human_bones[vrm_human_bone_node_index_uint32] = found_vrm_skeleton_joint_name->second;
                            }
                            else
                            {
                                assert((0 == vrm_human_bone_bone.compare("jaw")) || (0 == vrm_human_bone_bone.compare("upperChest")));
                            }
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                }
                else
                {
                    token_index = internal_skip_json(tokens, token_index);
                    assert(token_index <= token_count);
                }
            }
        }
        else
        {
            token_index = internal_skip_json(tokens, token_index);
            assert(token_index <= token_count);
        }

        if (internal_unlikely(token_index < 0))
        {
            assert(false);
            break;
        }
    }

    assert(token_index == token_count);
}

static inline void internal_parse_vrm_spring_bones(jsmntok_t const *const tokens, int const token_count, char const *const json, mcrt_vector<internal_vrm_collider_group_t> &out_vrm_collider_groups, mcrt_vector<internal_vrm_bone_group_t> &out_vrm_bone_groups)
{
    // Verlet Integration
    // [SpringBoneSystem::UpdateProcess](https://github.com/vrm-c/UniVRM/blob/master/Packages/VRM/Runtime/SpringBone/Logic/SpringBoneSystem.cs#L139)
    // [UpdateFastSpringBoneJob::Execute](https://github.com/vrm-c/UniVRM/blob/master/Packages/UniGLTF/Runtime/SpringBoneJobs/UpdateFastSpringBoneJob.cs#L47)

    assert(out_vrm_collider_groups.empty());
    assert(out_vrm_bone_groups.empty());

    int token_index = 0;
    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
    {
        assert(false);
        return;
    }

    int const vrm_property_count = tokens[token_index].size;
    ++token_index;
    assert(token_index <= token_count);

    for (int vrm_property_index = 0; vrm_property_index < vrm_property_count; ++vrm_property_index)
    {
        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
        {
            assert(false);
            return;
        }

        char const *const vrm_property_name_string = json + tokens[token_index].start;
        size_t const vrm_property_name_length = tokens[token_index].end - tokens[token_index].start;
        ++token_index;
        assert(token_index <= token_count);

        if ((18U == vrm_property_name_length) && (0 == std::strncmp("secondaryAnimation", vrm_property_name_string, 18U)))
        {
            if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
            {
                assert(false);
                return;
            }

            int const vrm_secondary_animation_property_count = tokens[token_index].size;
            ++token_index;
            assert(token_index <= token_count);

            for (int vrm_secondary_animation_property_index = 0; vrm_secondary_animation_property_index < vrm_secondary_animation_property_count; ++vrm_secondary_animation_property_index)
            {
                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                {
                    assert(false);
                    return;
                }

                char const *const vrm_secondary_animation_property_name_string = json + tokens[token_index].start;
                size_t const vrm_secondary_animation_property_name_length = tokens[token_index].end - tokens[token_index].start;
                ++token_index;
                assert(token_index <= token_count);

                if ((14U == vrm_secondary_animation_property_name_length) && (0 == std::strncmp("colliderGroups", vrm_secondary_animation_property_name_string, 14U)))
                {
                    if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                    {
                        assert(false);
                        return;
                    }

                    int const vrm_collider_group_element_count = tokens[token_index].size;
                    ++token_index;
                    assert(token_index <= token_count);

                    assert(out_vrm_collider_groups.empty());

                    for (int vrm_collider_group_element_index = 0; vrm_collider_group_element_index < vrm_collider_group_element_count; ++vrm_collider_group_element_index)
                    {
                        if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                        {
                            assert(false);
                            return;
                        }

                        int const vrm_collider_group_element_property_count = tokens[token_index].size;
                        ++token_index;
                        assert(token_index <= token_count);

                        bool found_vrm_collider_group_node_index = false;
                        unsigned long long vrm_collider_group_node_index = -1;

                        mcrt_vector<internal_vrm_collider_t> vrm_colliders;

                        for (int vrm_collider_group_element_property_index = 0; vrm_collider_group_element_property_index < vrm_collider_group_element_property_count; ++vrm_collider_group_element_property_index)
                        {
                            if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                            {
                                assert(false);
                                return;
                            }

                            char const *const vrm_collider_group_element_property_name_string = json + tokens[token_index].start;
                            size_t const vrm_collider_group_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                            ++token_index;
                            assert(token_index <= token_count);

                            if ((4U == vrm_collider_group_element_property_name_length) && (0 == std::strncmp("node", vrm_collider_group_element_property_name_string, 4U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_human_bone_element_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_human_bone_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_human_bone_element_property_value(vrm_human_bone_element_property_value_string, vrm_human_bone_element_property_value_length);

                                assert(!found_vrm_collider_group_node_index);
                                found_vrm_collider_group_node_index = true;

                                assert(-1 == vrm_collider_group_node_index);
                                vrm_collider_group_node_index = std::strtoull(vrm_human_bone_element_property_value.c_str(), NULL, 10);
                            }
                            else if ((9U == vrm_collider_group_element_property_name_length) && (0 == std::strncmp("colliders", vrm_collider_group_element_property_name_string, 9U)))
                            {
                                if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                int vrm_collider_group_element_collider_element_count = tokens[token_index].size;
                                ++token_index;
                                assert(token_index <= token_count);

                                assert(vrm_colliders.empty());

                                for (int vrm_collider_group_element_collider_element_index = 0; vrm_collider_group_element_collider_element_index < vrm_collider_group_element_collider_element_count; ++vrm_collider_group_element_collider_element_index)
                                {
                                    if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                                    {
                                        assert(false);
                                        return;
                                    }

                                    int const vrm_collider_group_element_collider_element_property_count = tokens[token_index].size;
                                    ++token_index;
                                    assert(token_index <= token_count);

                                    bool found_vrm_collider_group_element_collider_element_offset_x = false;
                                    float vrm_collider_group_element_collider_element_offset_x = 0.0F;

                                    bool found_vrm_collider_group_element_collider_element_offset_y = false;
                                    float vrm_collider_group_element_collider_element_offset_y = 0.0F;

                                    bool found_vrm_collider_group_element_collider_element_offset_z = false;
                                    float vrm_collider_group_element_collider_element_offset_z = 0.0F;

                                    bool found_vrm_collider_group_element_collider_element_radius = false;
                                    float vrm_collider_group_element_collider_element_radius = 0.0F;

                                    for (int vrm_collider_group_element_collider_element_property_index = 0; vrm_collider_group_element_collider_element_property_index < vrm_collider_group_element_collider_element_property_count; ++vrm_collider_group_element_collider_element_property_index)
                                    {
                                        if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                        {
                                            assert(false);
                                            return;
                                        }

                                        char const *const vrm_collider_group_element_collider_element_property_name_string = json + tokens[token_index].start;
                                        size_t const vrm_collider_group_element_collider_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                                        ++token_index;
                                        assert(token_index <= token_count);

                                        if ((6U == vrm_collider_group_element_collider_element_property_name_length) && (0 == std::strncmp("offset", vrm_collider_group_element_collider_element_property_name_string, 6U)))
                                        {
                                            if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            int const vrm_collider_group_element_collider_element_offset_property_value_property_count = tokens[token_index].size;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            for (int vrm_collider_group_element_collider_element_offset_property_value_property_index = 0; vrm_collider_group_element_collider_element_offset_property_value_property_index < vrm_collider_group_element_collider_element_offset_property_value_property_count; ++vrm_collider_group_element_collider_element_offset_property_value_property_index)
                                            {
                                                if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                                {
                                                    assert(false);
                                                    return;
                                                }

                                                char const *const vrm_collider_group_element_collider_element_offset_property_value_property_name_string = json + tokens[token_index].start;
                                                size_t const vrm_collider_group_element_collider_element_offset_property_value_property_name_length = tokens[token_index].end - tokens[token_index].start;
                                                ++token_index;
                                                assert(token_index <= token_count);

                                                if ((1U == vrm_collider_group_element_collider_element_offset_property_value_property_name_length) && (0 == std::strncmp("x", vrm_collider_group_element_collider_element_offset_property_value_property_name_string, 1U)))
                                                {
                                                    char const *const vrm_collider_group_element_collider_element_offset_property_value_property_value_string = json + tokens[token_index].start;
                                                    size_t const vrm_collider_group_element_collider_element_offset_property_value_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                                    ++token_index;
                                                    assert(token_index <= token_count);

                                                    mcrt_string vrm_collider_group_element_collider_element_offset_property_value_property_value(vrm_collider_group_element_collider_element_offset_property_value_property_value_string, vrm_collider_group_element_collider_element_offset_property_value_property_value_length);

                                                    assert(!found_vrm_collider_group_element_collider_element_offset_x);
                                                    found_vrm_collider_group_element_collider_element_offset_x = true;

                                                    assert(0.0F == vrm_collider_group_element_collider_element_offset_x);
                                                    vrm_collider_group_element_collider_element_offset_x = std::strtof(vrm_collider_group_element_collider_element_offset_property_value_property_value.c_str(), NULL);
                                                }
                                                else if ((1U == vrm_collider_group_element_collider_element_offset_property_value_property_name_length) && (0 == std::strncmp("y", vrm_collider_group_element_collider_element_offset_property_value_property_name_string, 1U)))
                                                {
                                                    char const *const vrm_collider_group_element_collider_element_offset_property_value_property_value_string = json + tokens[token_index].start;
                                                    size_t const vrm_collider_group_element_collider_element_offset_property_value_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                                    ++token_index;
                                                    assert(token_index <= token_count);

                                                    mcrt_string vrm_collider_group_element_collider_element_offset_property_value_property_value(vrm_collider_group_element_collider_element_offset_property_value_property_value_string, vrm_collider_group_element_collider_element_offset_property_value_property_value_length);

                                                    assert(!found_vrm_collider_group_element_collider_element_offset_y);
                                                    found_vrm_collider_group_element_collider_element_offset_y = true;

                                                    assert(0.0F == vrm_collider_group_element_collider_element_offset_y);
                                                    vrm_collider_group_element_collider_element_offset_y = std::strtof(vrm_collider_group_element_collider_element_offset_property_value_property_value.c_str(), NULL);
                                                }
                                                else if ((1U == vrm_collider_group_element_collider_element_offset_property_value_property_name_length) && (0 == std::strncmp("z", vrm_collider_group_element_collider_element_offset_property_value_property_name_string, 1U)))
                                                {
                                                    char const *const vrm_collider_group_element_collider_element_offset_property_value_property_value_string = json + tokens[token_index].start;
                                                    size_t const vrm_collider_group_element_collider_element_offset_property_value_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                                    ++token_index;
                                                    assert(token_index <= token_count);

                                                    mcrt_string vrm_collider_group_element_collider_element_offset_property_value_property_value(vrm_collider_group_element_collider_element_offset_property_value_property_value_string, vrm_collider_group_element_collider_element_offset_property_value_property_value_length);

                                                    assert(!found_vrm_collider_group_element_collider_element_offset_z);
                                                    found_vrm_collider_group_element_collider_element_offset_z = true;

                                                    assert(0.0F == vrm_collider_group_element_collider_element_offset_z);
                                                    vrm_collider_group_element_collider_element_offset_z = std::strtof(vrm_collider_group_element_collider_element_offset_property_value_property_value.c_str(), NULL);
                                                }
                                                else
                                                {
                                                    assert(false);
                                                    token_index = internal_skip_json(tokens, token_index);
                                                    assert(token_index <= token_count);
                                                }
                                            }
                                        }
                                        else if ((6U == vrm_collider_group_element_collider_element_property_name_length) && (0 == std::strncmp("radius", vrm_collider_group_element_collider_element_property_name_string, 6U)))
                                        {
                                            if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                            {
                                                assert(false);
                                                return;
                                            }

                                            char const *const vrm_collider_group_element_collider_element_radius_property_value_string = json + tokens[token_index].start;
                                            size_t const vrm_collider_group_element_collider_element_radius_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                            ++token_index;
                                            assert(token_index <= token_count);

                                            mcrt_string vrm_collider_group_element_collider_element_radius_property_value(vrm_collider_group_element_collider_element_radius_property_value_string, vrm_collider_group_element_collider_element_radius_property_value_length);

                                            assert(!found_vrm_collider_group_element_collider_element_radius);
                                            found_vrm_collider_group_element_collider_element_radius = true;

                                            assert(0.0F == vrm_collider_group_element_collider_element_radius);
                                            vrm_collider_group_element_collider_element_radius = std::strtof(vrm_collider_group_element_collider_element_radius_property_value.c_str(), NULL);
                                        }
                                        else
                                        {
                                            assert(false);
                                            token_index = internal_skip_json(tokens, token_index);
                                            assert(token_index <= token_count);
                                        }
                                    }

                                    if (found_vrm_collider_group_element_collider_element_offset_x && found_vrm_collider_group_element_collider_element_offset_y && found_vrm_collider_group_element_collider_element_offset_z && found_vrm_collider_group_element_collider_element_radius)
                                    {
                                        vrm_colliders.push_back(internal_vrm_collider_t{DirectX::XMFLOAT3(vrm_collider_group_element_collider_element_offset_x, vrm_collider_group_element_collider_element_offset_y, vrm_collider_group_element_collider_element_offset_z), vrm_collider_group_element_collider_element_radius});
                                    }
                                    else
                                    {
                                        assert(false);
                                    }
                                }
                            }
                            else
                            {
                                assert(false);
                                token_index = internal_skip_json(tokens, token_index);
                                assert(token_index <= token_count);
                            }
                        }

                        if (found_vrm_collider_group_node_index && (!vrm_colliders.empty()))
                        {
                            assert(vrm_collider_group_node_index <= static_cast<unsigned long long>(UINT32_MAX));
                            uint32_t const vrm_collider_group_node_index_uint32 = static_cast<uint32_t>(vrm_collider_group_node_index);

                            out_vrm_collider_groups.push_back(internal_vrm_collider_group_t{vrm_collider_group_node_index_uint32, std::move(vrm_colliders)});
                        }
                        else
                        {
                            assert(false);
                        }
                    }

                    assert(out_vrm_collider_groups.size() == vrm_collider_group_element_count);
                }
                else if ((10U == vrm_secondary_animation_property_name_length) && (0 == std::strncmp("boneGroups", vrm_secondary_animation_property_name_string, 10U)))
                {
                    if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                    {
                        assert(false);
                        return;
                    }

                    int const vrm_bone_group_element_count = tokens[token_index].size;
                    ++token_index;
                    assert(token_index <= token_count);

                    assert(out_vrm_bone_groups.empty());

                    for (int vrm_bone_group_element_index = 0; vrm_bone_group_element_index < vrm_bone_group_element_count; ++vrm_bone_group_element_index)
                    {
                        if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                        {
                            assert(false);
                            return;
                        }

                        int const vrm_bone_group_element_property_count = tokens[token_index].size;
                        ++token_index;
                        assert(token_index <= token_count);

                        bool found_vrm_bone_group_hit_radius = false;
                        float vrm_bone_group_hit_radius = -1.0F;

                        bool found_vrm_bone_group_drag_force = false;
                        float vrm_bone_group_drag_force = -1.0F;

                        bool found_vrm_bone_group_stiffiness = false;
                        float vrm_bone_group_stiffiness = -1.0F;

                        bool found_vrm_bone_group_gravity_power = false;
                        float vrm_bone_group_gravity_power = -1.0F;

                        bool found_vrm_bone_group_gravity_dir_x = false;
                        float vrm_bone_group_gravity_dir_x = 0.0F;

                        bool found_vrm_bone_group_gravity_dir_y = false;
                        float vrm_bone_group_gravity_dir_y = 0.0F;

                        bool found_vrm_bone_group_gravity_dir_z = false;
                        float vrm_bone_group_gravity_dir_z = 0.0F;

                        mcrt_vector<uint32_t> vrm_bones;
                        mcrt_vector<uint32_t> vrm_collider_groups;

                        for (int vrm_bone_group_element_property_index = 0; vrm_bone_group_element_property_index < vrm_bone_group_element_property_count; ++vrm_bone_group_element_property_index)
                        {
                            if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                            {
                                assert(false);
                                return;
                            }

                            char const *const vrm_bone_group_element_property_name_string = json + tokens[token_index].start;
                            size_t const vrm_bone_group_element_property_name_length = tokens[token_index].end - tokens[token_index].start;
                            ++token_index;
                            assert(token_index <= token_count);

                            if ((5U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("bones", vrm_bone_group_element_property_name_string, 5U)))
                            {
                                if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                int vrm_bone_group_element_bone_element_count = tokens[token_index].size;
                                ++token_index;
                                assert(token_index <= token_count);

                                assert(vrm_bones.empty());

                                for (int vrm_bone_group_element_bone_element_index = 0; vrm_bone_group_element_bone_element_index < vrm_bone_group_element_bone_element_count; ++vrm_bone_group_element_bone_element_index)
                                {
                                    if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                    {
                                        assert(false);
                                        return;
                                    }

                                    char const *const vrm_bone_group_element_bone_element_property_value_string = json + tokens[token_index].start;
                                    size_t const vrm_bone_group_element_bone_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                    ++token_index;
                                    assert(token_index <= token_count);

                                    mcrt_string vrm_bone_group_element_bone_element_property_value(vrm_bone_group_element_bone_element_property_value_string, vrm_bone_group_element_bone_element_property_value_length);

                                    unsigned long long vrm_bone_group_element_bone_index = std::strtoull(vrm_bone_group_element_bone_element_property_value.c_str(), NULL, 10);

                                    assert(vrm_bone_group_element_bone_index <= static_cast<unsigned long long>(UINT32_MAX));
                                    uint32_t const vrm_bone_group_element_bone_index_uint32 = static_cast<uint32_t>(vrm_bone_group_element_bone_index);

                                    vrm_bones.push_back(vrm_bone_group_element_bone_index_uint32);
                                }

                                assert(vrm_bones.size() == vrm_bone_group_element_bone_element_count);
                            }
                            else if ((14U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("colliderGroups", vrm_bone_group_element_property_name_string, 14U)))
                            {
                                if (internal_unlikely(JSMN_ARRAY != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                int const vrm_bone_group_element_collider_group_element_count = tokens[token_index].size;
                                ++token_index;
                                assert(token_index <= token_count);

                                assert(vrm_collider_groups.empty());

                                for (int vrm_bone_group_element_collider_group_element_index = 0; vrm_bone_group_element_collider_group_element_index < vrm_bone_group_element_collider_group_element_count; ++vrm_bone_group_element_collider_group_element_index)
                                {
                                    if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                    {
                                        assert(false);
                                        return;
                                    }

                                    char const *const vrm_bone_group_element_collider_group_element_property_value_string = json + tokens[token_index].start;
                                    size_t const vrm_bone_group_element_collider_group_element_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                    ++token_index;
                                    assert(token_index <= token_count);

                                    mcrt_string vrm_bone_group_element_collider_group_element_property_value(vrm_bone_group_element_collider_group_element_property_value_string, vrm_bone_group_element_collider_group_element_property_value_length);

                                    unsigned long long vrm_bone_group_element_collider_group_index = std::strtoull(vrm_bone_group_element_collider_group_element_property_value.c_str(), NULL, 10);

                                    assert(vrm_bone_group_element_collider_group_index <= static_cast<unsigned long long>(UINT32_MAX));
                                    uint32_t const vrm_bone_group_element_collider_group_index_uint32 = static_cast<uint32_t>(vrm_bone_group_element_collider_group_index);

                                    vrm_collider_groups.push_back(vrm_bone_group_element_collider_group_index_uint32);
                                }

                                assert(vrm_collider_groups.size() == vrm_bone_group_element_collider_group_element_count);
                            }
                            else if ((9U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("hitRadius", vrm_bone_group_element_property_name_string, 9U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_bone_group_element_hit_radius_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_bone_group_element_hit_radius_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_bone_group_element_hit_radius_property_value(vrm_bone_group_element_hit_radius_property_value_string, vrm_bone_group_element_hit_radius_property_value_length);

                                assert(!found_vrm_bone_group_hit_radius);
                                found_vrm_bone_group_hit_radius = true;

                                assert(-1.0F == vrm_bone_group_hit_radius);
                                vrm_bone_group_hit_radius = std::strtof(vrm_bone_group_element_hit_radius_property_value.c_str(), NULL);
                            }
                            else if ((9U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("dragForce", vrm_bone_group_element_property_name_string, 9U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_bone_group_element_drag_force_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_bone_group_element_drag_force_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_bone_group_element_drag_force_property_value(vrm_bone_group_element_drag_force_property_value_string, vrm_bone_group_element_drag_force_property_value_length);

                                assert(!found_vrm_bone_group_drag_force);
                                found_vrm_bone_group_drag_force = true;

                                assert(-1.0F == vrm_bone_group_drag_force);
                                vrm_bone_group_drag_force = std::strtof(vrm_bone_group_element_drag_force_property_value.c_str(), NULL);
                            }
                            else if ((10U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("stiffiness", vrm_bone_group_element_property_name_string, 10U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_bone_group_element_stiffiness_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_bone_group_element_stiffiness_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_bone_group_element_stiffiness_property_value(vrm_bone_group_element_stiffiness_property_value_string, vrm_bone_group_element_stiffiness_property_value_length);

                                assert(!found_vrm_bone_group_stiffiness);
                                found_vrm_bone_group_stiffiness = true;

                                assert(-1.0F == vrm_bone_group_stiffiness);
                                vrm_bone_group_stiffiness = std::strtof(vrm_bone_group_element_stiffiness_property_value.c_str(), NULL);
                            }
                            else if ((12U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("gravityPower", vrm_bone_group_element_property_name_string, 12U)))
                            {
                                if (internal_unlikely(JSMN_PRIMITIVE != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                char const *const vrm_bone_group_element_gravity_power_property_value_string = json + tokens[token_index].start;
                                size_t const vrm_bone_group_element_gravity_power_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                ++token_index;
                                assert(token_index <= token_count);

                                mcrt_string vrm_bone_group_element_gravity_power_property_value(vrm_bone_group_element_gravity_power_property_value_string, vrm_bone_group_element_gravity_power_property_value_length);

                                assert(!found_vrm_bone_group_gravity_power);
                                found_vrm_bone_group_gravity_power = true;

                                assert(-1.0F == vrm_bone_group_gravity_power);
                                vrm_bone_group_gravity_power = std::strtof(vrm_bone_group_element_gravity_power_property_value.c_str(), NULL);
                            }
                            else if ((10U == vrm_bone_group_element_property_name_length) && (0 == std::strncmp("gravityDir", vrm_bone_group_element_property_name_string, 10U)))
                            {
                                if (internal_unlikely(JSMN_OBJECT != tokens[token_index].type))
                                {
                                    assert(false);
                                    return;
                                }

                                int const vrm_bone_group_element_gravity_dir_property_value_property_count = tokens[token_index].size;
                                ++token_index;
                                assert(token_index <= token_count);

                                for (int vrm_bone_group_element_gravity_dir_property_value_property_index = 0; vrm_bone_group_element_gravity_dir_property_value_property_index < vrm_bone_group_element_gravity_dir_property_value_property_count; ++vrm_bone_group_element_gravity_dir_property_value_property_index)
                                {
                                    if (internal_unlikely(JSMN_STRING != tokens[token_index].type))
                                    {
                                        assert(false);
                                        return;
                                    }

                                    char const *const vrm_bone_group_element_gravity_dir_property_value_property_name_string = json + tokens[token_index].start;
                                    size_t const vrm_bone_group_element_gravity_dir_property_value_property_name_length = tokens[token_index].end - tokens[token_index].start;
                                    ++token_index;
                                    assert(token_index <= token_count);

                                    if ((1U == vrm_bone_group_element_gravity_dir_property_value_property_name_length) && (0 == std::strncmp("x", vrm_bone_group_element_gravity_dir_property_value_property_name_string, 1U)))
                                    {
                                        char const *const vrm_bone_group_element_gravity_dir_property_value_property_value_string = json + tokens[token_index].start;
                                        size_t const vrm_bone_group_element_gravity_dir_property_value_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                        ++token_index;
                                        assert(token_index <= token_count);

                                        mcrt_string vrm_bone_group_element_gravity_dir_property_value_property_value(vrm_bone_group_element_gravity_dir_property_value_property_value_string, vrm_bone_group_element_gravity_dir_property_value_property_value_length);

                                        assert(!found_vrm_bone_group_gravity_dir_x);
                                        found_vrm_bone_group_gravity_dir_x = true;

                                        assert(0.0F == vrm_bone_group_gravity_dir_x);
                                        vrm_bone_group_gravity_dir_x = std::strtof(vrm_bone_group_element_gravity_dir_property_value_property_value.c_str(), NULL);
                                    }
                                    else if ((1U == vrm_bone_group_element_gravity_dir_property_value_property_name_length) && (0 == std::strncmp("y", vrm_bone_group_element_gravity_dir_property_value_property_name_string, 1U)))
                                    {
                                        char const *const vrm_bone_group_element_gravity_dir_property_value_property_value_string = json + tokens[token_index].start;
                                        size_t const vrm_bone_group_element_gravity_dir_property_value_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                        ++token_index;
                                        assert(token_index <= token_count);

                                        mcrt_string vrm_bone_group_element_gravity_dir_property_value_property_value(vrm_bone_group_element_gravity_dir_property_value_property_value_string, vrm_bone_group_element_gravity_dir_property_value_property_value_length);

                                        assert(!found_vrm_bone_group_gravity_dir_y);
                                        found_vrm_bone_group_gravity_dir_y = true;

                                        assert(0.0F == vrm_bone_group_gravity_dir_y);
                                        vrm_bone_group_gravity_dir_y = std::strtof(vrm_bone_group_element_gravity_dir_property_value_property_value.c_str(), NULL);
                                    }
                                    else if ((1U == vrm_bone_group_element_gravity_dir_property_value_property_name_length) && (0 == std::strncmp("z", vrm_bone_group_element_gravity_dir_property_value_property_name_string, 1U)))
                                    {
                                        char const *const vrm_bone_group_element_gravity_dir_property_value_property_value_string = json + tokens[token_index].start;
                                        size_t const vrm_bone_group_element_gravity_dir_property_value_property_value_length = tokens[token_index].end - tokens[token_index].start;
                                        ++token_index;
                                        assert(token_index <= token_count);

                                        mcrt_string vrm_bone_group_element_gravity_dir_property_value_property_value(vrm_bone_group_element_gravity_dir_property_value_property_value_string, vrm_bone_group_element_gravity_dir_property_value_property_value_length);

                                        assert(!found_vrm_bone_group_gravity_dir_z);
                                        found_vrm_bone_group_gravity_dir_z = true;

                                        assert(0.0F == vrm_bone_group_gravity_dir_z);
                                        vrm_bone_group_gravity_dir_z = std::strtof(vrm_bone_group_element_gravity_dir_property_value_property_value.c_str(), NULL);
                                    }
                                    else
                                    {
                                        assert(false);
                                        token_index = internal_skip_json(tokens, token_index);
                                        assert(token_index <= token_count);
                                    }
                                }
                            }
                            else
                            {
                                token_index = internal_skip_json(tokens, token_index);
                                assert(token_index <= token_count);
                            }
                        }

                        assert(found_vrm_bone_group_stiffiness);

                        assert(found_vrm_bone_group_gravity_power);

                        assert(found_vrm_bone_group_gravity_dir_x);
                        assert(0.0F == vrm_bone_group_gravity_dir_x);

                        assert(found_vrm_bone_group_gravity_dir_y);
                        assert(-1.0F == vrm_bone_group_gravity_dir_y);

                        assert(found_vrm_bone_group_gravity_dir_z);
                        assert(0.0F == vrm_bone_group_gravity_dir_z);

                        if (found_vrm_bone_group_hit_radius && found_vrm_bone_group_drag_force)
                        {
                            out_vrm_bone_groups.push_back(internal_vrm_bone_group_t{vrm_bone_group_hit_radius, vrm_bone_group_drag_force, std::move(vrm_bones), std::move(vrm_collider_groups)});
                        }
                        else
                        {
                            assert(false);
                        }
                    }

                    assert(out_vrm_bone_groups.size() == vrm_bone_group_element_count);
                }
                else
                {
                    token_index = internal_skip_json(tokens, token_index);
                    assert(token_index <= token_count);
                }
            }
        }
        else
        {
            token_index = internal_skip_json(tokens, token_index);
            assert(token_index <= token_count);
        }

        if (internal_unlikely(token_index < 0))
        {
            assert(false);
            break;
        }
    }

    assert(token_index == token_count);
}

static inline int internal_skip_json(jsmntok_t const *const tokens, int const token_index)
{
    int i = token_index;
    int end = i + 1;

    while (i < end)
    {
        switch (tokens[i].type)
        {
        case JSMN_OBJECT:
            end += tokens[i].size * 2;
            ++i;
            break;

        case JSMN_ARRAY:
            end += tokens[i].size;
            ++i;
            break;

        case JSMN_PRIMITIVE:
        case JSMN_STRING:
            ++i;
            break;

        default:
            assert(false);
            i = -1;
        }

        if (internal_unlikely(i < 0))
        {
            assert(false);
            break;
        }
    }

    return i;
}

static inline void internal_import_animation_skeleton(cgltf_data const *in_gltf_data, mcrt_vector<DirectX::XMFLOAT4X4> &out_gltf_node_model_space_matrices, mcrt_vector<mcrt_set<uint32_t>> &out_gltf_meshes_instance_node_indices, mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_animation_skeleton_joint_names, mcrt_vector<uint32_t> &out_animation_skeleton_joint_parent_indices, mcrt_vector<uint32_t> &out_gltf_node_to_animation_skeleton_joint_map, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_local_space_matrices, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_model_space_matrices)
{
    assert(out_gltf_node_model_space_matrices.empty());
    assert(out_gltf_meshes_instance_node_indices.empty());
    assert(out_animation_skeleton_joint_names.empty());
    assert(out_animation_skeleton_joint_parent_indices.empty());
    assert(out_gltf_node_to_animation_skeleton_joint_map.empty());
    assert(out_animation_skeleton_bind_pose_local_space_matrices.empty());
    assert(out_animation_skeleton_bind_pose_model_space_matrices.empty());

    bool found_vrmc_vrm = false;
    mcrt_map<uint32_t, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> vrmc_vrm_human_bones;

    bool found_vrm = false;
    mcrt_map<uint32_t, BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> vrm_human_bones;

    for (uint32_t data_extension_index = 0U; data_extension_index < in_gltf_data->data_extensions_count; ++data_extension_index)
    {
        cgltf_extension const *const gltf_data_extension = in_gltf_data->data_extensions + data_extension_index;
        if ((NULL != gltf_data_extension->name) && (0 == std::strcmp("VRMC_vrm", gltf_data_extension->name)) && (NULL != gltf_data_extension->data))
        {
            size_t const gltf_data_extension_data_length = std::strlen(gltf_data_extension->data);

            jsmn_parser parser = {0, 0, 0};

            int const option_token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, NULL, 0U);

            jsmn_init(&parser);

            jsmntok_t *const tokens = static_cast<jsmntok_t *>(mcrt_malloc(sizeof(jsmntok_t) * (option_token_count + 1), alignof(jsmntok_t)));
            assert(NULL != tokens);

            int const token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, tokens, option_token_count);
            assert(option_token_count == token_count);

            tokens[token_count].type = JSMN_UNDEFINED;

            assert(!found_vrmc_vrm);

            found_vrmc_vrm = true;

            internal_parse_vrmc_vrm_skeleton_joint_names(tokens, token_count, gltf_data_extension->data, vrmc_vrm_human_bones);

            mcrt_free(tokens);

            break;
        }
        else if ((NULL != gltf_data_extension->name) && (0 == std::strcmp("VRM", gltf_data_extension->name)) && (NULL != gltf_data_extension->data))
        {
            size_t const gltf_data_extension_data_length = std::strlen(gltf_data_extension->data);

            jsmn_parser parser = {0, 0, 0};

            int const option_token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, NULL, 0U);

            jsmn_init(&parser);

            jsmntok_t *const tokens = static_cast<jsmntok_t *>(mcrt_malloc(sizeof(jsmntok_t) * (option_token_count + 1), alignof(jsmntok_t)));
            assert(NULL != tokens);

            int const token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, tokens, option_token_count);
            assert(option_token_count == token_count);

            tokens[token_count].type = JSMN_UNDEFINED;

            assert(!found_vrm);

            found_vrm = true;

            internal_parse_vrm_skeleton_joint_names(tokens, token_count, gltf_data_extension->data, vrm_human_bones);

            mcrt_free(tokens);

            break;
        }
    }

    mcrt_vector<uint32_t> animation_skeleton_joint_to_gltf_node_map;
    {
        mcrt_set<uint32_t> gltf_skeleton_node_indices;
        mcrt_vector<DirectX::XMFLOAT4X4> gltf_node_bind_pose_model_space_matrices(static_cast<size_t>(in_gltf_data->nodes_count));
        uint32_t gltf_skeleton_root_node_index;
        {
            mcrt_vector<uint32_t> gltf_node_parent_indices(static_cast<size_t>(in_gltf_data->nodes_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

            // some nodes may be NOT included in the scene // we initialize by the identity transform
            {
                DirectX::XMFLOAT4X4 identity_transform;
                DirectX::XMStoreFloat4x4(&identity_transform, DirectX::XMMatrixIdentity());
                out_gltf_node_model_space_matrices.assign(static_cast<size_t>(in_gltf_data->nodes_count), identity_transform);
            }

            out_gltf_meshes_instance_node_indices.resize(in_gltf_data->meshes_count);

            mcrt_set<uint32_t> gltf_skin_indices;

            {
                mcrt_vector<DirectX::XMFLOAT4X4> gltf_node_local_space_matrices(static_cast<size_t>(in_gltf_data->nodes_count));

                for (size_t gltf_node_index = 0; gltf_node_index < in_gltf_data->nodes_count; ++gltf_node_index)
                {
                    cgltf_node const *gltf_node = &in_gltf_data->nodes[gltf_node_index];

                    if (gltf_node->has_matrix)
                    {
                        DirectX::XMFLOAT4X4 raw_gltf_node_local_matrix(gltf_node->matrix);

                        DirectX::XMVECTOR simd_raw_gltf_node_local_translation;
                        DirectX::XMVECTOR simd_raw_gltf_node_local_scale;
                        DirectX::XMVECTOR simd_raw_gltf_node_local_rotation;
                        bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&simd_raw_gltf_node_local_scale, &simd_raw_gltf_node_local_rotation, &simd_raw_gltf_node_local_translation, DirectX::XMLoadFloat4x4(&raw_gltf_node_local_matrix));
                        assert(directx_xm_matrix_decompose);

                        constexpr float const INTERNAL_SCALE_EPSILON = 9E-5F;
                        assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(simd_raw_gltf_node_local_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));

                        DirectX::XMFLOAT3 raw_gltf_node_local_translation;
                        DirectX::XMStoreFloat3(&raw_gltf_node_local_translation, simd_raw_gltf_node_local_translation);

                        DirectX::XMFLOAT4 raw_gltf_node_local_rotation;
                        DirectX::XMStoreFloat4(&raw_gltf_node_local_rotation, simd_raw_gltf_node_local_rotation);

                        DirectX::XMFLOAT3 gltf_node_local_translation = internal_transform_translation(raw_gltf_node_local_translation, found_vrmc_vrm, found_vrm);

                        DirectX::XMFLOAT4 gltf_node_local_rotation = internal_transform_rotation(raw_gltf_node_local_rotation, found_vrmc_vrm, found_vrm);

                        DirectX::XMStoreFloat4x4(&gltf_node_local_space_matrices[gltf_node_index], DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&gltf_node_local_rotation)), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&gltf_node_local_translation))));
                    }
                    else
                    {
                        if (gltf_node->has_scale)
                        {
                            DirectX::XMFLOAT3 gltf_node_local_scale(gltf_node->scale);

                            constexpr float const INTERNAL_SCALE_EPSILON = 9E-5F;
                            assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&gltf_node_local_scale), DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));
                        }

                        DirectX::XMFLOAT4 gltf_node_local_rotation;
                        if (gltf_node->has_rotation)
                        {
                            gltf_node_local_rotation = internal_transform_rotation(DirectX::XMFLOAT4(gltf_node->rotation), found_vrmc_vrm, found_vrm);
                        }
                        else
                        {
                            DirectX::XMStoreFloat4(&gltf_node_local_rotation, DirectX::XMQuaternionIdentity());
                        }

                        DirectX::XMFLOAT3 gltf_node_local_translation;
                        if (gltf_node->has_translation)
                        {
                            gltf_node_local_translation = internal_transform_translation(DirectX::XMFLOAT3(gltf_node->translation), found_vrmc_vrm, found_vrm);
                        }
                        else
                        {
                            DirectX::XMStoreFloat3(&gltf_node_local_translation, DirectX::XMVectorZero());
                        }

                        DirectX::XMStoreFloat4x4(&gltf_node_local_space_matrices[gltf_node_index], DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&gltf_node_local_rotation)), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&gltf_node_local_translation))));
                    }
                }

                {
                    struct internal_scene_depth_first_search_traverse_context
                    {
                        mcrt_vector<DirectX::XMFLOAT4X4> const &in_gltf_node_local_space_matrices;
                        mcrt_vector<uint32_t> &out_gltf_node_parent_indices;
                        mcrt_vector<DirectX::XMFLOAT4X4> &out_gltf_node_model_space_matrices;
                        mcrt_vector<mcrt_set<uint32_t>> &out_gltf_meshes_instance_node_indices;
                        mcrt_set<uint32_t> &out_gltf_skin_indices;
                    };

                    internal_scene_depth_first_search_traverse_context scene_depth_first_search_traverse_context = {
                        gltf_node_local_space_matrices,
                        gltf_node_parent_indices,
                        out_gltf_node_model_space_matrices,
                        out_gltf_meshes_instance_node_indices,
                        gltf_skin_indices};

                    internal_scene_depth_first_search_traverse(
                        in_gltf_data,
                        [](cgltf_data const *in_gltf_data, cgltf_node const *in_current_gltf_node, cgltf_node const *in_parent_gltf_node, void *user_data) -> void
                        {
                            internal_scene_depth_first_search_traverse_context *const scene_depth_first_search_traverse_context = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data);

                            mcrt_vector<DirectX::XMFLOAT4X4> const &in_gltf_node_local_space_matrices = scene_depth_first_search_traverse_context->in_gltf_node_local_space_matrices;
                            mcrt_vector<uint32_t> &out_gltf_node_parent_indices = scene_depth_first_search_traverse_context->out_gltf_node_parent_indices;
                            mcrt_vector<DirectX::XMFLOAT4X4> &out_gltf_node_model_space_matrices = scene_depth_first_search_traverse_context->out_gltf_node_model_space_matrices;
                            mcrt_vector<mcrt_set<uint32_t>> &out_gltf_meshes_instance_node_indices = scene_depth_first_search_traverse_context->out_gltf_meshes_instance_node_indices;
                            mcrt_set<uint32_t> &out_gltf_skin_indices = scene_depth_first_search_traverse_context->out_gltf_skin_indices;

                            uint32_t const gltf_current_node_index = cgltf_node_index(in_gltf_data, in_current_gltf_node);

                            uint32_t const gltf_parent_node_index = (NULL != in_parent_gltf_node) ? cgltf_node_index(in_gltf_data, in_parent_gltf_node) : BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;

                            DirectX::XMMATRIX local_transform = DirectX::XMLoadFloat4x4(&in_gltf_node_local_space_matrices[gltf_current_node_index]);

                            DirectX::XMMATRIX parent_world_transform = (NULL != in_parent_gltf_node) ? DirectX::XMLoadFloat4x4(&out_gltf_node_model_space_matrices[gltf_parent_node_index]) : DirectX::XMMatrixIdentity();

                            DirectX::XMMATRIX world_transform = DirectX::XMMatrixMultiply(local_transform, parent_world_transform);

                            DirectX::XMStoreFloat4x4(&out_gltf_node_model_space_matrices[gltf_current_node_index], world_transform);

                            assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_gltf_node_parent_indices[gltf_current_node_index]);
                            out_gltf_node_parent_indices[gltf_current_node_index] = ((NULL != in_parent_gltf_node) ? gltf_parent_node_index : BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

                            assert(NULL == in_current_gltf_node->skin || NULL != in_current_gltf_node->mesh);

                            if (NULL != in_current_gltf_node->mesh)
                            {
                                mcrt_set<uint32_t> &out_gltf_mesh_instance_node_indices = out_gltf_meshes_instance_node_indices[cgltf_mesh_index(in_gltf_data, in_current_gltf_node->mesh)];
                                assert(out_gltf_mesh_instance_node_indices.end() == out_gltf_mesh_instance_node_indices.find(gltf_current_node_index));
                                out_gltf_mesh_instance_node_indices.insert(gltf_current_node_index);

                                if (NULL != in_current_gltf_node->skin)
                                {
                                    uint32_t const gltf_current_node_skin_index = cgltf_skin_index(in_gltf_data, in_current_gltf_node->skin);
                                    out_gltf_skin_indices.insert(gltf_current_node_skin_index);
                                }
                            }
                        },
                        &scene_depth_first_search_traverse_context);
                }
            }

#ifndef NDEBUG
            // there may be skins which are NOT included in the scene
            for (cgltf_size gltf_skin_index = 0U; gltf_skin_index < in_gltf_data->skins_count; ++gltf_skin_index)
            {
                if (gltf_skin_indices.end() == gltf_skin_indices.find(gltf_skin_index))
                {
                    cgltf_skin const *const gltf_skin = in_gltf_data->skins + gltf_skin_index;

                    std::cout << "Skeleton: \"" << ((NULL != gltf_skin->name) ? gltf_skin->name : "Name N/A") << "\" is not included in the scene and has been ignored." << std::endl;
                }
            }
#endif

            if (internal_unlikely(gltf_skin_indices.empty()))
            {
                // no skeleton
                return;
            }

            mcrt_set<uint32_t> gltf_skeleton_root_node_indices;

            for (uint32_t const gltf_skin_index : gltf_skin_indices)
            {
                cgltf_skin const *const gltf_skin = in_gltf_data->skins + gltf_skin_index;

                // we do not use the matrices from gltf skin
                // cgltf_accessor const *skin_inverse_bind_matrix_accessor = gltf_skin->inverse_bind_matrices;

                for (size_t gltf_joint_index = 0; gltf_joint_index < gltf_skin->joints_count; ++gltf_joint_index)
                {
                    mcrt_set<uint32_t>::iterator found_gltf_skeleton_node_index;
                    size_t gltf_node_index = cgltf_node_index(in_gltf_data, gltf_skin->joints[gltf_joint_index]);
                    while (gltf_skeleton_node_indices.end() == (found_gltf_skeleton_node_index = gltf_skeleton_node_indices.find(gltf_node_index)))
                    {
                        gltf_skeleton_node_indices.insert(found_gltf_skeleton_node_index, gltf_node_index);

                        if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != gltf_node_parent_indices[gltf_node_index])
                        {
                            gltf_node_index = gltf_node_parent_indices[gltf_node_index];
                        }
                        else
                        {
                            gltf_skeleton_root_node_indices.insert(gltf_node_index);
                            break;
                        }
                    }
                }
            }

            // we do not use the matrices from gltf skin
            // cgltf_accessor const *skin_inverse_bind_matrix_accessor = gltf_skin->inverse_bind_matrices;

            for (cgltf_size gltf_node_index = 0U; gltf_node_index < in_gltf_data->nodes_count; ++gltf_node_index)
            {
                gltf_node_bind_pose_model_space_matrices[gltf_node_index] = out_gltf_node_model_space_matrices[gltf_node_index];
            }

            if (gltf_skeleton_root_node_indices.size() == 1U)
            {
                gltf_skeleton_root_node_index = (*gltf_skeleton_root_node_indices.begin());
            }
            else
            {
                assert(gltf_skeleton_root_node_indices.size() > 1U);

                gltf_skeleton_root_node_index = BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;
            }
        }

        out_animation_skeleton_joint_names = {};
        out_animation_skeleton_joint_parent_indices = {};
        animation_skeleton_joint_to_gltf_node_map = {};
        out_gltf_node_to_animation_skeleton_joint_map.assign(static_cast<size_t>(in_gltf_data->nodes_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
        out_animation_skeleton_bind_pose_local_space_matrices = {};
        out_animation_skeleton_bind_pose_model_space_matrices = {};
        // the zero-th joint is always the root
        if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != gltf_skeleton_root_node_index)
        {
            out_animation_skeleton_joint_names.reserve(gltf_skeleton_node_indices.size());
            out_animation_skeleton_joint_parent_indices.reserve(gltf_skeleton_node_indices.size());
            out_animation_skeleton_bind_pose_local_space_matrices.reserve(gltf_skeleton_node_indices.size());
            out_animation_skeleton_bind_pose_model_space_matrices.reserve(gltf_skeleton_node_indices.size());

            out_animation_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

            animation_skeleton_joint_to_gltf_node_map.assign(static_cast<size_t>(gltf_skeleton_node_indices.size()), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

            animation_skeleton_joint_to_gltf_node_map[0U] = gltf_skeleton_root_node_index;
            out_gltf_node_to_animation_skeleton_joint_map[gltf_skeleton_root_node_index] = 0U;

            out_animation_skeleton_bind_pose_model_space_matrices.push_back(gltf_node_bind_pose_model_space_matrices[gltf_skeleton_root_node_index]);
        }
        else
        {
            out_animation_skeleton_joint_names.reserve(gltf_skeleton_node_indices.size() + 1U);
            out_animation_skeleton_joint_parent_indices.reserve(gltf_skeleton_node_indices.size() + 1U);
            out_animation_skeleton_bind_pose_local_space_matrices.reserve(gltf_skeleton_node_indices.size() + 1U);
            out_animation_skeleton_bind_pose_model_space_matrices.reserve(gltf_skeleton_node_indices.size() + 1U);

            out_animation_skeleton_joint_names.push_back(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID);

            out_animation_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

            animation_skeleton_joint_to_gltf_node_map.assign(static_cast<size_t>(gltf_skeleton_node_indices.size() + 1U), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

            DirectX::XMFLOAT4X4 identity_matrix;
            DirectX::XMStoreFloat4x4(&identity_matrix, DirectX::XMMatrixIdentity());
            out_animation_skeleton_bind_pose_local_space_matrices.push_back(identity_matrix);
            out_animation_skeleton_bind_pose_model_space_matrices.push_back(identity_matrix);
        }

        {

            struct internal_scene_depth_first_search_traverse_context
            {
                uint32_t const in_gltf_skeleton_root_node_index;
                mcrt_set<uint32_t> const &in_gltf_skeleton_node_indices;
                mcrt_vector<DirectX::XMFLOAT4X4> const &in_gltf_node_bind_pose_model_space_matrices;
                mcrt_vector<uint32_t> &out_animation_skeleton_joint_parent_indices;
                mcrt_vector<uint32_t> &out_gltf_node_to_animation_skeleton_joint_map;
                mcrt_vector<uint32_t> &out_animation_skeleton_joint_to_gltf_node_map;
                mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_model_space_matrices;
            };

            internal_scene_depth_first_search_traverse_context scene_depth_first_search_traverse_context = {
                gltf_skeleton_root_node_index,
                gltf_skeleton_node_indices,
                gltf_node_bind_pose_model_space_matrices,
                out_animation_skeleton_joint_parent_indices,
                out_gltf_node_to_animation_skeleton_joint_map,
                animation_skeleton_joint_to_gltf_node_map,
                out_animation_skeleton_bind_pose_model_space_matrices};

            internal_scene_depth_first_search_traverse(
                in_gltf_data,
                [](cgltf_data const *in_gltf_data, cgltf_node const *in_current_gltf_node, cgltf_node const *in_parent_gltf_node, void *user_data) -> void
                {
                    uint32_t const in_gltf_skeleton_root_node_index = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->in_gltf_skeleton_root_node_index;
                    mcrt_set<uint32_t> const &in_gltf_skeleton_node_indices = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->in_gltf_skeleton_node_indices;
                    mcrt_vector<DirectX::XMFLOAT4X4> const &in_gltf_node_bind_pose_model_space_matrices = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->in_gltf_node_bind_pose_model_space_matrices;
                    mcrt_vector<uint32_t> &out_animation_skeleton_joint_parent_indices = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->out_animation_skeleton_joint_parent_indices;
                    mcrt_vector<uint32_t> &out_gltf_node_to_animation_skeleton_joint_map = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->out_gltf_node_to_animation_skeleton_joint_map;
                    mcrt_vector<uint32_t> &out_animation_skeleton_joint_to_gltf_node_map = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->out_animation_skeleton_joint_to_gltf_node_map;
                    mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_model_space_matrices = static_cast<internal_scene_depth_first_search_traverse_context *>(user_data)->out_animation_skeleton_bind_pose_model_space_matrices;

                    uint32_t const gltf_current_node_index = cgltf_node_index(in_gltf_data, in_current_gltf_node);

                    uint32_t const gltf_parent_node_index = (NULL != in_parent_gltf_node) ? cgltf_node_index(in_gltf_data, in_parent_gltf_node) : BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;

                    if ((gltf_current_node_index != in_gltf_skeleton_root_node_index) && (in_gltf_skeleton_node_indices.end() != in_gltf_skeleton_node_indices.find(gltf_current_node_index)))
                    {
                        uint32_t const parent_joint_index = (NULL != in_parent_gltf_node) ? out_gltf_node_to_animation_skeleton_joint_map[gltf_parent_node_index] : 0U;
                        assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != parent_joint_index);

                        out_animation_skeleton_joint_parent_indices.push_back(parent_joint_index);
                        uint32_t const animation_skeleton_current_joint_index = (out_animation_skeleton_joint_parent_indices.size() - 1U);
                        assert(parent_joint_index < animation_skeleton_current_joint_index);

                        assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_gltf_node_to_animation_skeleton_joint_map[gltf_current_node_index]);
                        out_gltf_node_to_animation_skeleton_joint_map[gltf_current_node_index] = animation_skeleton_current_joint_index;

                        assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_animation_skeleton_joint_to_gltf_node_map[animation_skeleton_current_joint_index]);
                        out_animation_skeleton_joint_to_gltf_node_map[animation_skeleton_current_joint_index] = gltf_current_node_index;

                        out_animation_skeleton_bind_pose_model_space_matrices.push_back(in_gltf_node_bind_pose_model_space_matrices[gltf_current_node_index]);
                    }
                },
                &scene_depth_first_search_traverse_context);
        }

#ifndef NDEBUG
        for (uint32_t skeleton_joint_index = 0U; skeleton_joint_index < out_animation_skeleton_joint_parent_indices.size(); ++skeleton_joint_index)
        {
            assert((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_animation_skeleton_joint_parent_indices[skeleton_joint_index]) || (out_animation_skeleton_joint_parent_indices[skeleton_joint_index] < skeleton_joint_index));
        }
#endif

        assert(out_animation_skeleton_joint_parent_indices.size() == ((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != gltf_skeleton_root_node_index) ? gltf_skeleton_node_indices.size() : (gltf_skeleton_node_indices.size() + 1U)));
        assert(out_animation_skeleton_bind_pose_model_space_matrices.size() == ((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != gltf_skeleton_root_node_index) ? gltf_skeleton_node_indices.size() : (gltf_skeleton_node_indices.size() + 1U)));
    }

    uint32_t const animation_skeleton_joint_count = out_animation_skeleton_bind_pose_model_space_matrices.size();
    assert(animation_skeleton_joint_count == out_animation_skeleton_joint_parent_indices.size());

    assert(out_animation_skeleton_joint_names.empty() || (1U == out_animation_skeleton_joint_names.size()));
    uint32_t const animation_skeleton_joint_begin = out_animation_skeleton_joint_names.size();
    out_animation_skeleton_joint_names.resize(animation_skeleton_joint_count);
    {
        if (found_vrmc_vrm)
        {
            for (uint32_t current_animation_skeleton_joint_index = animation_skeleton_joint_begin; current_animation_skeleton_joint_index < animation_skeleton_joint_count; ++current_animation_skeleton_joint_index)
            {
                uint32_t const current_model_node_index = animation_skeleton_joint_to_gltf_node_map[current_animation_skeleton_joint_index];

                auto const found_vrmc_vrm_human_bone = vrmc_vrm_human_bones.find(current_model_node_index);
                if (vrmc_vrm_human_bones.end() != found_vrmc_vrm_human_bone)
                {
                    out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = found_vrmc_vrm_human_bone->second;
                }
                else
                {
                    out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID;
                }
            }
        }
        else if (found_vrm)
        {
            for (uint32_t current_animation_skeleton_joint_index = animation_skeleton_joint_begin; current_animation_skeleton_joint_index < animation_skeleton_joint_count; ++current_animation_skeleton_joint_index)
            {
                uint32_t const current_model_node_index = animation_skeleton_joint_to_gltf_node_map[current_animation_skeleton_joint_index];

                auto const found_vrm_human_bone = vrm_human_bones.find(current_model_node_index);
                if (vrm_human_bones.end() != found_vrm_human_bone)
                {
                    out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = found_vrm_human_bone->second;
                }
                else
                {
                    out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID;
                }
            }
        }
        else
        {
            mcrt_unordered_map<mcrt_string, uint32_t> mmd_skeleton_joint_indices;

            for (uint32_t current_animation_skeleton_joint_index = animation_skeleton_joint_begin; current_animation_skeleton_joint_index < animation_skeleton_joint_count; ++current_animation_skeleton_joint_index)
            {
                out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID;

                uint32_t const current_model_node_index = animation_skeleton_joint_to_gltf_node_map[current_animation_skeleton_joint_index];

                char *const current_model_node_name = in_gltf_data->nodes[current_model_node_index].name;
                cgltf_decode_string(current_model_node_name);

                auto const found_mmd_skeleton_joint_index = mmd_skeleton_joint_indices.find(current_model_node_name);

                if (mmd_skeleton_joint_indices.end() == found_mmd_skeleton_joint_index)
                {
                    mmd_skeleton_joint_indices.emplace_hint(found_mmd_skeleton_joint_index, current_model_node_name, current_animation_skeleton_joint_index);
                }
                else
                {
                    assert(false);
                }
            }

            mcrt_vector<mcrt_vector<mcrt_string>> mmd_skeleton_joint_name_strings(static_cast<size_t>(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT));
            internal_fill_mmd_skeleton_joint_name_strings(mmd_skeleton_joint_name_strings);

            for (uint32_t mmd_skeleton_joint_name = 0U; mmd_skeleton_joint_name < BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT; ++mmd_skeleton_joint_name)
            {
                for (mcrt_string const &mmd_skeleton_joint_name_string : mmd_skeleton_joint_name_strings[mmd_skeleton_joint_name])
                {
                    auto const found_mmd_name_and_skeleton_joint = mmd_skeleton_joint_indices.find(mmd_skeleton_joint_name_string);
                    if (mmd_skeleton_joint_indices.end() != found_mmd_name_and_skeleton_joint)
                    {
                        uint32_t const current_animation_skeleton_joint_index = found_mmd_name_and_skeleton_joint->second;
                        assert(current_animation_skeleton_joint_index < out_animation_skeleton_joint_names.size());
                        assert(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID == out_animation_skeleton_joint_names[current_animation_skeleton_joint_index]);
                        out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = static_cast<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME>(mmd_skeleton_joint_name);
                        break;
                    }
                }
            }
        }
    }

    assert(out_animation_skeleton_bind_pose_local_space_matrices.empty() || (1U == out_animation_skeleton_bind_pose_local_space_matrices.size()));
    out_animation_skeleton_bind_pose_local_space_matrices.resize(animation_skeleton_joint_count);
    {
        for (uint32_t current_animation_skeleton_joint_index = animation_skeleton_joint_begin; current_animation_skeleton_joint_index < animation_skeleton_joint_count; ++current_animation_skeleton_joint_index)
        {
            uint32_t const parent_animation_skeleton_joint_index = out_animation_skeleton_joint_parent_indices[current_animation_skeleton_joint_index];
            if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != parent_animation_skeleton_joint_index)
            {
                assert(parent_animation_skeleton_joint_index < current_animation_skeleton_joint_index);

                DirectX::XMVECTOR unused_determinant;
                DirectX::XMStoreFloat4x4(&out_animation_skeleton_bind_pose_local_space_matrices[current_animation_skeleton_joint_index], DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&out_animation_skeleton_bind_pose_model_space_matrices[current_animation_skeleton_joint_index]), DirectX::XMMatrixInverse(&unused_determinant, DirectX::XMLoadFloat4x4(&out_animation_skeleton_bind_pose_model_space_matrices[parent_animation_skeleton_joint_index]))));
            }
            else
            {
                out_animation_skeleton_bind_pose_local_space_matrices[current_animation_skeleton_joint_index] = out_animation_skeleton_bind_pose_model_space_matrices[current_animation_skeleton_joint_index];
            }
        }
    }
}

static inline void internal_import_ragdoll_physics(cgltf_data const *in_gltf_data, mcrt_vector<uint32_t> const &in_animation_skeleton_joint_parent_indices, mcrt_vector<uint32_t> const &in_gltf_node_to_animation_skeleton_joint_map, mcrt_vector<DirectX::XMFLOAT4X4> const &in_animation_skeleton_bind_pose_model_space, mcrt_vector<brx_asset_import_physics_rigid_body> &out_ragdoll_skeleton_rigid_bodies, mcrt_vector<brx_asset_import_physics_constraint> &out_ragdoll_skeleton_constraints, mcrt_vector<uint32_t> &out_ragdoll_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_animation_to_ragdoll_direct_mapping, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_ragdoll_to_animation_direct_mapping)
{
    bool found_vrmc_vrm = false;

    bool found_vrm = false;
    mcrt_vector<internal_vrm_collider_group_t> vrm_collider_groups;
    mcrt_vector<internal_vrm_bone_group_t> vrm_bone_groups;

    for (uint32_t data_extension_index = 0U; data_extension_index < in_gltf_data->data_extensions_count; ++data_extension_index)
    {
        cgltf_extension const *const gltf_data_extension = in_gltf_data->data_extensions + data_extension_index;
        if ((NULL != gltf_data_extension->name) && (0 == std::strcmp("VRMC_vrm", gltf_data_extension->name)) && (NULL != gltf_data_extension->data))
        {
            // TODO
        }
        else if ((NULL != gltf_data_extension->name) && (0 == std::strcmp("VRM", gltf_data_extension->name)) && (NULL != gltf_data_extension->data))
        {
            size_t const gltf_data_extension_data_length = std::strlen(gltf_data_extension->data);

            jsmn_parser parser = {0, 0, 0};

            int const option_token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, NULL, 0U);

            jsmn_init(&parser);

            jsmntok_t *const tokens = static_cast<jsmntok_t *>(mcrt_malloc(sizeof(jsmntok_t) * (option_token_count + 1), alignof(jsmntok_t)));
            assert(NULL != tokens);

            int const token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, tokens, option_token_count);
            assert(option_token_count == token_count);

            tokens[token_count].type = JSMN_UNDEFINED;

            assert(!found_vrm);
            found_vrm = true;

            internal_parse_vrm_spring_bones(tokens, token_count, gltf_data_extension->data, vrm_collider_groups, vrm_bone_groups);

            mcrt_free(tokens);

            break;
        }
    }

    assert(out_ragdoll_skeleton_rigid_bodies.empty());
    out_ragdoll_skeleton_rigid_bodies = {};

    assert(out_ragdoll_skeleton_constraints.empty());
    out_ragdoll_skeleton_constraints = {};

    assert(out_ragdoll_skeleton_joint_parent_indices.empty());
    out_ragdoll_skeleton_joint_parent_indices = {};

    assert(out_animation_to_ragdoll_direct_mapping.empty());
    out_animation_to_ragdoll_direct_mapping = {};

    assert(out_ragdoll_to_animation_direct_mapping.empty());
    out_ragdoll_to_animation_direct_mapping = {};

    if (found_vrmc_vrm)
    {
        //
    }
    else if (found_vrm)
    {
        uint32_t const vrm_collider_group_count = vrm_collider_groups.size();
        uint32_t const vrm_bone_group_count = vrm_bone_groups.size();
        assert((vrm_collider_group_count + vrm_bone_group_count) < 16U);

        for (uint32_t vrm_collider_group_index = 0U; vrm_collider_group_index < vrm_collider_group_count; ++vrm_collider_group_index)
        {
            uint32_t const animation_skeleton_current_joint_index = in_gltf_node_to_animation_skeleton_joint_map[vrm_collider_groups[vrm_collider_group_index].m_node_index];

            for (auto const &vrm_collider : vrm_collider_groups[vrm_collider_group_index].m_colliders)
            {
                DirectX::XMFLOAT4X4 animation_to_ragdoll_transform_model_space;
                {
                    DirectX::XMFLOAT3 animation_to_ragdoll_translation_model_space = internal_transform_translation(vrm_collider.m_offset, found_vrmc_vrm, found_vrm);

                    DirectX::XMStoreFloat4x4(&animation_to_ragdoll_transform_model_space, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&animation_to_ragdoll_translation_model_space)));
                }

                DirectX::XMFLOAT3 ragdoll_translation_model_space;
                DirectX::XMFLOAT4 ragdoll_rotation_model_space;
                {
                    DirectX::XMMATRIX ragdoll_transform_model_space = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&animation_to_ragdoll_transform_model_space), DirectX::XMLoadFloat4x4(&in_animation_skeleton_bind_pose_model_space[animation_skeleton_current_joint_index]));

                    DirectX::XMVECTOR simd_ragdoll_model_space_translation;
                    DirectX::XMVECTOR simd_ragdoll_model_space_scale;
                    DirectX::XMVECTOR simd_ragdoll_model_space_rotation;
                    bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&simd_ragdoll_model_space_scale, &simd_ragdoll_model_space_rotation, &simd_ragdoll_model_space_translation, ragdoll_transform_model_space);
                    assert(directx_xm_matrix_decompose);

                    constexpr float const INTERNAL_SCALE_EPSILON = 1E-7F;
                    assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(simd_ragdoll_model_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));

                    DirectX::XMStoreFloat3(&ragdoll_translation_model_space, simd_ragdoll_model_space_translation);
                    DirectX::XMStoreFloat4(&ragdoll_rotation_model_space, simd_ragdoll_model_space_rotation);
                }

                BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_TYPE const rigid_body_shape_type = BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_SPHERE;
                float const rigid_body_sphere_radius = vrm_collider.m_radius;
                BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_TYPE const rigid_body_motion_type = BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_KEYFRAME;
                uint32_t const rigid_body_collision_filter_group = vrm_collider_group_index;
                float const rigid_body_mass = 0.0F;
                float const rigid_body_linear_damping = 0.0F;
                float const rigid_body_angular_damping = 0.0F;
                float const rigid_body_friction = 0.0F;
                float const rigid_body_restitution = 0.0F;

                uint32_t rigid_body_collision_filter_mask = 0U;
                for (uint32_t vrm_bone_group_index = 0U; vrm_bone_group_index < vrm_bone_group_count; ++vrm_bone_group_index)
                {
                    uint32_t const vrm_bone_group_collision_filter_group = vrm_collider_group_count + vrm_bone_group_index;

                    for (uint32_t const vrm_collider_group_collision_filter_group : vrm_bone_groups[vrm_bone_group_index].m_collider_groups)
                    {
                        if (rigid_body_collision_filter_group == vrm_collider_group_collision_filter_group)
                        {
                            rigid_body_collision_filter_mask |= (1U << vrm_bone_group_collision_filter_group);
                            break;
                        }
                    }
                }

                uint32_t const ragdoll_skeleton_current_joint_index = out_ragdoll_skeleton_rigid_bodies.size();

                out_ragdoll_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

                out_ragdoll_skeleton_rigid_bodies.push_back(
                    brx_asset_import_physics_rigid_body{
                        {{
                             ragdoll_rotation_model_space.x,
                             ragdoll_rotation_model_space.y,
                             ragdoll_rotation_model_space.z,
                             ragdoll_rotation_model_space.w,

                         },
                         {ragdoll_translation_model_space.x,
                          ragdoll_translation_model_space.y,
                          ragdoll_translation_model_space.z}},
                        rigid_body_shape_type,
                        {
                            rigid_body_sphere_radius,
                            0.0F,
                            0.0F,
                        },
                        rigid_body_motion_type,
                        rigid_body_collision_filter_group,
                        rigid_body_collision_filter_mask,
                        rigid_body_mass,
                        rigid_body_linear_damping,
                        rigid_body_angular_damping,
                        rigid_body_friction,
                        rigid_body_restitution});

                out_animation_to_ragdoll_direct_mapping.push_back(
                    brx_asset_import_ragdoll_direct_mapping{
                        animation_skeleton_current_joint_index,
                        ragdoll_skeleton_current_joint_index,
                        {{
                             animation_to_ragdoll_transform_model_space.m[0][0],
                             animation_to_ragdoll_transform_model_space.m[0][1],
                             animation_to_ragdoll_transform_model_space.m[0][2],
                             animation_to_ragdoll_transform_model_space.m[0][3],
                         },
                         {
                             animation_to_ragdoll_transform_model_space.m[1][0],
                             animation_to_ragdoll_transform_model_space.m[1][1],
                             animation_to_ragdoll_transform_model_space.m[1][2],
                             animation_to_ragdoll_transform_model_space.m[1][3],
                         },
                         {
                             animation_to_ragdoll_transform_model_space.m[2][0],
                             animation_to_ragdoll_transform_model_space.m[2][1],
                             animation_to_ragdoll_transform_model_space.m[2][2],
                             animation_to_ragdoll_transform_model_space.m[2][3],
                         },
                         {
                             animation_to_ragdoll_transform_model_space.m[3][0],
                             animation_to_ragdoll_transform_model_space.m[3][1],
                             animation_to_ragdoll_transform_model_space.m[3][2],
                             animation_to_ragdoll_transform_model_space.m[3][3],
                         }}});
            }
        }

        for (uint32_t vrm_bone_group_index = 0U; vrm_bone_group_index < vrm_bone_group_count; ++vrm_bone_group_index)
        {
            uint32_t const animation_skeleton_joint_count = in_animation_skeleton_joint_parent_indices.size();

            mcrt_vector<uint32_t> animation_skeleton_joint_to_ragdoll_rigid_body_map(static_cast<size_t>(animation_skeleton_joint_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
            mcrt_vector<uint32_t> animation_skeleton_joint_depth_first_search_stack;
            {
                mcrt_set<uint32_t> vrm_bone_group_bone_animation_skeleton_parent_joint_indices;

                mcrt_vector<uint32_t> animation_skeleton_joint_depth_first_search_reverse_stack;

                for (uint32_t const vrm_bone_group_bone_index : vrm_bone_groups[vrm_bone_group_index].m_bones)
                {
                    uint32_t const animation_skeleton_current_joint_index = in_gltf_node_to_animation_skeleton_joint_map[vrm_bone_group_bone_index];

                    uint32_t const animation_skeleton_parent_joint_index = in_animation_skeleton_joint_parent_indices[animation_skeleton_current_joint_index];

                    if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != animation_skeleton_parent_joint_index)
                    {
                        auto found_vrm_bone_group_bone_animation_skeleton_parent_joint_index = vrm_bone_group_bone_animation_skeleton_parent_joint_indices.find(animation_skeleton_parent_joint_index);

                        if (vrm_bone_group_bone_animation_skeleton_parent_joint_indices.end() == found_vrm_bone_group_bone_animation_skeleton_parent_joint_index)
                        {
                            DirectX::XMFLOAT4X4 animation_to_ragdoll_transform_model_space;
                            {
                                DirectX::XMStoreFloat4x4(&animation_to_ragdoll_transform_model_space, DirectX::XMMatrixIdentity());
                            }

                            DirectX::XMFLOAT3 ragdoll_translation_model_space;
                            DirectX::XMFLOAT4 ragdoll_rotation_model_space;
                            {
                                DirectX::XMMATRIX ragdoll_transform_model_space = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&animation_to_ragdoll_transform_model_space), DirectX::XMLoadFloat4x4(&in_animation_skeleton_bind_pose_model_space[animation_skeleton_parent_joint_index]));

                                DirectX::XMVECTOR simd_ragdoll_model_space_translation;
                                DirectX::XMVECTOR simd_ragdoll_model_space_scale;
                                DirectX::XMVECTOR simd_ragdoll_model_space_rotation;
                                bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&simd_ragdoll_model_space_scale, &simd_ragdoll_model_space_rotation, &simd_ragdoll_model_space_translation, ragdoll_transform_model_space);
                                assert(directx_xm_matrix_decompose);

                                constexpr float const INTERNAL_SCALE_EPSILON = 1E-7F;
                                assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(simd_ragdoll_model_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));

                                DirectX::XMStoreFloat3(&ragdoll_translation_model_space, simd_ragdoll_model_space_translation);
                                DirectX::XMStoreFloat4(&ragdoll_rotation_model_space, simd_ragdoll_model_space_rotation);
                            }

                            BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_TYPE const rigid_body_shape_type = BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_SPHERE;
                            float const rigid_body_sphere_radius = vrm_bone_groups[vrm_bone_group_index].m_hit_radius;
                            BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_TYPE const rigid_body_motion_type = BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_KEYFRAME;
                            uint32_t const rigid_body_collision_filter_group = vrm_collider_group_count + vrm_bone_group_index;
                            float const rigid_body_mass = 0.0F;
                            float const rigid_body_linear_damping = 0.0F;
                            float const rigid_body_angular_damping = 0.0F;
                            float const rigid_body_friction = 0.0F;
                            float const rigid_body_restitution = 0.0F;

                            uint32_t rigid_body_collision_filter_mask = 0U;
                            for (uint32_t const vrm_collider_group_collision_filter_group : vrm_bone_groups[vrm_bone_group_index].m_collider_groups)
                            {
                                rigid_body_collision_filter_mask |= (1U << vrm_collider_group_collision_filter_group);
                            }

                            uint32_t const ragdoll_skeleton_current_joint_index = out_ragdoll_skeleton_rigid_bodies.size();

                            out_ragdoll_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

                            assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == animation_skeleton_joint_to_ragdoll_rigid_body_map[animation_skeleton_parent_joint_index]);
                            animation_skeleton_joint_to_ragdoll_rigid_body_map[animation_skeleton_parent_joint_index] = ragdoll_skeleton_current_joint_index;

                            out_ragdoll_skeleton_rigid_bodies.push_back(
                                brx_asset_import_physics_rigid_body{
                                    {{
                                         ragdoll_rotation_model_space.x,
                                         ragdoll_rotation_model_space.y,
                                         ragdoll_rotation_model_space.z,
                                         ragdoll_rotation_model_space.w,

                                     },
                                     {ragdoll_translation_model_space.x,
                                      ragdoll_translation_model_space.y,
                                      ragdoll_translation_model_space.z}},
                                    rigid_body_shape_type,
                                    {
                                        rigid_body_sphere_radius,
                                        0.0F,
                                        0.0F,
                                    },
                                    rigid_body_motion_type,
                                    rigid_body_collision_filter_group,
                                    rigid_body_collision_filter_mask,
                                    rigid_body_mass,
                                    rigid_body_linear_damping,
                                    rigid_body_angular_damping,
                                    rigid_body_friction,
                                    rigid_body_restitution});

                            out_animation_to_ragdoll_direct_mapping.push_back(
                                brx_asset_import_ragdoll_direct_mapping{
                                    animation_skeleton_parent_joint_index,
                                    ragdoll_skeleton_current_joint_index,
                                    {{
                                         animation_to_ragdoll_transform_model_space.m[0][0],
                                         animation_to_ragdoll_transform_model_space.m[0][1],
                                         animation_to_ragdoll_transform_model_space.m[0][2],
                                         animation_to_ragdoll_transform_model_space.m[0][3],
                                     },
                                     {
                                         animation_to_ragdoll_transform_model_space.m[1][0],
                                         animation_to_ragdoll_transform_model_space.m[1][1],
                                         animation_to_ragdoll_transform_model_space.m[1][2],
                                         animation_to_ragdoll_transform_model_space.m[1][3],
                                     },
                                     {
                                         animation_to_ragdoll_transform_model_space.m[2][0],
                                         animation_to_ragdoll_transform_model_space.m[2][1],
                                         animation_to_ragdoll_transform_model_space.m[2][2],
                                         animation_to_ragdoll_transform_model_space.m[2][3],
                                     },
                                     {
                                         animation_to_ragdoll_transform_model_space.m[3][0],
                                         animation_to_ragdoll_transform_model_space.m[3][1],
                                         animation_to_ragdoll_transform_model_space.m[3][2],
                                         animation_to_ragdoll_transform_model_space.m[3][3],
                                     }}});

                            vrm_bone_group_bone_animation_skeleton_parent_joint_indices.emplace(animation_skeleton_parent_joint_index);
                        }

                        animation_skeleton_joint_depth_first_search_reverse_stack.push_back(animation_skeleton_current_joint_index);
                    }
                    else
                    {
                        assert(false);
                    }
                }

                animation_skeleton_joint_depth_first_search_stack.reserve(animation_skeleton_joint_depth_first_search_reverse_stack.size());

                for (auto animation_skeleton_joint_index_reverse_iterator = animation_skeleton_joint_depth_first_search_reverse_stack.crbegin(); animation_skeleton_joint_depth_first_search_reverse_stack.crend() != animation_skeleton_joint_index_reverse_iterator; ++animation_skeleton_joint_index_reverse_iterator)
                {
                    animation_skeleton_joint_depth_first_search_stack.push_back(*animation_skeleton_joint_index_reverse_iterator);
                }

                assert(animation_skeleton_joint_depth_first_search_stack.size() == animation_skeleton_joint_depth_first_search_reverse_stack.size());
            }

            mcrt_vector<mcrt_vector<uint32_t>> animation_skeleton_joint_children_indices(static_cast<size_t>(animation_skeleton_joint_count));

            for (uint32_t animation_skeleton_joint_plus_1 = animation_skeleton_joint_count; animation_skeleton_joint_plus_1 > 0U; --animation_skeleton_joint_plus_1)
            {
                uint32_t const animation_skeleton_joint_index = animation_skeleton_joint_plus_1 - 1U;

                uint32_t animation_skeleton_parent_joint_index = in_animation_skeleton_joint_parent_indices[animation_skeleton_joint_index];

                if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != animation_skeleton_parent_joint_index)
                {
                    animation_skeleton_joint_children_indices[animation_skeleton_parent_joint_index].push_back(animation_skeleton_joint_index);
                }
            }

            mcrt_vector<bool> animation_skeleton_joint_visited_flags(static_cast<size_t>(animation_skeleton_joint_count), false);
            mcrt_vector<bool> animation_skeleton_joint_pushed_flags(static_cast<size_t>(animation_skeleton_joint_count), false);
            while (!animation_skeleton_joint_depth_first_search_stack.empty())
            {
                uint32_t const animation_skeleton_current_joint_index = animation_skeleton_joint_depth_first_search_stack.back();
                animation_skeleton_joint_depth_first_search_stack.pop_back();

                assert(!animation_skeleton_joint_visited_flags[animation_skeleton_current_joint_index]);
                animation_skeleton_joint_visited_flags[animation_skeleton_current_joint_index] = true;

                uint32_t const animation_skeleton_parent_joint_index = in_animation_skeleton_joint_parent_indices[animation_skeleton_current_joint_index];

                if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != animation_skeleton_parent_joint_index)
                {
                    DirectX::XMFLOAT4X4 ragdoll_to_animation_transform_model_space;
                    {
                        DirectX::XMStoreFloat4x4(&ragdoll_to_animation_transform_model_space, DirectX::XMMatrixIdentity());
                    }

                    DirectX::XMFLOAT3 ragdoll_translation_model_space;
                    DirectX::XMFLOAT4 ragdoll_rotation_model_space;
                    {
                        DirectX::XMVECTOR unused_determinant;
                        DirectX::XMMATRIX ragdoll_transform_model_space = DirectX::XMMatrixMultiply(DirectX::XMMatrixInverse(&unused_determinant, DirectX::XMLoadFloat4x4(&ragdoll_to_animation_transform_model_space)), DirectX::XMLoadFloat4x4(&in_animation_skeleton_bind_pose_model_space[animation_skeleton_current_joint_index]));

                        DirectX::XMVECTOR simd_ragdoll_model_space_translation;
                        DirectX::XMVECTOR simd_ragdoll_model_space_scale;
                        DirectX::XMVECTOR simd_ragdoll_model_space_rotation;
                        bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&simd_ragdoll_model_space_scale, &simd_ragdoll_model_space_rotation, &simd_ragdoll_model_space_translation, ragdoll_transform_model_space);
                        assert(directx_xm_matrix_decompose);

                        constexpr float const INTERNAL_SCALE_EPSILON = 1E-7F;
                        assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(simd_ragdoll_model_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));

                        DirectX::XMStoreFloat3(&ragdoll_translation_model_space, simd_ragdoll_model_space_translation);
                        DirectX::XMStoreFloat4(&ragdoll_rotation_model_space, simd_ragdoll_model_space_rotation);
                    }

                    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_TYPE const rigid_body_shape_type = BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_SPHERE;
                    float const rigid_body_sphere_radius = vrm_bone_groups[vrm_bone_group_index].m_hit_radius;
                    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_TYPE const rigid_body_motion_type = BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_DYNAMIC;
                    uint32_t const rigid_body_collision_filter_group = vrm_collider_group_count + vrm_bone_group_index;
                    float const rigid_body_mass = 1.0F;
                    float const rigid_body_linear_damping = vrm_bone_groups[vrm_bone_group_index].m_drag_force;
                    float const rigid_body_angular_damping = vrm_bone_groups[vrm_bone_group_index].m_drag_force;
                    float const rigid_body_friction = 0.0F;
                    float const rigid_body_restitution = 0.0F;

                    uint32_t rigid_body_collision_filter_mask = 0U;
                    for (uint32_t const vrm_collider_group_collision_filter_group : vrm_bone_groups[vrm_bone_group_index].m_collider_groups)
                    {
                        rigid_body_collision_filter_mask |= (1U << vrm_collider_group_collision_filter_group);
                    }

                    uint32_t const ragdoll_skeleton_current_joint_index = out_ragdoll_skeleton_rigid_bodies.size();

                    uint32_t const ragdoll_skeleton_parent_joint_index = animation_skeleton_joint_to_ragdoll_rigid_body_map[animation_skeleton_parent_joint_index];
                    assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != ragdoll_skeleton_parent_joint_index);

                    out_ragdoll_skeleton_joint_parent_indices.push_back(ragdoll_skeleton_parent_joint_index);

                    assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == animation_skeleton_joint_to_ragdoll_rigid_body_map[animation_skeleton_current_joint_index]);
                    animation_skeleton_joint_to_ragdoll_rigid_body_map[animation_skeleton_current_joint_index] = ragdoll_skeleton_current_joint_index;

                    out_ragdoll_skeleton_rigid_bodies.push_back(
                        brx_asset_import_physics_rigid_body{
                            {{
                                 ragdoll_rotation_model_space.x,
                                 ragdoll_rotation_model_space.y,
                                 ragdoll_rotation_model_space.z,
                                 ragdoll_rotation_model_space.w,

                             },
                             {ragdoll_translation_model_space.x,
                              ragdoll_translation_model_space.y,
                              ragdoll_translation_model_space.z}},
                            rigid_body_shape_type,
                            {
                                rigid_body_sphere_radius,
                                0.0F,
                                0.0F,
                            },
                            rigid_body_motion_type,
                            rigid_body_collision_filter_group,
                            rigid_body_collision_filter_mask,
                            rigid_body_mass,
                            rigid_body_linear_damping,
                            rigid_body_angular_damping,
                            rigid_body_friction,
                            rigid_body_restitution});

                    out_ragdoll_to_animation_direct_mapping.push_back(
                        brx_asset_import_ragdoll_direct_mapping{
                            ragdoll_skeleton_current_joint_index,
                            animation_skeleton_current_joint_index,
                            {{
                                 ragdoll_to_animation_transform_model_space.m[0][0],
                                 ragdoll_to_animation_transform_model_space.m[0][1],
                                 ragdoll_to_animation_transform_model_space.m[0][2],
                                 ragdoll_to_animation_transform_model_space.m[0][3],
                             },
                             {
                                 ragdoll_to_animation_transform_model_space.m[1][0],
                                 ragdoll_to_animation_transform_model_space.m[1][1],
                                 ragdoll_to_animation_transform_model_space.m[1][2],
                                 ragdoll_to_animation_transform_model_space.m[1][3],
                             },
                             {
                                 ragdoll_to_animation_transform_model_space.m[2][0],
                                 ragdoll_to_animation_transform_model_space.m[2][1],
                                 ragdoll_to_animation_transform_model_space.m[2][2],
                                 ragdoll_to_animation_transform_model_space.m[2][3],
                             },
                             {
                                 ragdoll_to_animation_transform_model_space.m[3][0],
                                 ragdoll_to_animation_transform_model_space.m[3][1],
                                 ragdoll_to_animation_transform_model_space.m[3][2],
                                 ragdoll_to_animation_transform_model_space.m[3][3],
                             }}});

                    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_TYPE const brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_RAGDOLL;
                    float const brx_twist_limit[2] = {-0.0F, 0.0F};
                    float const brx_plane_limit[2] = {-DirectX::XM_PIDIV4, DirectX::XM_PIDIV4};
                    float const brx_normal_limit[2] = {-DirectX::XM_PIDIV4, DirectX::XM_PIDIV4};

                    assert(ragdoll_skeleton_parent_joint_index < out_ragdoll_skeleton_rigid_bodies.size());
                    DirectX::XMFLOAT3 brx_pivot;
                    {
                        DirectX::XMFLOAT3 const constraint_translation_model_space(
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_translation[0],
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_translation[1],
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_translation[2]);

                        brx_pivot = constraint_translation_model_space;
                    }

                    DirectX::XMFLOAT3 brx_twist_axis;
                    DirectX::XMFLOAT3 brx_plane_axis;
                    DirectX::XMFLOAT3 brx_normal_axis;
                    {
                        DirectX::XMFLOAT4 const constraint_rotation_model_space(
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_rotation[0],
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_rotation[1],
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_rotation[2],
                            out_ragdoll_skeleton_rigid_bodies[ragdoll_skeleton_current_joint_index].m_model_space_transform.m_rotation[3]);

                        DirectX::XMMATRIX constraint_rotation_matrix_model_space = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&constraint_rotation_model_space));

                        DirectX::XMFLOAT3 local_axis_x(1.0F, 0.0F, 0.0F);
                        DirectX::XMStoreFloat3(&brx_twist_axis, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&local_axis_x), constraint_rotation_matrix_model_space));

                        DirectX::XMFLOAT3 local_axis_y(0.0F, 1.0F, 0.0F);
                        DirectX::XMStoreFloat3(&brx_plane_axis, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&local_axis_y), constraint_rotation_matrix_model_space));

                        DirectX::XMFLOAT3 local_axis_z(0.0F, 0.0F, 1.0F);
                        DirectX::XMStoreFloat3(&brx_normal_axis, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&local_axis_z), constraint_rotation_matrix_model_space));
                    }

                    out_ragdoll_skeleton_constraints.push_back(
                        brx_asset_import_physics_constraint{
                            ragdoll_skeleton_parent_joint_index,
                            ragdoll_skeleton_current_joint_index,
                            brx_constraint_type,
                            {
                                brx_pivot.x,
                                brx_pivot.y,
                                brx_pivot.z,
                            },
                            {
                                brx_twist_axis.x,
                                brx_twist_axis.y,
                                brx_twist_axis.z,
                            },
                            {
                                brx_plane_axis.x,
                                brx_plane_axis.y,
                                brx_plane_axis.z,
                            },
                            {
                                brx_normal_axis.x,
                                brx_normal_axis.y,
                                brx_normal_axis.z,
                            },
                            {
                                brx_twist_limit[0],
                                brx_twist_limit[1],
                            },
                            {
                                brx_plane_limit[0],
                                brx_plane_limit[1],
                            },
                            {
                                brx_normal_limit[0],
                                brx_normal_limit[1],
                            }});
                }
                else
                {
                    assert(false);
                }

                for (uint32_t animation_skeleton_joint_child_index_index_plus_1 = static_cast<uint32_t>(animation_skeleton_joint_children_indices[animation_skeleton_current_joint_index].size()); animation_skeleton_joint_child_index_index_plus_1 > 0U; --animation_skeleton_joint_child_index_index_plus_1)
                {
                    uint32_t const animation_skeleton_joint_child_index = animation_skeleton_joint_children_indices[animation_skeleton_current_joint_index][animation_skeleton_joint_child_index_index_plus_1 - 1U];

                    if ((!animation_skeleton_joint_visited_flags[animation_skeleton_joint_child_index]) && (!animation_skeleton_joint_pushed_flags[animation_skeleton_joint_child_index]))
                    {
                        animation_skeleton_joint_pushed_flags[animation_skeleton_joint_child_index] = true;
                        animation_skeleton_joint_depth_first_search_stack.push_back(animation_skeleton_joint_child_index);
                    }
                    else
                    {
                        assert(false);
                    }
                }
            }
        }
    }

    assert(out_ragdoll_skeleton_rigid_bodies.size() == out_ragdoll_skeleton_joint_parent_indices.size());
    assert(out_ragdoll_to_animation_direct_mapping.size() == out_ragdoll_skeleton_constraints.size());
    assert((out_animation_to_ragdoll_direct_mapping.size() + out_ragdoll_to_animation_direct_mapping.size()) == out_ragdoll_skeleton_rigid_bodies.size());
}

static inline void internal_import_surface(cgltf_data const *in_gltf_data, mcrt_vector<DirectX::XMFLOAT4X4> const &in_gltf_node_model_space_matrices, mcrt_vector<mcrt_set<uint32_t>> const &in_gltf_meshes_instance_node_indices, mcrt_vector<uint32_t> const &in_gltf_node_to_animation_skeleton_joint_map, mcrt_vector<std::pair<uint32_t, internal_vrm_mesh_section_t>> &out_materials_and_mesh_sections)
{
    assert(in_gltf_node_model_space_matrices.size() == in_gltf_data->nodes_count);

    cgltf_material const *const gltf_materials = in_gltf_data->materials;

    // TODO: we may not merge the surface since the alpha blending depends on the vertex order?
    auto const material_compare = [gltf_materials](uint32_t const &lhs, uint32_t const &rhs) -> bool
    {
        return lhs < rhs;
    };

    enum
    {
        MATERIAL_MESH_SECTION_VECTOR_SKIN_MORPH_INDEX = 0,
        MATERIAL_MESH_SECTION_VECTOR_SKIN_NON_MORPH_INDEX = 1,
        MATERIAL_MESH_SECTION_VECTOR_NON_SKIN_MORPH_INDEX = 2,
        MATERIAL_MESH_SECTION_VECTOR_NON_SKIN_NON_MORPH_INDEX = 3,
        MATERIAL_MESH_SECTION_VECTOR_SIZE = 4
    };

    mcrt_map<uint32_t, std::array<internal_vrm_mesh_section_t, MATERIAL_MESH_SECTION_VECTOR_SIZE>, decltype(material_compare)> materials_mesh_section_vector(material_compare);

    bool found_vrmc_vrm = false;

    mcrt_map<uint32_t, mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_map<uint32_t, float>>> vrmc_vrm_nodes_expressions_morph_target_binds;

    bool found_vrm = false;

    mcrt_map<uint32_t, mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_map<uint32_t, float>>> vrm_meshes_blend_shapes_morph_target_binds;

    for (uint32_t data_extension_index = 0U; data_extension_index < in_gltf_data->data_extensions_count; ++data_extension_index)
    {
        cgltf_extension const *const gltf_data_extension = in_gltf_data->data_extensions + data_extension_index;
        if ((NULL != gltf_data_extension->name) && (0 == std::strcmp("VRMC_vrm", gltf_data_extension->name)) && (NULL != gltf_data_extension->data))
        {
            size_t const gltf_data_extension_data_length = std::strlen(gltf_data_extension->data);

            jsmn_parser parser = {0, 0, 0};

            int const option_token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, NULL, 0U);

            jsmn_init(&parser);

            jsmntok_t *const tokens = static_cast<jsmntok_t *>(mcrt_malloc(sizeof(jsmntok_t) * (option_token_count + 1), alignof(jsmntok_t)));
            assert(NULL != tokens);

            int const token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, tokens, option_token_count);
            assert(option_token_count == token_count);

            tokens[token_count].type = JSMN_UNDEFINED;

            assert(!found_vrmc_vrm);
            found_vrmc_vrm = true;

            internal_parse_vrmc_vrm_morph_target_names(tokens, token_count, gltf_data_extension->data, vrmc_vrm_nodes_expressions_morph_target_binds);

            mcrt_free(tokens);

            break;
        }
        else if ((NULL != gltf_data_extension->name) && (0 == std::strcmp("VRM", gltf_data_extension->name)) && (NULL != gltf_data_extension->data))
        {
            size_t const gltf_data_extension_data_length = std::strlen(gltf_data_extension->data);

            jsmn_parser parser = {0, 0, 0};

            int const option_token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, NULL, 0U);

            jsmn_init(&parser);

            jsmntok_t *const tokens = static_cast<jsmntok_t *>(mcrt_malloc(sizeof(jsmntok_t) * (option_token_count + 1), alignof(jsmntok_t)));
            assert(NULL != tokens);

            int const token_count = jsmn_parse(&parser, gltf_data_extension->data, gltf_data_extension_data_length, tokens, option_token_count);
            assert(option_token_count == token_count);

            tokens[token_count].type = JSMN_UNDEFINED;

            assert(!found_vrm);
            found_vrm = true;

            internal_parse_vrm_morph_target_names(tokens, token_count, gltf_data_extension->data, vrm_meshes_blend_shapes_morph_target_binds);

            mcrt_free(tokens);

            break;
        }
    }

    // [glTF Validator: NODE_SKINNED_MESH_NON_ROOT](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L444)
    // [glTF Validator: NODE_SKINNED_MESH_LOCAL_TRANSFORMS](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L450)
    // skin mesh can NOT be affected by the node transform
    // but can be affected by the joint indices of the skeleton
    mcrt_set<uint32_t> gltf_skin_mesh_indices;
    assert(in_gltf_meshes_instance_node_indices.size() == in_gltf_data->meshes_count);
    for (size_t gltf_mesh_index = 0U; gltf_mesh_index < in_gltf_data->meshes_count; ++gltf_mesh_index)
    {
        mcrt_set<uint32_t> const &gltf_mesh_instance_node_indices = in_gltf_meshes_instance_node_indices[gltf_mesh_index];

        for (uint32_t const gltf_mesh_instance_node_index : gltf_mesh_instance_node_indices)
        {
            cgltf_node const *const gltf_mesh_instance_node = in_gltf_data->nodes + gltf_mesh_instance_node_index;

            cgltf_mesh const *const gltf_mesh = gltf_mesh_instance_node->mesh;

            cgltf_skin const *const gltf_skin = gltf_mesh_instance_node->skin;

            for (uint32_t gltf_primitive_index = 0U; gltf_primitive_index < gltf_mesh->primitives_count; ++gltf_primitive_index)
            {
                cgltf_primitive const *const gltf_primitive = gltf_mesh->primitives + gltf_primitive_index;

                cgltf_material const *const gltf_material = gltf_primitive->material;

                cgltf_accessor const *gltf_position_accessor = NULL;
                {
                    // TODO: we select the smallest index but this may NOT be correct
                    cgltf_int gltf_position_index = -1;

                    for (size_t gltf_vertex_attribute_index = 0U; gltf_vertex_attribute_index < gltf_primitive->attributes_count; ++gltf_vertex_attribute_index)
                    {
                        cgltf_attribute const *const gltf_vertex_attribute = gltf_primitive->attributes + gltf_vertex_attribute_index;

                        switch (gltf_vertex_attribute->type)
                        {
                        case cgltf_attribute_type_position:
                        {
                            assert(cgltf_attribute_type_position == gltf_vertex_attribute->type);

                            if (NULL == gltf_position_accessor || gltf_vertex_attribute->index < gltf_position_index)
                            {
                                gltf_position_accessor = gltf_vertex_attribute->data;
                                gltf_position_index = gltf_vertex_attribute->index;
                            }
                        }
                        break;
                        default:
                        {
                            // Do Nothing
                        }
                        }
                    }
                }

                if (((cgltf_primitive_type_triangles == gltf_primitive->type) || (cgltf_primitive_type_triangle_strip == gltf_primitive->type) || (cgltf_primitive_type_triangle_fan == gltf_primitive->type)) && (NULL != gltf_position_accessor))
                {
                    mcrt_vector<uint32_t> raw_indices;
                    {
                        cgltf_accessor const *const gltf_index_accessor = gltf_primitive->indices;

                        if (NULL != gltf_index_accessor)
                        {
                            uint32_t const gltf_index_count = gltf_index_accessor->count;

                            mcrt_vector<uint32_t> gltf_indices(static_cast<size_t>(gltf_index_count));
                            {
                                assert(cgltf_type_scalar == gltf_index_accessor->type);

                                switch (gltf_index_accessor->component_type)
                                {
                                case cgltf_component_type_r_8u:
                                {
                                    assert(cgltf_component_type_r_8u == gltf_index_accessor->component_type);

                                    for (uint32_t gltf_index_index = 0; gltf_index_index < gltf_index_count; ++gltf_index_index)
                                    {
                                        cgltf_uint gltf_index_ubyte;
                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_index_accessor, gltf_index_index, &gltf_index_ubyte, 1U);
                                        assert(result_gltf_accessor_read_uint);

                                        assert(gltf_index_ubyte <= static_cast<cgltf_uint>(UINT8_MAX));

                                        gltf_indices[gltf_index_index] = gltf_index_ubyte;
                                    }
                                }
                                break;
                                case cgltf_component_type_r_16u:
                                {
                                    assert(cgltf_component_type_r_16u == gltf_index_accessor->component_type);

                                    for (uint32_t gltf_index_index = 0; gltf_index_index < gltf_index_count; ++gltf_index_index)
                                    {
                                        cgltf_uint gltf_index_ushort;
                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_index_accessor, gltf_index_index, &gltf_index_ushort, 1U);
                                        assert(result_gltf_accessor_read_uint);

                                        gltf_indices[gltf_index_index] = gltf_index_ushort;
                                    }
                                }
                                break;
                                case cgltf_component_type_r_32u:
                                {
                                    assert(cgltf_component_type_r_32u == gltf_index_accessor->component_type);

                                    for (uint32_t gltf_index_index = 0; gltf_index_index < gltf_index_count; ++gltf_index_index)
                                    {
                                        cgltf_uint gltf_index_uint;
                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_index_accessor, gltf_index_index, &gltf_index_uint, 1U);
                                        assert(result_gltf_accessor_read_uint);

                                        gltf_indices[gltf_index_index] = gltf_index_uint;
                                    }
                                }
                                break;
                                default:
                                    assert(0);
                                }
                            }

                            if (cgltf_primitive_type_triangles == gltf_primitive->type)
                            {
                                assert(0U == (gltf_index_count % 3U));

                                assert(raw_indices.empty());
                                raw_indices.resize(gltf_index_count);

                                for (uint32_t gltf_index_index = 0U; gltf_index_index < gltf_index_count; ++gltf_index_index)
                                {
                                    raw_indices[gltf_index_index] = gltf_indices[gltf_index_index];
                                }
                            }
                            else if (cgltf_primitive_type_triangle_strip == gltf_primitive->type)
                            {
                                assert(raw_indices.empty());
                                raw_indices.resize(3U * ((gltf_index_count > 2U) ? (gltf_index_count - 2U) : 0U));

                                // restart is NOT supported by glTF

                                for (uint32_t gltf_index_index = 0U; (gltf_index_index + 2U) < gltf_index_count; ++gltf_index_index)
                                {
                                    if (0U == (gltf_index_index & 1U))
                                    {
                                        raw_indices[3U * gltf_index_index + 0U] = gltf_indices[gltf_index_index + 0U];
                                        raw_indices[3U * gltf_index_index + 1U] = gltf_indices[gltf_index_index + 1U];
                                        raw_indices[3U * gltf_index_index + 2U] = gltf_indices[gltf_index_index + 2U];
                                    }
                                    else
                                    {
                                        raw_indices[3U * gltf_index_index + 0U] = gltf_indices[gltf_index_index + 1U];
                                        raw_indices[3U * gltf_index_index + 1U] = gltf_indices[gltf_index_index + 0U];
                                        raw_indices[3U * gltf_index_index + 2U] = gltf_indices[gltf_index_index + 2U];
                                    }
                                }
                            }
                            else if (cgltf_primitive_type_triangle_fan == gltf_primitive->type)
                            {
                                assert(raw_indices.empty());
                                raw_indices.resize(3U * ((gltf_index_count > 2U) ? (gltf_index_count - 2U) : 0U));

                                for (uint32_t gltf_index_index = 1U; (gltf_index_index + 1U) < gltf_index_count; ++gltf_index_index)
                                {
                                    raw_indices[3U * (gltf_index_index - 1U) + 0U] = gltf_indices[0U];
                                    raw_indices[3U * (gltf_index_index - 1U) + 1U] = gltf_indices[gltf_index_index + 0U];
                                    raw_indices[3U * (gltf_index_index - 1U) + 2U] = gltf_indices[gltf_index_index + 1U];
                                }
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        else
                        {
                            uint32_t const gltf_vertex_count = gltf_position_accessor->count;

                            if (cgltf_primitive_type_triangles == gltf_primitive->type)
                            {
                                assert(0U == (gltf_vertex_count % 3U));

                                assert(raw_indices.empty());
                                raw_indices.resize(gltf_vertex_count);

                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    raw_indices[gltf_vertex_index] = gltf_vertex_index;
                                }
                            }
                            else if (cgltf_primitive_type_triangle_strip == gltf_primitive->type)
                            {
                                assert(raw_indices.empty());
                                raw_indices.resize(3U * ((gltf_vertex_count > 2U) ? (gltf_vertex_count - 2U) : 0U));

                                for (uint32_t gltf_vertex_index = 0U; (gltf_vertex_index + 2U) < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    if (0U == (gltf_vertex_index & 1U))
                                    {
                                        raw_indices[3U * gltf_vertex_index + 0U] = gltf_vertex_index + 0U;
                                        raw_indices[3U * gltf_vertex_index + 1U] = gltf_vertex_index + 1U;
                                        raw_indices[3U * gltf_vertex_index + 2U] = gltf_vertex_index + 2U;
                                    }
                                    else
                                    {
                                        raw_indices[3U * gltf_vertex_index + 0U] = gltf_vertex_index + 1U;
                                        raw_indices[3U * gltf_vertex_index + 1U] = gltf_vertex_index + 0U;
                                        raw_indices[3U * gltf_vertex_index + 2U] = gltf_vertex_index + 2U;
                                    }
                                }
                            }
                            else if (cgltf_primitive_type_triangle_fan == gltf_primitive->type)
                            {
                                assert(raw_indices.empty());
                                raw_indices.resize(3U * ((gltf_vertex_count > 2U) ? (gltf_vertex_count - 2U) : 0U));

                                for (uint32_t gltf_vertex_index = 1U; (gltf_vertex_index + 1U) < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    raw_indices[3U * (gltf_vertex_index - 1U) + 0U] = 0U;
                                    raw_indices[3U * (gltf_vertex_index - 1U) + 1U] = gltf_vertex_index + 0U;
                                    raw_indices[3U * (gltf_vertex_index - 1U) + 2U] = gltf_vertex_index + 1U;
                                }
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                    }

                    mcrt_vector<internal_vrm_vertex_t> raw_vertices;
                    mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<internal_vrm_morph_target_vertex_t>> raw_morph_targets_vertices;
                    {
                        uint32_t const gltf_vertex_count = gltf_position_accessor->count;

                        mcrt_vector<DirectX::XMFLOAT3> gltf_positions;
                        mcrt_vector<DirectX::XMFLOAT3> gltf_normals;
                        mcrt_vector<DirectX::XMFLOAT4> gltf_tangents;
                        mcrt_vector<DirectX::XMFLOAT2> gltf_texcoords;
                        bool gltf_compute_normals = false;
                        bool gltf_compute_tangents = false;
                        {

                            cgltf_accessor const *gltf_normal_accessor = NULL;
                            cgltf_accessor const *gltf_tangent_accessor = NULL;
                            cgltf_accessor const *gltf_texcoord_accessor = NULL;
                            {
                                // TODO: we select the smallest index but this may NOT be correct
                                cgltf_int gltf_normal_index = -1;
                                cgltf_int gltf_tangent_index = -1;
                                cgltf_int gltf_texcoord_index = -1;

                                for (size_t gltf_vertex_attribute_index = 0U; gltf_vertex_attribute_index < gltf_primitive->attributes_count; ++gltf_vertex_attribute_index)
                                {
                                    cgltf_attribute const *const gltf_vertex_attribute = gltf_primitive->attributes + gltf_vertex_attribute_index;

                                    switch (gltf_vertex_attribute->type)
                                    {
                                    case cgltf_attribute_type_normal:
                                    {
                                        assert(cgltf_attribute_type_normal == gltf_vertex_attribute->type);

                                        if (NULL == gltf_normal_accessor || gltf_vertex_attribute->index < gltf_normal_index)
                                        {
                                            gltf_normal_accessor = gltf_vertex_attribute->data;
                                            gltf_normal_index = gltf_vertex_attribute->index;
                                        }
                                    }
                                    break;
                                    case cgltf_attribute_type_tangent:
                                    {
                                        assert(cgltf_attribute_type_tangent == gltf_vertex_attribute->type);

                                        if (NULL == gltf_tangent_accessor || gltf_vertex_attribute->index < gltf_tangent_index)
                                        {
                                            gltf_tangent_accessor = gltf_vertex_attribute->data;
                                            gltf_tangent_index = gltf_vertex_attribute->index;
                                        }
                                    }
                                    break;
                                    case cgltf_attribute_type_texcoord:
                                    {
                                        assert(cgltf_attribute_type_texcoord == gltf_vertex_attribute->type);

                                        if (NULL == gltf_texcoord_accessor || gltf_vertex_attribute->index < gltf_texcoord_index)
                                        {
                                            gltf_texcoord_accessor = gltf_vertex_attribute->data;
                                            gltf_texcoord_index = gltf_vertex_attribute->index;
                                        }
                                    }
                                    break;
                                    default:
                                    {
                                        // Do Nothing
                                    }
                                    }
                                }
                            }

                            assert(gltf_positions.empty());
                            gltf_positions.resize(gltf_vertex_count);

                            {
                                assert(cgltf_type_vec3 == gltf_position_accessor->type);
                                assert(cgltf_component_type_r_32f == gltf_position_accessor->component_type);

                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    cgltf_float gltf_position_float3[3];
                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_position_accessor, gltf_vertex_index, gltf_position_float3, 3U);
                                    assert(result_gltf_accessor_read_float);

                                    gltf_positions[gltf_vertex_index] = DirectX::XMFLOAT3(gltf_position_float3[0], gltf_position_float3[1], gltf_position_float3[2]);
                                }
                            }

                            assert(gltf_normals.empty());
                            gltf_normals.resize(gltf_vertex_count);

                            if (NULL != gltf_normal_accessor)
                            {
                                assert(gltf_normal_accessor->count == gltf_vertex_count);

                                assert(cgltf_type_vec3 == gltf_normal_accessor->type);
                                assert(cgltf_component_type_r_32f == gltf_normal_accessor->component_type);

                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    cgltf_float gltf_normal_float3[3];
                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_normal_accessor, gltf_vertex_index, gltf_normal_float3, 3U);
                                    assert(result_gltf_accessor_read_float);

                                    gltf_normals[gltf_vertex_index] = DirectX::XMFLOAT3(gltf_normal_float3[0], gltf_normal_float3[1], gltf_normal_float3[2]);
                                }
                            }
                            else
                            {
                                assert(!gltf_compute_normals);
                                gltf_compute_normals = true;

                                assert(0U == (raw_indices.size() % 3U));
                                uint32_t const face_count = raw_indices.size() / 3U;
                                bool const directx_compute_normals = DirectX::ComputeNormals(raw_indices.data(), face_count, gltf_positions.data(), gltf_vertex_count, DirectX::CNORM_DEFAULT, gltf_normals.data());
                                assert(directx_compute_normals);
                            }

                            assert(gltf_texcoords.empty());
                            gltf_texcoords.resize(gltf_vertex_count);

                            if (NULL != gltf_texcoord_accessor)
                            {
                                assert(gltf_texcoord_accessor->count == gltf_vertex_count);

                                assert(cgltf_type_vec2 == gltf_texcoord_accessor->type);

                                switch (gltf_texcoord_accessor->component_type)
                                {
                                case cgltf_component_type_r_8u:
                                {
                                    assert(cgltf_component_type_r_8u == gltf_texcoord_accessor->component_type);

                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                    {
                                        cgltf_uint gltf_texcoord_ubyte2[2];
                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_texcoord_accessor, gltf_vertex_index, gltf_texcoord_ubyte2, 2U);
                                        assert(result_gltf_accessor_read_uint);

                                        assert(gltf_texcoord_ubyte2[0] <= static_cast<cgltf_uint>(UINT8_MAX));
                                        assert(gltf_texcoord_ubyte2[1] <= static_cast<cgltf_uint>(UINT8_MAX));
                                        DirectX::PackedVector::XMUBYTEN2 packed_vector_ubyten2(static_cast<uint8_t>(gltf_texcoord_ubyte2[0]), static_cast<uint8_t>(gltf_texcoord_ubyte2[1]));

                                        DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN2(&packed_vector_ubyten2);

                                        DirectX::XMStoreFloat2(&gltf_texcoords[gltf_vertex_index], unpacked_vector);
                                    }
                                }
                                break;
                                case cgltf_component_type_r_16u:
                                {
                                    assert(cgltf_component_type_r_16u == gltf_texcoord_accessor->component_type);

                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                    {
                                        cgltf_uint gltf_texcoord_ushortn2[2];
                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_texcoord_accessor, gltf_vertex_index, gltf_texcoord_ushortn2, 2U);
                                        assert(result_gltf_accessor_read_uint);

                                        assert(gltf_texcoord_ushortn2[0] <= static_cast<cgltf_uint>(UINT16_MAX));
                                        assert(gltf_texcoord_ushortn2[1] <= static_cast<cgltf_uint>(UINT16_MAX));
                                        DirectX::PackedVector::XMUSHORTN2 packed_vector_ushortn2(static_cast<uint16_t>(gltf_texcoord_ushortn2[0]), static_cast<uint16_t>(gltf_texcoord_ushortn2[1]));

                                        DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN2(&packed_vector_ushortn2);

                                        DirectX::XMStoreFloat2(&gltf_texcoords[gltf_vertex_index], unpacked_vector);
                                    }
                                }
                                break;
                                case cgltf_component_type_r_32f:
                                {
                                    assert(cgltf_component_type_r_32f == gltf_texcoord_accessor->component_type);

                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                    {
                                        cgltf_float gltf_texcoord_float2[2];
                                        cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_texcoord_accessor, gltf_vertex_index, gltf_texcoord_float2, 2U);
                                        assert(result_gltf_accessor_read_float);

                                        gltf_texcoords[gltf_vertex_index] = DirectX::XMFLOAT2(gltf_texcoord_float2[0], gltf_texcoord_float2[1]);
                                    }
                                }
                                break;
                                default:
                                    assert(0);
                                }
                            }
                            else
                            {
                                assert(false);
                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    gltf_texcoords[gltf_vertex_index] = DirectX::XMFLOAT2(0.0F, 0.0F);
                                }
                            }

                            assert(gltf_tangents.empty());
                            gltf_tangents.resize(gltf_vertex_count);

                            if (NULL != gltf_tangent_accessor)
                            {
                                assert(gltf_tangent_accessor->count == gltf_vertex_count);

                                assert(cgltf_type_vec4 == gltf_tangent_accessor->type);
                                assert(cgltf_component_type_r_32f == gltf_tangent_accessor->component_type);

                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    cgltf_float gltf_tangent_float4[4];
                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_tangent_accessor, gltf_vertex_index, gltf_tangent_float4, 4U);
                                    assert(result_gltf_accessor_read_float);

                                    gltf_tangents[gltf_vertex_index] = DirectX::XMFLOAT4(gltf_tangent_float4[0], gltf_tangent_float4[1], gltf_tangent_float4[2], gltf_tangent_float4[3]);
                                }
                            }
                            else
                            {
                                assert(!gltf_compute_tangents);
                                gltf_compute_tangents = true;

                                assert(0U == (raw_indices.size() % 3U));
                                uint32_t const face_count = raw_indices.size() / 3U;
                                bool const directx_compute_tangent_frame = DirectX::ComputeTangentFrame(raw_indices.data(), face_count, gltf_positions.data(), gltf_normals.data(), gltf_texcoords.data(), gltf_vertex_count, gltf_tangents.data());
                                assert(directx_compute_tangent_frame);
                            }
                        }

                        // We should transform the absolute value instead of the offset
                        mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<DirectX::XMFLOAT3>> gltf_morph_targets_absolute_positions;
                        mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<DirectX::XMFLOAT3>> gltf_morph_targets_absolute_normals;
                        mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<DirectX::XMFLOAT2>> gltf_morph_targets_absolute_texcoords;
                        mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<DirectX::XMFLOAT4>> gltf_morph_targets_absolute_tangents;
                        if (gltf_primitive->targets_count > 0U)
                        {
                            if (found_vrmc_vrm)
                            {
                                auto found_vrmc_vrm_node_expressions_morph_target_binds = vrmc_vrm_nodes_expressions_morph_target_binds.find(gltf_mesh_instance_node_index);
                                if (vrmc_vrm_nodes_expressions_morph_target_binds.end() != found_vrmc_vrm_node_expressions_morph_target_binds)
                                {
                                    auto const &vrmc_vrm_node_expressions_morph_target_binds = found_vrmc_vrm_node_expressions_morph_target_binds->second;

                                    for (auto const &vrmc_vrm_node_expression_morph_target_name_and_binds : vrmc_vrm_node_expressions_morph_target_binds)
                                    {
                                        BRX_ASSET_IMPORT_MORPH_TARGET_NAME const vrmc_vrm_node_expression_morph_target_name = vrmc_vrm_node_expression_morph_target_name_and_binds.first;
                                        auto const &vrmc_vrm_node_expression_morph_target_binds = vrmc_vrm_node_expression_morph_target_name_and_binds.second;

                                        uint32_t const gltf_vertex_count = gltf_position_accessor->count;

                                        auto &gltf_morph_target_absolute_positions = gltf_morph_targets_absolute_positions[vrmc_vrm_node_expression_morph_target_name];
                                        auto &gltf_morph_target_absolute_normals = gltf_morph_targets_absolute_normals[vrmc_vrm_node_expression_morph_target_name];
                                        auto &gltf_morph_target_absolute_texcoords = gltf_morph_targets_absolute_texcoords[vrmc_vrm_node_expression_morph_target_name];
                                        auto &gltf_morph_target_absolute_tangents = gltf_morph_targets_absolute_tangents[vrmc_vrm_node_expression_morph_target_name];

                                        assert(gltf_morph_target_absolute_positions.empty());
                                        gltf_morph_target_absolute_positions.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_normals.empty());
                                        gltf_morph_target_absolute_normals.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_texcoords.empty());
                                        gltf_morph_target_absolute_texcoords.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_tangents.empty());
                                        gltf_morph_target_absolute_tangents.resize(gltf_vertex_count);

                                        for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            gltf_morph_target_absolute_positions[gltf_vertex_index] = gltf_positions[gltf_vertex_index];
                                            gltf_morph_target_absolute_normals[gltf_vertex_index] = gltf_normals[gltf_vertex_index];
                                            gltf_morph_target_absolute_texcoords[gltf_vertex_index] = gltf_texcoords[gltf_vertex_index];
                                            gltf_morph_target_absolute_tangents[gltf_vertex_index] = gltf_tangents[gltf_vertex_index];
                                        }

                                        for (auto const &vrmc_vrm_node_expression_morph_target_bind : vrmc_vrm_node_expression_morph_target_binds)
                                        {
                                            uint32_t const gltf_morph_target_index = vrmc_vrm_node_expression_morph_target_bind.first;

                                            cgltf_morph_target const *const gltf_morph_target = gltf_primitive->targets + gltf_morph_target_index;

                                            cgltf_accessor const *gltf_morph_target_position_accessor = NULL;
                                            cgltf_accessor const *gltf_morph_target_normal_accessor = NULL;
                                            cgltf_accessor const *gltf_morph_target_tangent_accessor = NULL;
                                            cgltf_accessor const *gltf_morph_target_texcoord_accessor = NULL;
                                            {
                                                // TODO: we select the smallest index but this may NOT be correct
                                                cgltf_int gltf_morph_target_position_index = -1;
                                                cgltf_int gltf_morph_target_normal_index = -1;
                                                cgltf_int gltf_morph_target_tangent_index = -1;
                                                cgltf_int gltf_morph_target_texcoord_index = -1;

                                                for (size_t gltf_morph_target_vertex_attribute_index = 0U; gltf_morph_target_vertex_attribute_index < gltf_morph_target->attributes_count; ++gltf_morph_target_vertex_attribute_index)
                                                {
                                                    cgltf_attribute const *const gltf_morph_target_vertex_attribute = gltf_morph_target->attributes + gltf_morph_target_vertex_attribute_index;

                                                    switch (gltf_morph_target_vertex_attribute->type)
                                                    {
                                                    case cgltf_attribute_type_position:
                                                    {
                                                        assert(cgltf_attribute_type_position == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_position_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_position_index)
                                                        {
                                                            gltf_morph_target_position_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_position_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_normal:
                                                    {
                                                        assert(cgltf_attribute_type_normal == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_normal_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_normal_index)
                                                        {
                                                            gltf_morph_target_normal_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_normal_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_tangent:
                                                    {
                                                        assert(cgltf_attribute_type_tangent == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_tangent_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_tangent_index)
                                                        {
                                                            gltf_morph_target_tangent_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_tangent_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_texcoord:
                                                    {
                                                        assert(cgltf_attribute_type_texcoord == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_texcoord_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_texcoord_index)
                                                        {
                                                            gltf_morph_target_texcoord_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_texcoord_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_joints:
                                                    {
                                                        assert(cgltf_attribute_type_joints == gltf_morph_target_vertex_attribute->type);

                                                        assert(false);
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_weights:
                                                    {
                                                        assert(cgltf_attribute_type_weights == gltf_morph_target_vertex_attribute->type);

                                                        assert(false);
                                                    }
                                                    break;
                                                    default:
                                                    {
                                                        // Do Nothing

                                                        assert(false);
                                                    }
                                                    }
                                                }
                                            }

                                            mcrt_vector<DirectX::XMFLOAT3> vrmc_vrm_morph_target_bind_absolute_positions(static_cast<size_t>(gltf_vertex_count));
                                            mcrt_vector<DirectX::XMFLOAT3> vrmc_vrm_morph_target_bind_absolute_normals(static_cast<size_t>(gltf_vertex_count));
                                            mcrt_vector<DirectX::XMFLOAT2> vrmc_vrm_morph_target_bind_absolute_texcoords(static_cast<size_t>(gltf_vertex_count));
                                            mcrt_vector<DirectX::XMFLOAT4> vrmc_vrm_morph_target_bind_absolute_tangents(static_cast<size_t>(gltf_vertex_count));

                                            if (NULL != gltf_morph_target_position_accessor)
                                            {
                                                assert(gltf_morph_target_position_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec3 == gltf_morph_target_position_accessor->type);
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_position_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_float gltf_morph_target_position_float3[3];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_position_accessor, gltf_vertex_index, gltf_morph_target_position_float3, 3U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT3 const gltf_morph_target_position(gltf_morph_target_position_float3[0], gltf_morph_target_position_float3[1], gltf_morph_target_position_float3[2]);

                                                    DirectX::XMStoreFloat3(&vrmc_vrm_morph_target_bind_absolute_positions[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_positions[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_morph_target_position)));
                                                }
                                            }
                                            else
                                            {
                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    vrmc_vrm_morph_target_bind_absolute_positions[gltf_vertex_index] = gltf_positions[gltf_vertex_index];
                                                }
                                            }

                                            if (NULL != gltf_morph_target_normal_accessor)
                                            {
                                                assert(gltf_morph_target_normal_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec3 == gltf_morph_target_normal_accessor->type);
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_normal_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_float gltf_morph_target_normal_float3[3];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_normal_accessor, gltf_vertex_index, gltf_morph_target_normal_float3, 3U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT3 const gltf_morph_target_normal(gltf_morph_target_normal_float3[0], gltf_morph_target_normal_float3[1], gltf_morph_target_normal_float3[2]);

                                                    DirectX::XMStoreFloat3(&vrmc_vrm_morph_target_bind_absolute_normals[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_normals[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_morph_target_normal)));
                                                }
                                            }
                                            else
                                            {
                                                if (gltf_compute_normals)
                                                {
                                                    assert(0U == (raw_indices.size() % 3U));
                                                    uint32_t const face_count = raw_indices.size() / 3U;
                                                    bool const directx_compute_normals = DirectX::ComputeNormals(raw_indices.data(), face_count, vrmc_vrm_morph_target_bind_absolute_positions.data(), gltf_vertex_count, DirectX::CNORM_DEFAULT, vrmc_vrm_morph_target_bind_absolute_normals.data());
                                                    assert(directx_compute_normals);
                                                }
                                                else
                                                {
                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        vrmc_vrm_morph_target_bind_absolute_normals[gltf_vertex_index] = gltf_normals[gltf_vertex_index];
                                                    }
                                                }
                                            }

                                            if (NULL != gltf_morph_target_texcoord_accessor)
                                            {
                                                assert(gltf_morph_target_texcoord_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec2 == gltf_morph_target_texcoord_accessor->type);

                                                switch (gltf_morph_target_texcoord_accessor->component_type)
                                                {
                                                case cgltf_component_type_r_8u:
                                                {
                                                    assert(cgltf_component_type_r_8u == gltf_morph_target_texcoord_accessor->component_type);

                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        cgltf_uint gltf_morph_target_texcoord_ubyte2[2];
                                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_ubyte2, 2U);
                                                        assert(result_gltf_accessor_read_uint);

                                                        assert(gltf_morph_target_texcoord_ubyte2[0] <= static_cast<cgltf_uint>(UINT8_MAX));
                                                        assert(gltf_morph_target_texcoord_ubyte2[1] <= static_cast<cgltf_uint>(UINT8_MAX));

                                                        DirectX::PackedVector::XMUBYTEN2 packed_vector_ubyten2(static_cast<uint8_t>(gltf_morph_target_texcoord_ubyte2[0]), static_cast<uint8_t>(gltf_morph_target_texcoord_ubyte2[1]));

                                                        DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN2(&packed_vector_ubyten2);

                                                        DirectX::XMStoreFloat2(&vrmc_vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), unpacked_vector));
                                                    }
                                                }
                                                break;
                                                case cgltf_component_type_r_16u:
                                                {
                                                    assert(cgltf_component_type_r_16u == gltf_morph_target_texcoord_accessor->component_type);

                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        cgltf_uint gltf_morph_target_texcoord_ushortn2[2];
                                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_ushortn2, 2U);
                                                        assert(result_gltf_accessor_read_uint);

                                                        assert(gltf_morph_target_texcoord_ushortn2[0] <= static_cast<cgltf_uint>(UINT16_MAX));
                                                        assert(gltf_morph_target_texcoord_ushortn2[1] <= static_cast<cgltf_uint>(UINT16_MAX));

                                                        DirectX::PackedVector::XMUSHORTN2 packed_vector_ushortn2(static_cast<uint16_t>(gltf_morph_target_texcoord_ushortn2[0]), static_cast<uint16_t>(gltf_morph_target_texcoord_ushortn2[1]));

                                                        DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN2(&packed_vector_ushortn2);

                                                        DirectX::XMStoreFloat2(&vrmc_vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), unpacked_vector));
                                                    }
                                                }
                                                break;
                                                case cgltf_component_type_r_32f:
                                                {
                                                    assert(cgltf_component_type_r_32f == gltf_morph_target_texcoord_accessor->component_type);

                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        cgltf_float gltf_morph_target_texcoord_float2[2];
                                                        cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_float2, 2U);
                                                        assert(result_gltf_accessor_read_float);

                                                        DirectX::XMFLOAT2 const gltf_morph_target_texcoord(gltf_morph_target_texcoord_float2[0], gltf_morph_target_texcoord_float2[1]);

                                                        DirectX::XMStoreFloat2(&vrmc_vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), DirectX::XMLoadFloat2(&gltf_morph_target_texcoord)));
                                                    }
                                                }
                                                break;
                                                default:
                                                    assert(0);
                                                }
                                            }
                                            else
                                            {
                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    vrmc_vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index] = gltf_texcoords[gltf_vertex_index];
                                                }
                                            }

                                            if (NULL != gltf_morph_target_tangent_accessor)
                                            {
                                                assert(gltf_morph_target_tangent_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec4 == gltf_morph_target_tangent_accessor->type);
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_tangent_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    float gltf_morph_target_tangent_float4[4];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_tangent_accessor, gltf_vertex_index, gltf_morph_target_tangent_float4, 4U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT4 const gltf_morph_target_tangent(gltf_morph_target_tangent_float4[0], gltf_morph_target_tangent_float4[1], gltf_morph_target_tangent_float4[2], gltf_morph_target_tangent_float4[3]);

                                                    DirectX::XMStoreFloat4(&vrmc_vrm_morph_target_bind_absolute_tangents[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&gltf_tangents[gltf_vertex_index]), DirectX::XMLoadFloat4(&gltf_morph_target_tangent)));
                                                }
                                            }
                                            else
                                            {
                                                if (gltf_compute_tangents)
                                                {
                                                    assert(0U == (raw_indices.size() % 3U));
                                                    uint32_t const face_count = raw_indices.size() / 3U;
                                                    bool const directx_compute_tangent_frame = DirectX::ComputeTangentFrame(raw_indices.data(), face_count, vrmc_vrm_morph_target_bind_absolute_positions.data(), vrmc_vrm_morph_target_bind_absolute_normals.data(), vrmc_vrm_morph_target_bind_absolute_texcoords.data(), gltf_vertex_count, vrmc_vrm_morph_target_bind_absolute_tangents.data());
                                                    assert(directx_compute_tangent_frame);
                                                }
                                                else
                                                {
                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        vrmc_vrm_morph_target_bind_absolute_tangents[gltf_vertex_index] = gltf_tangents[gltf_vertex_index];
                                                    }
                                                }
                                            }

                                            float const vrmc_vrm_morph_target_bind_weight = vrmc_vrm_node_expression_morph_target_bind.second;

                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                DirectX::XMStoreFloat3(&gltf_morph_target_absolute_positions[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_positions[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&vrmc_vrm_morph_target_bind_absolute_positions[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_positions[gltf_vertex_index])), vrmc_vrm_morph_target_bind_weight)));
                                                DirectX::XMStoreFloat3(&gltf_morph_target_absolute_normals[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_normals[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&vrmc_vrm_morph_target_bind_absolute_normals[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_normals[gltf_vertex_index])), vrmc_vrm_morph_target_bind_weight)));
                                                DirectX::XMStoreFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&vrmc_vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index]), DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index])), vrmc_vrm_morph_target_bind_weight)));
                                                DirectX::XMStoreFloat4(&gltf_morph_target_absolute_tangents[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&gltf_morph_target_absolute_tangents[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat4(&vrmc_vrm_morph_target_bind_absolute_tangents[gltf_vertex_index]), DirectX::XMLoadFloat4(&gltf_tangents[gltf_vertex_index])), vrmc_vrm_morph_target_bind_weight)));
                                            }
                                        }
                                    }
                                }
                            }
                            else if (found_vrm)
                            {
                                uint32_t const gltf_mesh_index = cgltf_mesh_index(in_gltf_data, gltf_mesh);

                                auto found_vrm_mesh_blend_shapes_morph_target_binds = vrm_meshes_blend_shapes_morph_target_binds.find(gltf_mesh_index);
                                if (vrm_meshes_blend_shapes_morph_target_binds.end() != found_vrm_mesh_blend_shapes_morph_target_binds)
                                {
                                    auto const &vrm_mesh_blend_shapes_morph_target_binds = found_vrm_mesh_blend_shapes_morph_target_binds->second;

                                    for (auto const &vrm_mesh_blend_shape_morph_target_name_and_binds : vrm_mesh_blend_shapes_morph_target_binds)
                                    {
                                        BRX_ASSET_IMPORT_MORPH_TARGET_NAME const vrm_mesh_blend_shape_morph_target_name = vrm_mesh_blend_shape_morph_target_name_and_binds.first;
                                        auto const &vrm_mesh_blend_shape_morph_target_binds = vrm_mesh_blend_shape_morph_target_name_and_binds.second;

                                        uint32_t const gltf_vertex_count = gltf_position_accessor->count;

                                        auto &gltf_morph_target_absolute_positions = gltf_morph_targets_absolute_positions[vrm_mesh_blend_shape_morph_target_name];
                                        auto &gltf_morph_target_absolute_normals = gltf_morph_targets_absolute_normals[vrm_mesh_blend_shape_morph_target_name];
                                        auto &gltf_morph_target_absolute_texcoords = gltf_morph_targets_absolute_texcoords[vrm_mesh_blend_shape_morph_target_name];
                                        auto &gltf_morph_target_absolute_tangents = gltf_morph_targets_absolute_tangents[vrm_mesh_blend_shape_morph_target_name];

                                        assert(gltf_morph_target_absolute_positions.empty());
                                        gltf_morph_target_absolute_positions.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_normals.empty());
                                        gltf_morph_target_absolute_normals.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_texcoords.empty());
                                        gltf_morph_target_absolute_texcoords.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_tangents.empty());
                                        gltf_morph_target_absolute_tangents.resize(gltf_vertex_count);

                                        for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            gltf_morph_target_absolute_positions[gltf_vertex_index] = gltf_positions[gltf_vertex_index];
                                            gltf_morph_target_absolute_normals[gltf_vertex_index] = gltf_normals[gltf_vertex_index];
                                            gltf_morph_target_absolute_texcoords[gltf_vertex_index] = gltf_texcoords[gltf_vertex_index];
                                            gltf_morph_target_absolute_tangents[gltf_vertex_index] = gltf_tangents[gltf_vertex_index];
                                        }

                                        for (auto const &vrm_mesh_blend_shape_morph_target_bind : vrm_mesh_blend_shape_morph_target_binds)
                                        {
                                            uint32_t const gltf_morph_target_index = vrm_mesh_blend_shape_morph_target_bind.first;

                                            cgltf_morph_target const *const gltf_morph_target = gltf_primitive->targets + gltf_morph_target_index;

                                            cgltf_accessor const *gltf_morph_target_position_accessor = NULL;
                                            cgltf_accessor const *gltf_morph_target_normal_accessor = NULL;
                                            cgltf_accessor const *gltf_morph_target_tangent_accessor = NULL;
                                            cgltf_accessor const *gltf_morph_target_texcoord_accessor = NULL;
                                            {
                                                // TODO: we select the smallest index but this may NOT be correct
                                                cgltf_int gltf_morph_target_position_index = -1;
                                                cgltf_int gltf_morph_target_normal_index = -1;
                                                cgltf_int gltf_morph_target_tangent_index = -1;
                                                cgltf_int gltf_morph_target_texcoord_index = -1;

                                                for (size_t gltf_morph_target_vertex_attribute_index = 0U; gltf_morph_target_vertex_attribute_index < gltf_morph_target->attributes_count; ++gltf_morph_target_vertex_attribute_index)
                                                {
                                                    cgltf_attribute const *const gltf_morph_target_vertex_attribute = gltf_morph_target->attributes + gltf_morph_target_vertex_attribute_index;

                                                    switch (gltf_morph_target_vertex_attribute->type)
                                                    {
                                                    case cgltf_attribute_type_position:
                                                    {
                                                        assert(cgltf_attribute_type_position == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_position_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_position_index)
                                                        {
                                                            gltf_morph_target_position_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_position_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_normal:
                                                    {
                                                        assert(cgltf_attribute_type_normal == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_normal_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_normal_index)
                                                        {
                                                            gltf_morph_target_normal_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_normal_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_tangent:
                                                    {
                                                        assert(cgltf_attribute_type_tangent == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_tangent_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_tangent_index)
                                                        {
                                                            gltf_morph_target_tangent_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_tangent_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_texcoord:
                                                    {
                                                        assert(cgltf_attribute_type_texcoord == gltf_morph_target_vertex_attribute->type);

                                                        if (NULL == gltf_morph_target_texcoord_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_texcoord_index)
                                                        {
                                                            gltf_morph_target_texcoord_accessor = gltf_morph_target_vertex_attribute->data;
                                                            gltf_morph_target_texcoord_index = gltf_morph_target_vertex_attribute->index;
                                                        }
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_joints:
                                                    {
                                                        assert(cgltf_attribute_type_joints == gltf_morph_target_vertex_attribute->type);

                                                        assert(false);
                                                    }
                                                    break;
                                                    case cgltf_attribute_type_weights:
                                                    {
                                                        assert(cgltf_attribute_type_weights == gltf_morph_target_vertex_attribute->type);

                                                        assert(false);
                                                    }
                                                    break;
                                                    default:
                                                    {
                                                        // Do Nothing

                                                        assert(false);
                                                    }
                                                    }
                                                }
                                            }

                                            mcrt_vector<DirectX::XMFLOAT3> vrm_morph_target_bind_absolute_positions(static_cast<size_t>(gltf_vertex_count));
                                            mcrt_vector<DirectX::XMFLOAT3> vrm_morph_target_bind_absolute_normals(static_cast<size_t>(gltf_vertex_count));
                                            mcrt_vector<DirectX::XMFLOAT2> vrm_morph_target_bind_absolute_texcoords(static_cast<size_t>(gltf_vertex_count));
                                            mcrt_vector<DirectX::XMFLOAT4> vrm_morph_target_bind_absolute_tangents(static_cast<size_t>(gltf_vertex_count));

                                            if (NULL != gltf_morph_target_position_accessor)
                                            {
                                                assert(gltf_morph_target_position_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec3 == gltf_morph_target_position_accessor->type);
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_position_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_float gltf_morph_target_position_float3[3];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_position_accessor, gltf_vertex_index, gltf_morph_target_position_float3, 3U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT3 const gltf_morph_target_position(gltf_morph_target_position_float3[0], gltf_morph_target_position_float3[1], gltf_morph_target_position_float3[2]);

                                                    DirectX::XMStoreFloat3(&vrm_morph_target_bind_absolute_positions[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_positions[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_morph_target_position)));
                                                }
                                            }
                                            else
                                            {
                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    vrm_morph_target_bind_absolute_positions[gltf_vertex_index] = gltf_positions[gltf_vertex_index];
                                                }
                                            }

                                            if (NULL != gltf_morph_target_normal_accessor)
                                            {
                                                assert(gltf_morph_target_normal_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec3 == gltf_morph_target_normal_accessor->type);
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_normal_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_float gltf_morph_target_normal_float3[3];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_normal_accessor, gltf_vertex_index, gltf_morph_target_normal_float3, 3U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT3 const gltf_morph_target_normal(gltf_morph_target_normal_float3[0], gltf_morph_target_normal_float3[1], gltf_morph_target_normal_float3[2]);

                                                    DirectX::XMStoreFloat3(&vrm_morph_target_bind_absolute_normals[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_normals[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_morph_target_normal)));
                                                }
                                            }
                                            else
                                            {
                                                if (gltf_compute_normals)
                                                {
                                                    assert(0U == (raw_indices.size() % 3U));
                                                    uint32_t const face_count = raw_indices.size() / 3U;
                                                    bool const directx_compute_normals = DirectX::ComputeNormals(raw_indices.data(), face_count, vrm_morph_target_bind_absolute_positions.data(), gltf_vertex_count, DirectX::CNORM_DEFAULT, vrm_morph_target_bind_absolute_normals.data());
                                                    assert(directx_compute_normals);
                                                }
                                                else
                                                {
                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        vrm_morph_target_bind_absolute_normals[gltf_vertex_index] = gltf_normals[gltf_vertex_index];
                                                    }
                                                }
                                            }

                                            if (NULL != gltf_morph_target_texcoord_accessor)
                                            {
                                                assert(gltf_morph_target_texcoord_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec2 == gltf_morph_target_texcoord_accessor->type);

                                                switch (gltf_morph_target_texcoord_accessor->component_type)
                                                {
                                                case cgltf_component_type_r_8u:
                                                {
                                                    assert(cgltf_component_type_r_8u == gltf_morph_target_texcoord_accessor->component_type);

                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        cgltf_uint gltf_morph_target_texcoord_ubyte2[2];
                                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_ubyte2, 2U);
                                                        assert(result_gltf_accessor_read_uint);

                                                        assert(gltf_morph_target_texcoord_ubyte2[0] <= static_cast<cgltf_uint>(UINT8_MAX));
                                                        assert(gltf_morph_target_texcoord_ubyte2[1] <= static_cast<cgltf_uint>(UINT8_MAX));

                                                        DirectX::PackedVector::XMUBYTEN2 packed_vector_ubyten2(static_cast<uint8_t>(gltf_morph_target_texcoord_ubyte2[0]), static_cast<uint8_t>(gltf_morph_target_texcoord_ubyte2[1]));

                                                        DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN2(&packed_vector_ubyten2);

                                                        DirectX::XMStoreFloat2(&vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), unpacked_vector));
                                                    }
                                                }
                                                break;
                                                case cgltf_component_type_r_16u:
                                                {
                                                    assert(cgltf_component_type_r_16u == gltf_morph_target_texcoord_accessor->component_type);

                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        cgltf_uint gltf_morph_target_texcoord_ushortn2[2];
                                                        cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_ushortn2, 2U);
                                                        assert(result_gltf_accessor_read_uint);

                                                        assert(gltf_morph_target_texcoord_ushortn2[0] <= static_cast<cgltf_uint>(UINT16_MAX));
                                                        assert(gltf_morph_target_texcoord_ushortn2[1] <= static_cast<cgltf_uint>(UINT16_MAX));

                                                        DirectX::PackedVector::XMUSHORTN2 packed_vector_ushortn2(static_cast<uint16_t>(gltf_morph_target_texcoord_ushortn2[0]), static_cast<uint16_t>(gltf_morph_target_texcoord_ushortn2[1]));

                                                        DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN2(&packed_vector_ushortn2);

                                                        DirectX::XMStoreFloat2(&vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), unpacked_vector));
                                                    }
                                                }
                                                break;
                                                case cgltf_component_type_r_32f:
                                                {
                                                    assert(cgltf_component_type_r_32f == gltf_morph_target_texcoord_accessor->component_type);

                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        cgltf_float gltf_morph_target_texcoord_float2[2];
                                                        cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_float2, 2U);
                                                        assert(result_gltf_accessor_read_float);

                                                        DirectX::XMFLOAT2 const gltf_morph_target_texcoord(gltf_morph_target_texcoord_float2[0], gltf_morph_target_texcoord_float2[1]);

                                                        DirectX::XMStoreFloat2(&vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), DirectX::XMLoadFloat2(&gltf_morph_target_texcoord)));
                                                    }
                                                }
                                                break;
                                                default:
                                                    assert(0);
                                                }
                                            }
                                            else
                                            {
                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index] = gltf_texcoords[gltf_vertex_index];
                                                }
                                            }

                                            if (NULL != gltf_morph_target_tangent_accessor)
                                            {
                                                assert(gltf_morph_target_tangent_accessor->count == gltf_vertex_count);

                                                assert(cgltf_type_vec4 == gltf_morph_target_tangent_accessor->type);
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_tangent_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    float gltf_morph_target_tangent_float4[4];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_tangent_accessor, gltf_vertex_index, gltf_morph_target_tangent_float4, 4U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT4 const gltf_morph_target_tangent(gltf_morph_target_tangent_float4[0], gltf_morph_target_tangent_float4[1], gltf_morph_target_tangent_float4[2], gltf_morph_target_tangent_float4[3]);

                                                    DirectX::XMStoreFloat4(&vrm_morph_target_bind_absolute_tangents[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&gltf_tangents[gltf_vertex_index]), DirectX::XMLoadFloat4(&gltf_morph_target_tangent)));
                                                }
                                            }
                                            else
                                            {
                                                if (gltf_compute_tangents)
                                                {
                                                    assert(0U == (raw_indices.size() % 3U));
                                                    uint32_t const face_count = raw_indices.size() / 3U;
                                                    bool const directx_compute_tangent_frame = DirectX::ComputeTangentFrame(raw_indices.data(), face_count, vrm_morph_target_bind_absolute_positions.data(), vrm_morph_target_bind_absolute_normals.data(), vrm_morph_target_bind_absolute_texcoords.data(), gltf_vertex_count, vrm_morph_target_bind_absolute_tangents.data());
                                                    assert(directx_compute_tangent_frame);
                                                }
                                                else
                                                {
                                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                    {
                                                        vrm_morph_target_bind_absolute_tangents[gltf_vertex_index] = gltf_tangents[gltf_vertex_index];
                                                    }
                                                }
                                            }

                                            float const vrm_morph_target_bind_weight = vrm_mesh_blend_shape_morph_target_bind.second;

                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                DirectX::XMStoreFloat3(&gltf_morph_target_absolute_positions[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_positions[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&vrm_morph_target_bind_absolute_positions[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_positions[gltf_vertex_index])), vrm_morph_target_bind_weight)));
                                                DirectX::XMStoreFloat3(&gltf_morph_target_absolute_normals[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_normals[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&vrm_morph_target_bind_absolute_normals[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_normals[gltf_vertex_index])), vrm_morph_target_bind_weight)));
                                                DirectX::XMStoreFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&vrm_morph_target_bind_absolute_texcoords[gltf_vertex_index]), DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index])), vrm_morph_target_bind_weight)));
                                                DirectX::XMStoreFloat4(&gltf_morph_target_absolute_tangents[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&gltf_morph_target_absolute_tangents[gltf_vertex_index]), DirectX::XMVectorScale(DirectX::XMVectorSubtract(DirectX::XMLoadFloat4(&vrm_morph_target_bind_absolute_tangents[gltf_vertex_index]), DirectX::XMLoadFloat4(&gltf_tangents[gltf_vertex_index])), vrm_morph_target_bind_weight)));
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> gltf_mesh_morph_target_names(static_cast<size_t>(gltf_mesh->target_names_count));
                                {
                                    mcrt_unordered_map<mcrt_string, uint32_t> mmd_morph_target_indices;

                                    for (uint32_t current_morph_target_index = 0U; current_morph_target_index < gltf_mesh->target_names_count; ++current_morph_target_index)
                                    {
                                        gltf_mesh_morph_target_names[current_morph_target_index] = BRX_ASSET_IMPORT_MORPH_TARGET_NAME_INVALID;

                                        char *const current_morph_target_name = gltf_mesh->target_names[current_morph_target_index];
                                        cgltf_decode_string(current_morph_target_name);

                                        auto const found_mmd_morph_target_index = mmd_morph_target_indices.find(current_morph_target_name);

                                        if (mmd_morph_target_indices.end() == found_mmd_morph_target_index)
                                        {
                                            mmd_morph_target_indices.emplace_hint(found_mmd_morph_target_index, current_morph_target_name, current_morph_target_index);
                                        }
                                        else
                                        {
                                            assert(false);
                                        }
                                    }

                                    mcrt_vector<mcrt_vector<mcrt_string>> mmd_morph_target_name_strings(static_cast<size_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT));
                                    internal_fill_mmd_morph_target_name_strings(mmd_morph_target_name_strings);

                                    for (uint32_t mmd_morph_target_name = 0U; mmd_morph_target_name < BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT; ++mmd_morph_target_name)
                                    {
                                        for (mcrt_string const &mmd_morph_target_name_string : mmd_morph_target_name_strings[mmd_morph_target_name])
                                        {
                                            auto const found_mmd_name_and_morph_target = mmd_morph_target_indices.find(mmd_morph_target_name_string);
                                            if (mmd_morph_target_indices.end() != found_mmd_name_and_morph_target)
                                            {
                                                uint32_t const current_morph_target_index = found_mmd_name_and_morph_target->second;
                                                assert(current_morph_target_index < gltf_mesh_morph_target_names.size());
                                                assert(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_INVALID == gltf_mesh_morph_target_names[current_morph_target_index]);
                                                gltf_mesh_morph_target_names[current_morph_target_index] = static_cast<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>(mmd_morph_target_name);
                                                break;
                                            }
                                        }
                                    }
                                }

                                assert((0U == gltf_mesh->target_names_count) || (gltf_mesh->target_names_count == gltf_primitive->targets_count));
                                uint32_t const gltf_morph_target_count = gltf_primitive->targets_count;

                                for (uint32_t gltf_morph_target_index = 0U; gltf_morph_target_index < gltf_morph_target_count; ++gltf_morph_target_index)
                                {
                                    cgltf_morph_target const *const gltf_morph_target = gltf_primitive->targets + gltf_morph_target_index;

                                    cgltf_accessor const *gltf_morph_target_position_accessor = NULL;
                                    cgltf_accessor const *gltf_morph_target_normal_accessor = NULL;
                                    cgltf_accessor const *gltf_morph_target_tangent_accessor = NULL;
                                    cgltf_accessor const *gltf_morph_target_texcoord_accessor = NULL;
                                    {
                                        // TODO: we select the smallest index but this may NOT be correct
                                        cgltf_int gltf_morph_target_position_index = -1;
                                        cgltf_int gltf_morph_target_normal_index = -1;
                                        cgltf_int gltf_morph_target_tangent_index = -1;
                                        cgltf_int gltf_morph_target_texcoord_index = -1;

                                        for (size_t gltf_morph_target_vertex_attribute_index = 0U; gltf_morph_target_vertex_attribute_index < gltf_morph_target->attributes_count; ++gltf_morph_target_vertex_attribute_index)
                                        {
                                            cgltf_attribute const *const gltf_morph_target_vertex_attribute = gltf_morph_target->attributes + gltf_morph_target_vertex_attribute_index;

                                            switch (gltf_morph_target_vertex_attribute->type)
                                            {
                                            case cgltf_attribute_type_position:
                                            {
                                                assert(cgltf_attribute_type_position == gltf_morph_target_vertex_attribute->type);

                                                if (NULL == gltf_morph_target_position_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_position_index)
                                                {
                                                    gltf_morph_target_position_accessor = gltf_morph_target_vertex_attribute->data;
                                                    gltf_morph_target_position_index = gltf_morph_target_vertex_attribute->index;
                                                }
                                            }
                                            break;
                                            case cgltf_attribute_type_normal:
                                            {
                                                assert(cgltf_attribute_type_normal == gltf_morph_target_vertex_attribute->type);

                                                if (NULL == gltf_morph_target_normal_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_normal_index)
                                                {
                                                    gltf_morph_target_normal_accessor = gltf_morph_target_vertex_attribute->data;
                                                    gltf_morph_target_normal_index = gltf_morph_target_vertex_attribute->index;
                                                }
                                            }
                                            break;
                                            case cgltf_attribute_type_tangent:
                                            {
                                                assert(cgltf_attribute_type_tangent == gltf_morph_target_vertex_attribute->type);

                                                if (NULL == gltf_morph_target_tangent_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_tangent_index)
                                                {
                                                    gltf_morph_target_tangent_accessor = gltf_morph_target_vertex_attribute->data;
                                                    gltf_morph_target_tangent_index = gltf_morph_target_vertex_attribute->index;
                                                }
                                            }
                                            break;
                                            case cgltf_attribute_type_texcoord:
                                            {
                                                assert(cgltf_attribute_type_texcoord == gltf_morph_target_vertex_attribute->type);

                                                if (NULL == gltf_morph_target_texcoord_accessor || gltf_morph_target_vertex_attribute->index < gltf_morph_target_texcoord_index)
                                                {
                                                    gltf_morph_target_texcoord_accessor = gltf_morph_target_vertex_attribute->data;
                                                    gltf_morph_target_texcoord_index = gltf_morph_target_vertex_attribute->index;
                                                }
                                            }
                                            break;
                                            case cgltf_attribute_type_joints:
                                            {
                                                assert(cgltf_attribute_type_joints == gltf_morph_target_vertex_attribute->type);

                                                assert(false);
                                            }
                                            break;
                                            case cgltf_attribute_type_weights:
                                            {
                                                assert(cgltf_attribute_type_weights == gltf_morph_target_vertex_attribute->type);

                                                assert(false);
                                            }
                                            break;
                                            default:
                                            {
                                                // Do Nothing

                                                assert(false);
                                            }
                                            }
                                        }
                                    }

                                    BRX_ASSET_IMPORT_MORPH_TARGET_NAME const gltf_mesh_morph_target_name = gltf_mesh_morph_target_names[gltf_morph_target_index];
                                    if (BRX_ASSET_IMPORT_MORPH_TARGET_NAME_INVALID != gltf_mesh_morph_target_name)
                                    {
                                        uint32_t const gltf_vertex_count = gltf_position_accessor->count;

                                        auto &gltf_morph_target_absolute_positions = gltf_morph_targets_absolute_positions[gltf_mesh_morph_target_name];
                                        auto &gltf_morph_target_absolute_normals = gltf_morph_targets_absolute_normals[gltf_mesh_morph_target_name];
                                        auto &gltf_morph_target_absolute_texcoords = gltf_morph_targets_absolute_texcoords[gltf_mesh_morph_target_name];
                                        auto &gltf_morph_target_absolute_tangents = gltf_morph_targets_absolute_tangents[gltf_mesh_morph_target_name];

                                        assert(gltf_morph_target_absolute_positions.empty());
                                        gltf_morph_target_absolute_positions.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_normals.empty());
                                        gltf_morph_target_absolute_normals.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_texcoords.empty());
                                        gltf_morph_target_absolute_texcoords.resize(gltf_vertex_count);

                                        assert(gltf_morph_target_absolute_tangents.empty());
                                        gltf_morph_target_absolute_tangents.resize(gltf_vertex_count);

                                        if (NULL != gltf_morph_target_position_accessor)
                                        {
                                            assert(gltf_morph_target_position_accessor->count == gltf_vertex_count);

                                            assert(cgltf_type_vec3 == gltf_morph_target_position_accessor->type);
                                            assert(cgltf_component_type_r_32f == gltf_morph_target_position_accessor->component_type);

                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                cgltf_float gltf_morph_target_position_float3[3];
                                                cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_position_accessor, gltf_vertex_index, gltf_morph_target_position_float3, 3U);
                                                assert(result_gltf_accessor_read_float);

                                                DirectX::XMFLOAT3 const gltf_morph_target_position(gltf_morph_target_position_float3[0], gltf_morph_target_position_float3[1], gltf_morph_target_position_float3[2]);

                                                DirectX::XMStoreFloat3(&gltf_morph_target_absolute_positions[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_positions[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_morph_target_position)));
                                            }
                                        }
                                        else
                                        {
                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                gltf_morph_target_absolute_positions[gltf_vertex_index] = gltf_positions[gltf_vertex_index];
                                            }
                                        }

                                        if (NULL != gltf_morph_target_normal_accessor)
                                        {
                                            assert(gltf_morph_target_normal_accessor->count == gltf_vertex_count);

                                            assert(cgltf_type_vec3 == gltf_morph_target_normal_accessor->type);
                                            assert(cgltf_component_type_r_32f == gltf_morph_target_normal_accessor->component_type);

                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                cgltf_float gltf_morph_target_normal_float3[3];
                                                cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_normal_accessor, gltf_vertex_index, gltf_morph_target_normal_float3, 3U);
                                                assert(result_gltf_accessor_read_float);

                                                DirectX::XMFLOAT3 const gltf_morph_target_normal(gltf_morph_target_normal_float3[0], gltf_morph_target_normal_float3[1], gltf_morph_target_normal_float3[2]);

                                                DirectX::XMStoreFloat3(&gltf_morph_target_absolute_normals[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&gltf_normals[gltf_vertex_index]), DirectX::XMLoadFloat3(&gltf_morph_target_normal)));
                                            }
                                        }
                                        else
                                        {
                                            if (gltf_compute_normals)
                                            {
                                                assert(0U == (raw_indices.size() % 3U));
                                                uint32_t const face_count = raw_indices.size() / 3U;
                                                bool const directx_compute_normals = DirectX::ComputeNormals(raw_indices.data(), face_count, gltf_morph_target_absolute_positions.data(), gltf_vertex_count, DirectX::CNORM_DEFAULT, gltf_morph_target_absolute_normals.data());
                                                assert(directx_compute_normals);
                                            }
                                            else
                                            {
                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    gltf_morph_target_absolute_normals[gltf_vertex_index] = gltf_normals[gltf_vertex_index];
                                                }
                                            }
                                        }

                                        if (NULL != gltf_morph_target_texcoord_accessor)
                                        {
                                            assert(gltf_morph_target_texcoord_accessor->count == gltf_vertex_count);

                                            assert(cgltf_type_vec2 == gltf_morph_target_texcoord_accessor->type);

                                            switch (gltf_morph_target_texcoord_accessor->component_type)
                                            {
                                            case cgltf_component_type_r_8u:
                                            {
                                                assert(cgltf_component_type_r_8u == gltf_morph_target_texcoord_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_uint gltf_morph_target_texcoord_ubyte2[2];
                                                    cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_ubyte2, 2U);
                                                    assert(result_gltf_accessor_read_uint);

                                                    assert(gltf_morph_target_texcoord_ubyte2[0] <= static_cast<cgltf_uint>(UINT8_MAX));
                                                    assert(gltf_morph_target_texcoord_ubyte2[1] <= static_cast<cgltf_uint>(UINT8_MAX));

                                                    DirectX::PackedVector::XMUBYTEN2 packed_vector_ubyten2(static_cast<uint8_t>(gltf_morph_target_texcoord_ubyte2[0]), static_cast<uint8_t>(gltf_morph_target_texcoord_ubyte2[1]));

                                                    DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN2(&packed_vector_ubyten2);

                                                    DirectX::XMStoreFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), unpacked_vector));
                                                }
                                            }
                                            break;
                                            case cgltf_component_type_r_16u:
                                            {
                                                assert(cgltf_component_type_r_16u == gltf_morph_target_texcoord_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_uint gltf_morph_target_texcoord_ushortn2[2];
                                                    cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_ushortn2, 2U);
                                                    assert(result_gltf_accessor_read_uint);

                                                    assert(gltf_morph_target_texcoord_ushortn2[0] <= static_cast<cgltf_uint>(UINT16_MAX));
                                                    assert(gltf_morph_target_texcoord_ushortn2[1] <= static_cast<cgltf_uint>(UINT16_MAX));

                                                    DirectX::PackedVector::XMUSHORTN2 packed_vector_ushortn2(static_cast<uint16_t>(gltf_morph_target_texcoord_ushortn2[0]), static_cast<uint16_t>(gltf_morph_target_texcoord_ushortn2[1]));

                                                    DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN2(&packed_vector_ushortn2);

                                                    DirectX::XMStoreFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), unpacked_vector));
                                                }
                                            }
                                            break;
                                            case cgltf_component_type_r_32f:
                                            {
                                                assert(cgltf_component_type_r_32f == gltf_morph_target_texcoord_accessor->component_type);

                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    cgltf_float gltf_morph_target_texcoord_float2[2];
                                                    cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_texcoord_accessor, gltf_vertex_index, gltf_morph_target_texcoord_float2, 2U);
                                                    assert(result_gltf_accessor_read_float);

                                                    DirectX::XMFLOAT2 const gltf_morph_target_texcoord(gltf_morph_target_texcoord_float2[0], gltf_morph_target_texcoord_float2[1]);

                                                    DirectX::XMStoreFloat2(&gltf_morph_target_absolute_texcoords[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat2(&gltf_texcoords[gltf_vertex_index]), DirectX::XMLoadFloat2(&gltf_morph_target_texcoord)));
                                                }
                                            }
                                            break;
                                            default:
                                                assert(0);
                                            }
                                        }
                                        else
                                        {
                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                gltf_morph_target_absolute_texcoords[gltf_vertex_index] = gltf_texcoords[gltf_vertex_index];
                                            }
                                        }

                                        if (NULL != gltf_morph_target_tangent_accessor)
                                        {
                                            assert(gltf_morph_target_tangent_accessor->count == gltf_vertex_count);

                                            assert(cgltf_type_vec4 == gltf_morph_target_tangent_accessor->type);
                                            assert(cgltf_component_type_r_32f == gltf_morph_target_tangent_accessor->component_type);

                                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                            {
                                                float gltf_morph_target_tangent_float4[4];
                                                cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_morph_target_tangent_accessor, gltf_vertex_index, gltf_morph_target_tangent_float4, 4U);
                                                assert(result_gltf_accessor_read_float);

                                                DirectX::XMFLOAT4 const gltf_morph_target_tangent(gltf_morph_target_tangent_float4[0], gltf_morph_target_tangent_float4[1], gltf_morph_target_tangent_float4[2], gltf_morph_target_tangent_float4[3]);

                                                DirectX::XMStoreFloat4(&gltf_morph_target_absolute_tangents[gltf_vertex_index], DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&gltf_tangents[gltf_vertex_index]), DirectX::XMLoadFloat4(&gltf_morph_target_tangent)));
                                            }
                                        }
                                        else
                                        {
                                            if (gltf_compute_tangents)
                                            {
                                                assert(0U == (raw_indices.size() % 3U));
                                                uint32_t const face_count = raw_indices.size() / 3U;
                                                bool const directx_compute_tangent_frame = DirectX::ComputeTangentFrame(raw_indices.data(), face_count, gltf_morph_target_absolute_positions.data(), gltf_morph_target_absolute_normals.data(), gltf_morph_target_absolute_texcoords.data(), gltf_vertex_count, gltf_morph_target_absolute_tangents.data());
                                                assert(directx_compute_tangent_frame);
                                            }
                                            else
                                            {
                                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                                {
                                                    gltf_morph_target_absolute_tangents[gltf_vertex_index] = gltf_tangents[gltf_vertex_index];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            assert(gltf_morph_targets_absolute_positions.empty());
                            assert(gltf_morph_targets_absolute_normals.empty());
                            assert(gltf_morph_targets_absolute_texcoords.empty());
                            assert(gltf_morph_targets_absolute_tangents.empty());
                        }

                        {
                            assert(raw_vertices.empty());
                            raw_vertices.resize(gltf_vertex_count);

                            if (NULL != gltf_skin)
                            {
#ifndef NDEBUG
                                {
                                    // [glTF Validator: NODE_SKINNED_MESH_NON_ROOT](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L444)
                                    // [glTF Validator: NODE_SKINNED_MESH_LOCAL_TRANSFORMS](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L450)
                                    DirectX::XMVECTOR gltf_node_model_space_translation;
                                    DirectX::XMVECTOR gltf_node_model_space_scale;
                                    DirectX::XMVECTOR gltf_node_model_space_rotation;
                                    bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&gltf_node_model_space_scale, &gltf_node_model_space_rotation, &gltf_node_model_space_translation, DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index]));
                                    assert(directx_xm_matrix_decompose);

                                    constexpr float const INTERNAL_TRANSLATION_EPSILON = 1E-7F;
                                    constexpr float const INTERNAL_SCALE_EPSILON = 1E-7F;
                                    constexpr float const INTERNAL_ROTATION_EPSILON = 1E-7F;
                                    assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(gltf_node_model_space_translation, DirectX::XMVectorZero())), DirectX::XMVectorReplicate(INTERNAL_TRANSLATION_EPSILON)));
                                    assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(gltf_node_model_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));
                                    assert(DirectX::XMVector4Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(gltf_node_model_space_rotation, DirectX::XMQuaternionIdentity())), DirectX::XMVectorReplicate(INTERNAL_ROTATION_EPSILON)));
                                }
#endif
                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    DirectX::XMFLOAT3 gltf_position = internal_transform_translation(gltf_positions[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                    DirectX::XMFLOAT3 gltf_normal = internal_transform_normal(gltf_normals[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                    DirectX::XMFLOAT4 gltf_tangent = internal_transform_tangent(gltf_tangents[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                    DirectX::XMFLOAT2 gltf_texcoord = gltf_texcoords[gltf_vertex_index];

                                    raw_vertices[gltf_vertex_index].m_position = gltf_position;
                                    raw_vertices[gltf_vertex_index].m_normal = gltf_normal;
                                    raw_vertices[gltf_vertex_index].m_tangent = gltf_tangent;
                                    raw_vertices[gltf_vertex_index].m_texcoord = gltf_texcoord;
                                }
                            }
                            else
                            {
                                for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                {
                                    DirectX::XMFLOAT3 gltf_position = internal_transform_translation(gltf_positions[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                    DirectX::XMFLOAT3 gltf_normal = internal_transform_normal(gltf_normals[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                    DirectX::XMFLOAT4 gltf_tangent = internal_transform_tangent(gltf_tangents[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                    DirectX::XMFLOAT2 gltf_texcoord = gltf_texcoords[gltf_vertex_index];

                                    DirectX::XMFLOAT3 raw_position;
                                    DirectX::XMStoreFloat3(&raw_position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&gltf_position), DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index])));

                                    DirectX::XMFLOAT3 raw_normal;
                                    DirectX::XMStoreFloat3(&raw_normal, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&gltf_normal), DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index])));

                                    DirectX::XMFLOAT4 raw_tangent;
                                    {
                                        DirectX::XMFLOAT3 raw_tangent_xyz;
                                        DirectX::XMFLOAT3 const gltf_tangent_xyz(gltf_tangent.x, gltf_tangent.y, gltf_tangent.z);
                                        DirectX::XMStoreFloat3(&raw_tangent_xyz, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&gltf_tangent_xyz), DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index])));

                                        float const raw_tangent_w = gltf_tangent.w;

                                        raw_tangent = DirectX::XMFLOAT4(raw_tangent_xyz.x, raw_tangent_xyz.y, raw_tangent_xyz.z, raw_tangent_w);
                                    }

                                    raw_vertices[gltf_vertex_index].m_position = raw_position;
                                    raw_vertices[gltf_vertex_index].m_normal = raw_normal;
                                    raw_vertices[gltf_vertex_index].m_tangent = raw_tangent;
                                    raw_vertices[gltf_vertex_index].m_texcoord = gltf_texcoord;
                                }
                            }
                        }

                        for (uint32_t mmd_morph_target_name_index = 0U; mmd_morph_target_name_index < BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT; ++mmd_morph_target_name_index)
                        {
                            BRX_ASSET_IMPORT_MORPH_TARGET_NAME const mmd_morph_target_name = static_cast<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>(mmd_morph_target_name_index);

                            auto found_gltf_morph_target_absolute_positions = gltf_morph_targets_absolute_positions.find(mmd_morph_target_name);
                            auto found_gltf_morph_target_absolute_normals = gltf_morph_targets_absolute_normals.find(mmd_morph_target_name);
                            auto found_gltf_morph_target_absolute_tangents = gltf_morph_targets_absolute_tangents.find(mmd_morph_target_name);
                            auto found_gltf_morph_target_absolute_texcoords = gltf_morph_targets_absolute_texcoords.find(mmd_morph_target_name);

                            if ((gltf_morph_targets_absolute_positions.end() != found_gltf_morph_target_absolute_positions) && (gltf_morph_targets_absolute_normals.end() != found_gltf_morph_target_absolute_normals) && (gltf_morph_targets_absolute_tangents.end() != found_gltf_morph_target_absolute_tangents) && (gltf_morph_targets_absolute_texcoords.end() != found_gltf_morph_target_absolute_texcoords))
                            {
                                auto &raw_morph_target_vertices = raw_morph_targets_vertices[mmd_morph_target_name];

                                assert(raw_morph_target_vertices.empty());
                                raw_morph_target_vertices.resize(gltf_vertex_count);
                                if (NULL != gltf_skin)
                                {
#ifndef NDEBUG
                                    {
                                        // [glTF Validator: NODE_SKINNED_MESH_NON_ROOT](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L444)
                                        // [glTF Validator: NODE_SKINNED_MESH_LOCAL_TRANSFORMS](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L450)
                                        DirectX::XMVECTOR gltf_node_model_space_translation;
                                        DirectX::XMVECTOR gltf_node_model_space_scale;
                                        DirectX::XMVECTOR gltf_node_model_space_rotation;
                                        bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&gltf_node_model_space_scale, &gltf_node_model_space_rotation, &gltf_node_model_space_translation, DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index]));
                                        assert(directx_xm_matrix_decompose);

                                        constexpr float const INTERNAL_TRANSLATION_EPSILON = 1E-7F;
                                        constexpr float const INTERNAL_SCALE_EPSILON = 1E-7F;
                                        constexpr float const INTERNAL_ROTATION_EPSILON = 1E-7F;
                                        assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(gltf_node_model_space_translation, DirectX::XMVectorZero())), DirectX::XMVectorReplicate(INTERNAL_TRANSLATION_EPSILON)));
                                        assert(DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(gltf_node_model_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON)));
                                        assert(DirectX::XMVector4Less(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(gltf_node_model_space_rotation, DirectX::XMQuaternionIdentity())), DirectX::XMVectorReplicate(INTERNAL_ROTATION_EPSILON)));
                                    }
#endif

                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                    {
                                        DirectX::XMFLOAT3 gltf_morph_target_absolute_position = internal_transform_translation(found_gltf_morph_target_absolute_positions->second[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                        DirectX::XMFLOAT3 gltf_morph_target_absolute_normal = internal_transform_normal(found_gltf_morph_target_absolute_normals->second[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                        DirectX::XMFLOAT4 gltf_morph_target_absolute_tangent = internal_transform_tangent(found_gltf_morph_target_absolute_tangents->second[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                        DirectX::XMFLOAT2 gltf_morph_target_absolute_texcoord = found_gltf_morph_target_absolute_texcoords->second[gltf_vertex_index];

                                        DirectX::XMFLOAT3 raw_morph_target_position;
                                        DirectX::XMStoreFloat3(&raw_morph_target_position, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_position), DirectX::XMLoadFloat3(&raw_vertices[gltf_vertex_index].m_position)));

                                        DirectX::XMFLOAT3 raw_morph_target_normal;
                                        DirectX::XMStoreFloat3(&raw_morph_target_normal, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_normal), DirectX::XMLoadFloat3(&raw_vertices[gltf_vertex_index].m_normal)));

                                        DirectX::XMFLOAT4 raw_morph_target_tangent;
                                        DirectX::XMStoreFloat4(&raw_morph_target_tangent, DirectX::XMVectorSubtract(DirectX::XMLoadFloat4(&gltf_morph_target_absolute_tangent), DirectX::XMLoadFloat4(&raw_vertices[gltf_vertex_index].m_tangent)));

                                        DirectX::XMFLOAT2 raw_morph_target_texcoord;
                                        DirectX::XMStoreFloat2(&raw_morph_target_texcoord, DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&gltf_morph_target_absolute_texcoord), DirectX::XMLoadFloat2(&raw_vertices[gltf_vertex_index].m_texcoord)));

                                        raw_morph_target_vertices[gltf_vertex_index].m_position = raw_morph_target_position;
                                        raw_morph_target_vertices[gltf_vertex_index].m_normal = raw_morph_target_normal;
                                        raw_morph_target_vertices[gltf_vertex_index].m_tangent = raw_morph_target_tangent;
                                        raw_morph_target_vertices[gltf_vertex_index].m_texcoord = raw_morph_target_texcoord;
                                    }
                                }
                                else
                                {

                                    for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                    {
                                        DirectX::XMFLOAT3 gltf_morph_target_absolute_position = internal_transform_translation(found_gltf_morph_target_absolute_positions->second[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                        DirectX::XMFLOAT3 gltf_morph_target_absolute_normal = internal_transform_normal(found_gltf_morph_target_absolute_normals->second[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                        DirectX::XMFLOAT4 gltf_morph_target_absolute_tangent = internal_transform_tangent(found_gltf_morph_target_absolute_tangents->second[gltf_vertex_index], found_vrmc_vrm, found_vrm);
                                        DirectX::XMFLOAT2 gltf_morph_target_absolute_texcoord = found_gltf_morph_target_absolute_texcoords->second[gltf_vertex_index];

                                        DirectX::XMFLOAT3 raw_morph_target_position;
                                        DirectX::XMStoreFloat3(&raw_morph_target_position, DirectX::XMVectorSubtract(DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_position), DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index])), DirectX::XMLoadFloat3(&raw_vertices[gltf_vertex_index].m_position)));

                                        DirectX::XMFLOAT3 raw_morph_target_normal;
                                        DirectX::XMStoreFloat3(&raw_morph_target_normal, DirectX::XMVectorSubtract(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&gltf_morph_target_absolute_normal), DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index])), DirectX::XMLoadFloat3(&raw_vertices[gltf_vertex_index].m_normal)));

                                        DirectX::XMFLOAT4 raw_morph_target_tangent;
                                        {
                                            DirectX::XMFLOAT4 raw_tangent;
                                            {
                                                DirectX::XMFLOAT3 raw_tangent_xyz;
                                                DirectX::XMFLOAT3 const gltf_tangent_xyz(gltf_morph_target_absolute_tangent.x, gltf_morph_target_absolute_tangent.y, gltf_morph_target_absolute_tangent.z);
                                                DirectX::XMStoreFloat3(&raw_tangent_xyz, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&gltf_tangent_xyz), DirectX::XMLoadFloat4x4(&in_gltf_node_model_space_matrices[gltf_mesh_instance_node_index])));

                                                float const raw_tangent_w = gltf_morph_target_absolute_tangent.w;

                                                raw_tangent = DirectX::XMFLOAT4(raw_tangent_xyz.x, raw_tangent_xyz.y, raw_tangent_xyz.z, raw_tangent_w);
                                            }

                                            DirectX::XMStoreFloat4(&raw_morph_target_tangent, DirectX::XMVectorSubtract(DirectX::XMLoadFloat4(&raw_tangent), DirectX::XMLoadFloat4(&raw_vertices[gltf_vertex_index].m_tangent)));
                                        }

                                        DirectX::XMFLOAT2 raw_morph_target_texcoord;
                                        DirectX::XMStoreFloat2(&raw_morph_target_texcoord, DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&gltf_morph_target_absolute_texcoord), DirectX::XMLoadFloat2(&raw_vertices[gltf_vertex_index].m_texcoord)));

                                        raw_morph_target_vertices[gltf_vertex_index].m_position = raw_morph_target_position;
                                        raw_morph_target_vertices[gltf_vertex_index].m_normal = raw_morph_target_normal;
                                        raw_morph_target_vertices[gltf_vertex_index].m_tangent = raw_morph_target_tangent;
                                        raw_morph_target_vertices[gltf_vertex_index].m_texcoord = raw_morph_target_texcoord;
                                    }
                                }
                            }
                            else
                            {
                                assert(gltf_morph_targets_absolute_positions.end() == found_gltf_morph_target_absolute_positions);
                                assert(gltf_morph_targets_absolute_normals.end() == found_gltf_morph_target_absolute_normals);
                                assert(gltf_morph_targets_absolute_tangents.end() == found_gltf_morph_target_absolute_tangents);
                                assert(gltf_morph_targets_absolute_texcoords.end() == found_gltf_morph_target_absolute_texcoords);
                            }
                        }
                    }

                    mcrt_vector<internal_vrm_blending_vertex_t> raw_blending_vertices;
                    {
                        uint32_t const gltf_vertex_count = gltf_position_accessor->count;

                        mcrt_vector<DirectX::XMUINT4> gltf_joint_indices;
                        mcrt_vector<DirectX::XMFLOAT4> gltf_joint_weights;
                        {
                            cgltf_accessor const *gltf_joints_accessor = NULL;
                            cgltf_accessor const *gltf_weights_accessor = NULL;
                            {
                                // TODO: we select the smallest index but this may NOT be correct
                                cgltf_int gltf_joints_index = -1;
                                cgltf_int gltf_weights_index = -1;

                                for (size_t gltf_vertex_attribute_index = 0U; gltf_vertex_attribute_index < gltf_primitive->attributes_count; ++gltf_vertex_attribute_index)
                                {
                                    cgltf_attribute const *const gltf_vertex_attribute = gltf_primitive->attributes + gltf_vertex_attribute_index;

                                    switch (gltf_vertex_attribute->type)
                                    {
                                    case cgltf_attribute_type_joints:
                                    {
                                        assert(cgltf_attribute_type_joints == gltf_vertex_attribute->type);

                                        if (NULL == gltf_joints_accessor || gltf_vertex_attribute->index < gltf_joints_index)
                                        {
                                            gltf_joints_accessor = gltf_vertex_attribute->data;
                                            gltf_joints_index = gltf_vertex_attribute->index;
                                        }
                                    }
                                    break;
                                    case cgltf_attribute_type_weights:
                                    {
                                        assert(cgltf_attribute_type_weights == gltf_vertex_attribute->type);

                                        if (NULL == gltf_weights_accessor || gltf_vertex_attribute->index < gltf_weights_index)
                                        {
                                            gltf_weights_accessor = gltf_vertex_attribute->data;
                                            gltf_weights_index = gltf_vertex_attribute->index;
                                        }
                                    }
                                    break;
                                    default:
                                    {
                                        // Do Nothing
                                    }
                                    }
                                }
                            }

                            // skin mesh can NOT be affected by the node transform
                            // but can be affected by the joint indices of the skeleton
                            if ((NULL != gltf_skin) && (NULL != gltf_joints_accessor) && (NULL != gltf_weights_accessor))
                            {
                                // Joint Indices
                                {
                                    assert(gltf_joints_accessor->count == gltf_vertex_count);

                                    assert(gltf_joint_indices.empty());
                                    gltf_joint_indices.resize(gltf_vertex_count);

                                    assert(cgltf_type_vec4 == gltf_joints_accessor->type);

                                    switch (gltf_joints_accessor->component_type)
                                    {
                                    case cgltf_component_type_r_8u:
                                    {
                                        assert(cgltf_component_type_r_8u == gltf_joints_accessor->component_type);

                                        for (uint32_t gltf_vertex_index = 0; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            cgltf_uint gltf_joint_indices_ubyte4[4];
                                            cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_joints_accessor, gltf_vertex_index, gltf_joint_indices_ubyte4, 4U);
                                            assert(result_gltf_accessor_read_uint);

                                            assert(gltf_joint_indices_ubyte4[0] <= static_cast<cgltf_uint>(UINT8_MAX));
                                            assert(gltf_joint_indices_ubyte4[1] <= static_cast<cgltf_uint>(UINT8_MAX));
                                            assert(gltf_joint_indices_ubyte4[2] <= static_cast<cgltf_uint>(UINT8_MAX));
                                            assert(gltf_joint_indices_ubyte4[3] <= static_cast<cgltf_uint>(UINT8_MAX));

                                            uint16_t raw_joint_index_x = static_cast<uint16_t>(gltf_joint_indices_ubyte4[0]);
                                            uint16_t raw_joint_index_y = static_cast<uint16_t>(gltf_joint_indices_ubyte4[1]);
                                            uint16_t raw_joint_index_z = static_cast<uint16_t>(gltf_joint_indices_ubyte4[2]);
                                            uint16_t raw_joint_index_w = static_cast<uint16_t>(gltf_joint_indices_ubyte4[3]);

                                            uint32_t const skeleton_joint_index_x = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_x])];
                                            uint32_t const skeleton_joint_index_y = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_y])];
                                            uint32_t const skeleton_joint_index_z = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_z])];
                                            uint32_t const skeleton_joint_index_w = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_w])];
                                            assert(skeleton_joint_index_x <= static_cast<uint32_t>(UINT16_MAX));
                                            assert(skeleton_joint_index_y <= static_cast<uint32_t>(UINT16_MAX));
                                            assert(skeleton_joint_index_z <= static_cast<uint32_t>(UINT16_MAX));
                                            assert(skeleton_joint_index_w <= static_cast<uint32_t>(UINT16_MAX));

                                            gltf_joint_indices[gltf_vertex_index] = DirectX::XMUINT4(skeleton_joint_index_x, skeleton_joint_index_y, skeleton_joint_index_z, skeleton_joint_index_w);
                                        }
                                    }
                                    break;
                                    case cgltf_component_type_r_16u:
                                    {
                                        assert(cgltf_component_type_r_16u == gltf_joints_accessor->component_type);

                                        for (uint32_t gltf_vertex_index = 0; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            cgltf_uint gltf_joint_indices_ushort4[4];
                                            cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_joints_accessor, gltf_vertex_index, gltf_joint_indices_ushort4, 4U);
                                            assert(result_gltf_accessor_read_uint);

                                            assert(gltf_joint_indices_ushort4[0] <= static_cast<cgltf_uint>(UINT16_MAX));
                                            assert(gltf_joint_indices_ushort4[1] <= static_cast<cgltf_uint>(UINT16_MAX));
                                            assert(gltf_joint_indices_ushort4[2] <= static_cast<cgltf_uint>(UINT16_MAX));
                                            assert(gltf_joint_indices_ushort4[3] <= static_cast<cgltf_uint>(UINT16_MAX));

                                            uint16_t raw_joint_index_x = gltf_joint_indices_ushort4[0];
                                            uint16_t raw_joint_index_y = gltf_joint_indices_ushort4[1];
                                            uint16_t raw_joint_index_z = gltf_joint_indices_ushort4[2];
                                            uint16_t raw_joint_index_w = gltf_joint_indices_ushort4[3];

                                            uint32_t const skeleton_joint_index_x = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_x])];
                                            uint32_t const skeleton_joint_index_y = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_y])];
                                            uint32_t const skeleton_joint_index_z = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_z])];
                                            uint32_t const skeleton_joint_index_w = in_gltf_node_to_animation_skeleton_joint_map[cgltf_node_index(in_gltf_data, gltf_skin->joints[raw_joint_index_w])];
                                            assert(skeleton_joint_index_x <= static_cast<uint32_t>(UINT16_MAX));
                                            assert(skeleton_joint_index_y <= static_cast<uint32_t>(UINT16_MAX));
                                            assert(skeleton_joint_index_z <= static_cast<uint32_t>(UINT16_MAX));
                                            assert(skeleton_joint_index_w <= static_cast<uint32_t>(UINT16_MAX));

                                            gltf_joint_indices[gltf_vertex_index] = DirectX::XMUINT4(skeleton_joint_index_x, skeleton_joint_index_y, skeleton_joint_index_z, skeleton_joint_index_w);
                                        }
                                    }
                                    break;
                                    default:
                                        assert(0);
                                    }
                                }

                                // Joint Weights
                                {
                                    assert(gltf_weights_accessor->count == gltf_vertex_count);
                                    assert(gltf_joint_weights.empty());
                                    gltf_joint_weights.resize(gltf_weights_accessor->count);

                                    assert(cgltf_type_vec4 == gltf_weights_accessor->type);

                                    switch (gltf_weights_accessor->component_type)
                                    {
                                    case cgltf_component_type_r_8u:
                                    {
                                        assert(cgltf_component_type_r_8u == gltf_weights_accessor->component_type);

                                        for (uint32_t gltf_vertex_index = 0; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            cgltf_uint gltf_joint_weights_ubyte4[4];
                                            cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_weights_accessor, gltf_vertex_index, gltf_joint_weights_ubyte4, 4U);
                                            assert(result_gltf_accessor_read_uint);

                                            assert(gltf_joint_weights_ubyte4[0] <= static_cast<cgltf_uint>(UINT8_MAX));
                                            assert(gltf_joint_weights_ubyte4[1] <= static_cast<cgltf_uint>(UINT8_MAX));
                                            assert(gltf_joint_weights_ubyte4[2] <= static_cast<cgltf_uint>(UINT8_MAX));
                                            assert(gltf_joint_weights_ubyte4[3] <= static_cast<cgltf_uint>(UINT8_MAX));

                                            DirectX::PackedVector::XMUBYTEN4 packed_vector_ubyten4(static_cast<uint8_t>(gltf_joint_weights_ubyte4[0]), static_cast<uint8_t>(gltf_joint_weights_ubyte4[1]), static_cast<uint8_t>(gltf_joint_weights_ubyte4[2]), static_cast<uint8_t>(gltf_joint_weights_ubyte4[3]));

                                            DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN4(&packed_vector_ubyten4);

                                            DirectX::XMStoreFloat4(&gltf_joint_weights[gltf_vertex_index], unpacked_vector);
                                        }
                                    }
                                    break;
                                    case cgltf_component_type_r_16u:
                                    {
                                        assert(cgltf_component_type_r_16u == gltf_weights_accessor->component_type);

                                        for (uint32_t gltf_vertex_index = 0; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            cgltf_uint gltf_joint_weights_ushortn4[4];
                                            cgltf_bool result_gltf_accessor_read_uint = cgltf_accessor_read_uint(gltf_weights_accessor, gltf_vertex_index, gltf_joint_weights_ushortn4, 4U);
                                            assert(result_gltf_accessor_read_uint);

                                            assert(gltf_joint_weights_ushortn4[0] <= static_cast<cgltf_uint>(UINT16_MAX));
                                            assert(gltf_joint_weights_ushortn4[1] <= static_cast<cgltf_uint>(UINT16_MAX));
                                            assert(gltf_joint_weights_ushortn4[2] <= static_cast<cgltf_uint>(UINT16_MAX));
                                            assert(gltf_joint_weights_ushortn4[3] <= static_cast<cgltf_uint>(UINT16_MAX));

                                            DirectX::PackedVector::XMUSHORTN4 packed_vector_ushortn4(static_cast<uint16_t>(gltf_joint_weights_ushortn4[0]), static_cast<uint16_t>(gltf_joint_weights_ushortn4[1]), static_cast<uint16_t>(gltf_joint_weights_ushortn4[2]), static_cast<uint16_t>(gltf_joint_weights_ushortn4[3]));

                                            DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN4(&packed_vector_ushortn4);

                                            DirectX::XMStoreFloat4(&gltf_joint_weights[gltf_vertex_index], unpacked_vector);
                                        }
                                    }
                                    break;
                                    case cgltf_component_type_r_32f:
                                    {
                                        assert(cgltf_component_type_r_32f == gltf_weights_accessor->component_type);

                                        for (uint32_t gltf_vertex_index = 0; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                                        {
                                            cgltf_float gltf_joint_weights_float4[4];
                                            cgltf_bool result_gltf_accessor_read_float = cgltf_accessor_read_float(gltf_weights_accessor, gltf_vertex_index, gltf_joint_weights_float4, 4U);
                                            assert(result_gltf_accessor_read_float);

                                            gltf_joint_weights[gltf_vertex_index] = DirectX::XMFLOAT4(gltf_joint_weights_float4[0], gltf_joint_weights_float4[1], gltf_joint_weights_float4[2], gltf_joint_weights_float4[3]);
                                        }
                                    }
                                    break;
                                    default:
                                        assert(0);
                                    }
                                }
                            }
                            else
                            {
                                assert(NULL == gltf_skin);
                                assert(NULL == gltf_joints_accessor);
                                assert(NULL == gltf_weights_accessor);

                                assert(gltf_joint_indices.empty());
                                assert(gltf_joint_weights.empty());
                            }
                        }

                        if (NULL != gltf_skin)
                        {
                            assert(raw_blending_vertices.empty());
                            raw_blending_vertices.resize(gltf_vertex_count);

                            for (uint32_t gltf_vertex_index = 0U; gltf_vertex_index < gltf_vertex_count; ++gltf_vertex_index)
                            {
                                raw_blending_vertices[gltf_vertex_index].m_indices = gltf_joint_indices[gltf_vertex_index];
                                raw_blending_vertices[gltf_vertex_index].m_weights = gltf_joint_weights[gltf_vertex_index];
                            }
                        }
                        else
                        {
                            assert(gltf_joint_indices.empty());
                            assert(gltf_joint_weights.empty());

                            assert(raw_blending_vertices.empty());
                        }
                    }

                    struct internal_mesh_section_face_information
                    {
                        bool m_skin;
                        bool m_morph;
                        mcrt_vector<uint32_t> m_face_indices;
                    };

                    mcrt_vector<internal_mesh_section_face_information> mesh_sections_face_information;
                    {
                        mcrt_set<uint32_t> morph_face_indices;
                        mcrt_set<uint32_t> non_morph_face_indices;
                        {
                            mcrt_set<uint32_t> morph_lower_bound_vertex_indices;
                            {
                                if (!raw_morph_targets_vertices.empty())
                                {
                                    uint32_t const vertex_count = raw_vertices.size();

                                    for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
                                    {
                                        bool morph_target_vertex = false;

                                        for (auto const &raw_morph_target_name_and_vertices : raw_morph_targets_vertices)
                                        {
                                            auto const &raw_morph_target_vertices = raw_morph_target_name_and_vertices.second;
                                            assert(raw_morph_target_vertices.size() == vertex_count);

                                            auto const &raw_morph_target_vertex = raw_morph_target_vertices[vertex_index];

                                            constexpr float const INTERNAL_MORPH_TARGET_POSITION_EPSILON = 1E-7F;
                                            constexpr float const INTERNAL_MORPH_TARGET_NORMAL_EPSILON = 1E-7F;
                                            constexpr float const INTERNAL_MORPH_TARGET_TANGENT_EPSILON = 1E-7F;
                                            constexpr float const INTERNAL_MORPH_TARGET_TEXCOORD_EPSILON = 1E-7F;

                                            bool const non_morph_target_position = DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMLoadFloat3(&raw_morph_target_vertex.m_position)), DirectX::XMVectorReplicate(INTERNAL_MORPH_TARGET_POSITION_EPSILON));

                                            bool const non_morph_target_normal = DirectX::XMVector3Less(DirectX::XMVectorAbs(DirectX::XMLoadFloat3(&raw_morph_target_vertex.m_normal)), DirectX::XMVectorReplicate(INTERNAL_MORPH_TARGET_NORMAL_EPSILON));

                                            bool const non_morph_target_tangent = DirectX::XMVector4Less(DirectX::XMVectorAbs(DirectX::XMLoadFloat4(&raw_morph_target_vertex.m_tangent)), DirectX::XMVectorReplicate(INTERNAL_MORPH_TARGET_TANGENT_EPSILON));

                                            bool const non_morph_target_texcoord = DirectX::XMVector2Less(DirectX::XMVectorAbs(DirectX::XMLoadFloat2(&raw_morph_target_vertices[vertex_index].m_texcoord)), DirectX::XMVectorReplicate(INTERNAL_MORPH_TARGET_TEXCOORD_EPSILON));

                                            if ((!non_morph_target_position) || (!non_morph_target_normal) || (!non_morph_target_tangent) || (!non_morph_target_texcoord))
                                            {
                                                morph_target_vertex = true;
                                                break;
                                            }
                                        }

                                        if (morph_target_vertex)
                                        {
                                            morph_lower_bound_vertex_indices.emplace(vertex_index);
                                        }
                                    }
                                }
                                else
                                {
                                    assert(morph_lower_bound_vertex_indices.empty());
                                }
                            }

                            {
                                assert(0U == (raw_indices.size() % 3U));
                                uint32_t const face_count = raw_indices.size() / 3U;

                                mcrt_map<uint32_t, mcrt_vector<uint32_t>> vertex_index_to_face_indices_map;
                                for (uint32_t face_index = 0U; face_index < face_count; ++face_index)
                                {
                                    vertex_index_to_face_indices_map[raw_indices[3U * face_index + 0U]].push_back(face_index);
                                    vertex_index_to_face_indices_map[raw_indices[3U * face_index + 1U]].push_back(face_index);
                                    vertex_index_to_face_indices_map[raw_indices[3U * face_index + 2U]].push_back(face_index);
                                }

                                for (uint32_t const morph_lower_bound_vertex_index : morph_lower_bound_vertex_indices)
                                {
                                    for (uint32_t const face_index : vertex_index_to_face_indices_map[morph_lower_bound_vertex_index])
                                    {
                                        morph_face_indices.emplace(face_index);
                                    }
                                }

                                for (uint32_t face_index = 0U; face_index < face_count; ++face_index)
                                {
                                    if (morph_face_indices.end() == morph_face_indices.find(face_index))
                                    {
                                        non_morph_face_indices.emplace(face_index);
                                    }
                                }

                                assert((morph_face_indices.size() + non_morph_face_indices.size()) == face_count);
                            }
                        }

                        mesh_sections_face_information.resize(2U);

                        mesh_sections_face_information[0].m_skin = (!raw_blending_vertices.empty());
                        mesh_sections_face_information[0].m_morph = true;
                        mesh_sections_face_information[0].m_face_indices.reserve(morph_face_indices.size());
                        for (uint32_t const morph_face_index : morph_face_indices)
                        {
                            mesh_sections_face_information[0].m_face_indices.push_back(morph_face_index);
                        }

                        mesh_sections_face_information[1].m_skin = (!raw_blending_vertices.empty());
                        mesh_sections_face_information[1].m_morph = false;
                        mesh_sections_face_information[1].m_face_indices.reserve(non_morph_face_indices.size());
                        for (uint32_t const non_morph_face_index : non_morph_face_indices)
                        {
                            mesh_sections_face_information[1].m_face_indices.push_back(non_morph_face_index);
                        }

#ifndef NDEBUG
                        {
                            assert(0U == (raw_indices.size() % 3U));
                            uint32_t const face_count = raw_indices.size() / 3U;

                            mcrt_set<uint32_t> face_indices;

                            for (auto const &mesh_section_face_information : mesh_sections_face_information)
                            {
                                for (uint32_t const mesh_section_face_index : mesh_section_face_information.m_face_indices)
                                {
                                    assert(face_indices.end() == face_indices.find(mesh_section_face_index));
                                    face_indices.emplace_hint(face_indices.end(), mesh_section_face_index);
                                }
                            }

                            assert(face_count == face_indices.size());
                        }
#endif
                    }

                    for (auto const &mesh_section_face_information : mesh_sections_face_information)
                    {
                        bool const mesh_section_skin_switch = mesh_section_face_information.m_skin;
                        bool const mesh_section_morph_switch = mesh_section_face_information.m_morph;
                        auto const &mesh_section_face_indices = mesh_section_face_information.m_face_indices;

                        mcrt_vector<internal_vrm_vertex_t> new_vertices;
                        mcrt_vector<internal_vrm_blending_vertex_t> new_blending_vertices;
                        mcrt_map<BRX_ASSET_IMPORT_MORPH_TARGET_NAME, mcrt_vector<internal_vrm_morph_target_vertex_t>> new_morph_targets_vertices;
                        mcrt_vector<uint32_t> new_indices;
                        {
                            mcrt_set<uint32_t> original_vertex_indices;
                            for (uint32_t const mesh_section_face_index : mesh_section_face_indices)
                            {
                                for (uint32_t face_vertex_index = 0U; face_vertex_index < 3U; ++face_vertex_index)
                                {
                                    uint32_t const original_vertex_index = raw_indices[3U * mesh_section_face_index + face_vertex_index];

                                    auto const found_vertex = original_vertex_indices.find(original_vertex_index);

                                    if (original_vertex_indices.end() == found_vertex)
                                    {
                                        original_vertex_indices.emplace_hint(found_vertex, original_vertex_index);
                                    }
                                }
                            }

                            mcrt_map<uint32_t, uint32_t> original_vertex_index_to_new_vertex_index;
                            for (auto const &original_vertex_index : original_vertex_indices)
                            {
                                uint32_t const new_vertex_index = new_vertices.size();

                                new_vertices.push_back(raw_vertices[original_vertex_index]);

                                if (mesh_section_skin_switch)
                                {
                                    assert(raw_blending_vertices.size() == raw_vertices.size());
                                    assert(NULL != gltf_skin);

                                    new_blending_vertices.push_back(raw_blending_vertices[original_vertex_index]);
                                }
                                else
                                {
                                    assert(raw_blending_vertices.empty());
                                    assert(NULL == gltf_skin);

                                    assert(new_blending_vertices.empty());
                                }

                                if (mesh_section_morph_switch)
                                {
                                    for (auto const &original_morph_target_name_and_vertices : raw_morph_targets_vertices)
                                    {
                                        BRX_ASSET_IMPORT_MORPH_TARGET_NAME const original_morph_target_name = original_morph_target_name_and_vertices.first;
                                        auto const &original_morph_target_vertices = original_morph_target_name_and_vertices.second;

                                        auto &new_morph_target_vertices = new_morph_targets_vertices[original_morph_target_name];

                                        new_morph_target_vertices.push_back(original_morph_target_vertices[original_vertex_index]);
                                    }
                                }

                                original_vertex_index_to_new_vertex_index[original_vertex_index] = new_vertex_index;
                            }

                            for (uint32_t const mesh_section_face_index : mesh_section_face_indices)
                            {
                                for (uint32_t face_vertex_index = 0U; face_vertex_index < 3U; ++face_vertex_index)
                                {
                                    uint32_t const original_vertex_index = raw_indices[3U * mesh_section_face_index + face_vertex_index];

                                    auto const found_new_vertex_index = original_vertex_index_to_new_vertex_index.find(original_vertex_index);
                                    assert(original_vertex_index_to_new_vertex_index.end() != found_new_vertex_index);

                                    assert(original_vertex_index == found_new_vertex_index->first);
                                    uint32_t const new_vertex_index = found_new_vertex_index->second;

                                    new_indices.push_back(new_vertex_index);
                                }
                            }
                        }

                        uint32_t const new_vertex_count = new_vertices.size();
#ifndef NDEBUG
                        {
                            assert(new_blending_vertices.empty() || new_blending_vertices.size() == new_vertex_count);

                            for (auto const &new_morph_target_name_and_vertices : new_morph_targets_vertices)
                            {
                                auto const &new_morph_target_vertices = new_morph_target_name_and_vertices.second;

                                assert(new_morph_target_vertices.size() == new_vertex_count);
                            }
                        }
#endif

                        auto &material_mesh_section_vector = materials_mesh_section_vector[cgltf_material_index(in_gltf_data, gltf_material)];

                        uint32_t material_mesh_section_vector_index;
                        if (mesh_section_skin_switch)
                        {
                            if (mesh_section_morph_switch)
                            {
                                material_mesh_section_vector_index = MATERIAL_MESH_SECTION_VECTOR_SKIN_MORPH_INDEX;
                            }
                            else
                            {
                                material_mesh_section_vector_index = MATERIAL_MESH_SECTION_VECTOR_SKIN_NON_MORPH_INDEX;
                            }
                        }
                        else
                        {
                            if (mesh_section_morph_switch)
                            {
                                material_mesh_section_vector_index = MATERIAL_MESH_SECTION_VECTOR_NON_SKIN_MORPH_INDEX;
                            }
                            else
                            {
                                material_mesh_section_vector_index = MATERIAL_MESH_SECTION_VECTOR_NON_SKIN_NON_MORPH_INDEX;
                            }
                        }

                        auto &material_mesh_section = material_mesh_section_vector[material_mesh_section_vector_index];

                        uint32_t const material_mesh_section_vertex_count = material_mesh_section.m_vertices.size();

#ifndef NDEBUG
                        {
                            assert(material_mesh_section.m_blending_vertices.empty() || material_mesh_section.m_blending_vertices.size() == material_mesh_section_vertex_count);

                            for (auto const &material_mesh_section_morph_target_name_and_vertices : material_mesh_section.m_morph_targets_vertices)
                            {
                                auto const &material_mesh_section_morph_target_vertices = material_mesh_section_morph_target_name_and_vertices.second;

                                assert(material_mesh_section_morph_target_vertices.size() == material_mesh_section_vertex_count);
                            }
                        }
#endif

                        material_mesh_section.m_vertices.resize(material_mesh_section_vertex_count + new_vertex_count);

                        for (uint32_t new_vertex_index = 0; new_vertex_index < new_vertex_count; ++new_vertex_index)
                        {
                            material_mesh_section.m_vertices[material_mesh_section_vertex_count + new_vertex_index] = new_vertices[new_vertex_index];
                        }

                        if (mesh_section_skin_switch)
                        {
                            assert(material_mesh_section.m_blending_vertices.size() == material_mesh_section_vertex_count);
                            assert(new_blending_vertices.size() == new_vertex_count);

                            material_mesh_section.m_blending_vertices.resize(material_mesh_section_vertex_count + new_vertex_count);

                            for (uint32_t new_vertex_index = 0; new_vertex_index < new_vertex_count; ++new_vertex_index)
                            {
                                material_mesh_section.m_blending_vertices[material_mesh_section_vertex_count + new_vertex_index] = new_blending_vertices[new_vertex_index];
                            }
                        }
                        else
                        {
                            assert(material_mesh_section.m_blending_vertices.empty());
                            assert(new_blending_vertices.empty());
                        }

                        if (mesh_section_morph_switch)
                        {
                            for (auto const &new_morph_target_name_and_vertices : new_morph_targets_vertices)
                            {
                                BRX_ASSET_IMPORT_MORPH_TARGET_NAME const new_morph_target_name = new_morph_target_name_and_vertices.first;
                                auto const &new_morph_target_vertices = new_morph_target_name_and_vertices.second;

                                auto &material_mesh_section_morph_target_vertices = material_mesh_section.m_morph_targets_vertices[new_morph_target_name];
                                assert(material_mesh_section_morph_target_vertices.empty() || material_mesh_section_morph_target_vertices.size() == material_mesh_section_vertex_count);

                                material_mesh_section_morph_target_vertices.resize(material_mesh_section_vertex_count + new_vertex_count, internal_vrm_morph_target_vertex_t{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F, 0.0F}, {0.0F, 0.0F}});

                                for (uint32_t new_vertex_index = 0; new_vertex_index < new_vertex_count; ++new_vertex_index)
                                {
                                    material_mesh_section_morph_target_vertices[material_mesh_section_vertex_count + new_vertex_index] = new_morph_target_vertices[new_vertex_index];
                                }
                            }
                        }
                        else
                        {
                            assert(material_mesh_section.m_morph_targets_vertices.empty());
                            assert(new_morph_targets_vertices.empty());
                        }

                        uint32_t const new_index_count = new_indices.size();

                        uint32_t const material_mesh_section_index_count = material_mesh_section.m_indices.size();

                        material_mesh_section.m_indices.resize(material_mesh_section_index_count + new_index_count);

                        for (uint32_t new_index_index = 0; new_index_index < new_index_count; ++new_index_index)
                        {
                            material_mesh_section.m_indices[material_mesh_section_index_count + new_index_index] = (material_mesh_section_vertex_count + new_indices[new_index_index]);
                        }
                    }
                }
            }
        }
    }

    for (auto &material_mesh_section_vector : materials_mesh_section_vector)
    {
        uint32_t const gltf_material_index = material_mesh_section_vector.first;

        for (auto &material_mesh_section : material_mesh_section_vector.second)
        {
            if ((!material_mesh_section.m_vertices.empty()) && (!material_mesh_section.m_indices.empty()))
            {
#ifndef NDEBUG
                {
                    assert(material_mesh_section.m_blending_vertices.empty() || material_mesh_section.m_blending_vertices.size() == material_mesh_section.m_vertices.size());

                    for (auto const &material_mesh_section_morph_target_name_and_vertices : material_mesh_section.m_morph_targets_vertices)
                    {
                        auto const &material_mesh_section_morph_target_vertices = material_mesh_section_morph_target_name_and_vertices.second;

                        assert(material_mesh_section_morph_target_vertices.size() == material_mesh_section.m_vertices.size());
                    }
                }
#endif

                out_materials_and_mesh_sections.emplace_back(gltf_material_index, std::move(material_mesh_section));
            }
            else
            {
                assert(material_mesh_section.m_vertices.empty());
                assert(material_mesh_section.m_blending_vertices.empty());
                assert(material_mesh_section.m_morph_targets_vertices.empty());
                assert(material_mesh_section.m_indices.empty());
            }
        }
    }
}

static inline void internal_import_material(cgltf_material const *const in_gltf_material, bool &out_is_double_sided, mcrt_vector<uint8_t> &out_emissive_image_url, uint32_t &out_emissive_factor, mcrt_vector<uint8_t> &out_normal_image_url, uint64_t &out_normal_scale, mcrt_vector<uint8_t> &out_base_color_image_url, uint32_t &out_base_color_factor, mcrt_vector<uint8_t> &out_metallic_roughness_image_url, uint32_t &out_metallic_roughness_factor)
{
    // double sided
    out_is_double_sided = in_gltf_material->double_sided;

    // emissive
    {
        {
            DirectX::PackedVector::XMUBYTEN4 packed_emissive_factor;
            DirectX::XMFLOAT4 const gltf_emissive_factor(in_gltf_material->emissive_factor[0], in_gltf_material->emissive_factor[1], in_gltf_material->emissive_factor[2], 0.0F);
            DirectX::PackedVector::XMStoreUByteN4(&packed_emissive_factor, DirectX::XMLoadFloat4(&gltf_emissive_factor));
            out_emissive_factor = packed_emissive_factor.v;
        }

        cgltf_texture const *const gltf_emissive_texture = in_gltf_material->emissive_texture.texture;
        cgltf_image const *const gltf_emissive_image = (NULL != gltf_emissive_texture) ? (gltf_emissive_texture->has_webp ? gltf_emissive_texture->webp_image : gltf_emissive_texture->image) : NULL;
        if (NULL != gltf_emissive_image)
        {
            if (NULL != gltf_emissive_image->buffer_view)
            {
                assert(NULL == gltf_emissive_image->uri);

                uint8_t const uri_scheme_string[] = {'d', 'a', 't', 'a', ':', '/', '/'};
                size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                uint8_t const *const buffer_view_data_base = cgltf_buffer_view_data(gltf_emissive_image->buffer_view);
                size_t const buffer_view_data_size = gltf_emissive_image->buffer_view->size;

                assert(out_emissive_image_url.empty());
                out_emissive_image_url.resize(uri_scheme_length + buffer_view_data_size);
                std::memcpy(out_emissive_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                std::memcpy(out_emissive_image_url.data() + uri_scheme_length, buffer_view_data_base, sizeof(uint8_t) * buffer_view_data_size);
            }
            else if (NULL != gltf_emissive_image->uri)
            {
                assert(NULL == gltf_emissive_image->buffer_view);

                uint8_t const uri_scheme_string[] = {'f', 'i', 'l', 'e', ':', '/', '/'};
                size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                cgltf_decode_uri(gltf_emissive_image->uri);

                char const *const file_path_string = gltf_emissive_image->uri;
                size_t const file_path_length = std::strlen(gltf_emissive_image->uri);

                assert(out_emissive_image_url.empty());
                out_emissive_image_url.resize(uri_scheme_length + file_path_length);
                std::memcpy(out_emissive_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                std::memcpy(out_emissive_image_url.data() + uri_scheme_length, file_path_string, sizeof(char) * file_path_length);
            }
            else
            {
                assert(false);
                assert(out_emissive_image_url.empty());
            }
        }
        else
        {
            assert(NULL == gltf_emissive_texture);
            assert(out_emissive_image_url.empty());
        }
    }

    // normal
    {
        {
            DirectX::PackedVector::XMUSHORTN4 packed_normal_factor;
            DirectX::XMFLOAT4 const gltf_normal_factor(in_gltf_material->normal_texture.scale, in_gltf_material->normal_texture.scale, 1.0, 0.0F);
            DirectX::PackedVector::XMStoreUShortN4(&packed_normal_factor, DirectX::XMLoadFloat4(&gltf_normal_factor));
            out_normal_scale = packed_normal_factor.v;
        }

        cgltf_texture const *const gltf_normal_texture = in_gltf_material->normal_texture.texture;
        cgltf_image const *const gltf_normal_image = (NULL != gltf_normal_texture) ? (gltf_normal_texture->has_webp ? gltf_normal_texture->webp_image : gltf_normal_texture->image) : NULL;
        if (NULL != gltf_normal_image)
        {
            if (NULL != gltf_normal_image->buffer_view)
            {
                assert(NULL == gltf_normal_image->uri);

                uint8_t const uri_scheme_string[] = {'d', 'a', 't', 'a', ':', '/', '/'};
                size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                uint8_t const *const buffer_view_data_base = cgltf_buffer_view_data(gltf_normal_image->buffer_view);
                size_t const buffer_view_data_size = gltf_normal_image->buffer_view->size;

                assert(out_normal_image_url.empty());
                out_normal_image_url.resize(uri_scheme_length + buffer_view_data_size);
                std::memcpy(out_normal_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                std::memcpy(out_normal_image_url.data() + uri_scheme_length, buffer_view_data_base, sizeof(uint8_t) * buffer_view_data_size);
            }
            else if (NULL != gltf_normal_image->uri)
            {
                assert(NULL == gltf_normal_image->buffer_view);

                uint8_t const uri_scheme_string[] = {'f', 'i', 'l', 'e', ':', '/', '/'};
                size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                cgltf_decode_uri(gltf_normal_image->uri);

                char const *const file_path_string = gltf_normal_image->uri;
                size_t const file_path_length = std::strlen(gltf_normal_image->uri);

                assert(out_normal_image_url.empty());
                out_normal_image_url.resize(uri_scheme_length + file_path_length);
                std::memcpy(out_normal_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                std::memcpy(out_normal_image_url.data() + uri_scheme_length, file_path_string, sizeof(char) * file_path_length);
            }
            else
            {
                assert(false);
                assert(out_normal_image_url.empty());
            }
        }
        else
        {
            assert(NULL == gltf_normal_texture);
            assert(out_normal_image_url.empty());
        }
    }

    // base color
    {
        if (in_gltf_material->has_pbr_metallic_roughness)
        {
            {
                DirectX::PackedVector::XMUBYTEN4 packed_base_color_factor;
                DirectX::XMFLOAT4 const gltf_base_color_factor(in_gltf_material->pbr_metallic_roughness.base_color_factor);
                DirectX::PackedVector::XMStoreUByteN4(&packed_base_color_factor, DirectX::XMLoadFloat4(&gltf_base_color_factor));
                out_base_color_factor = packed_base_color_factor.v;
            }

            cgltf_texture const *const gltf_base_color_texture = in_gltf_material->pbr_metallic_roughness.base_color_texture.texture;
            cgltf_image const *const gltf_base_color_image = (NULL != gltf_base_color_texture) ? (gltf_base_color_texture->has_webp ? gltf_base_color_texture->webp_image : gltf_base_color_texture->image) : NULL;
            if (NULL != gltf_base_color_image)
            {
                if (NULL != gltf_base_color_image->buffer_view)
                {
                    assert(NULL == gltf_base_color_image->uri);

                    uint8_t const uri_scheme_string[] = {'d', 'a', 't', 'a', ':', '/', '/'};
                    size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                    uint8_t const *const buffer_view_data_base = cgltf_buffer_view_data(gltf_base_color_image->buffer_view);
                    size_t const buffer_view_data_size = gltf_base_color_image->buffer_view->size;

                    assert(out_base_color_image_url.empty());
                    out_base_color_image_url.resize(uri_scheme_length + buffer_view_data_size);
                    std::memcpy(out_base_color_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                    std::memcpy(out_base_color_image_url.data() + uri_scheme_length, buffer_view_data_base, sizeof(uint8_t) * buffer_view_data_size);
                }
                else if (NULL != gltf_base_color_image->uri)
                {
                    assert(NULL == gltf_base_color_image->buffer_view);

                    uint8_t const uri_scheme_string[] = {'f', 'i', 'l', 'e', ':', '/', '/'};
                    size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                    cgltf_decode_uri(gltf_base_color_image->uri);

                    char const *const file_path_string = gltf_base_color_image->uri;
                    size_t const file_path_length = std::strlen(gltf_base_color_image->uri);

                    assert(out_base_color_image_url.empty());
                    out_base_color_image_url.resize(uri_scheme_length + file_path_length);
                    std::memcpy(out_base_color_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                    std::memcpy(out_base_color_image_url.data() + uri_scheme_length, file_path_string, sizeof(char) * file_path_length);
                }
                else
                {
                    assert(false);
                    assert(out_base_color_image_url.empty());
                }
            }
            else
            {
                assert(NULL == gltf_base_color_texture);
                assert(out_base_color_image_url.empty());
            }
        }
        else
        {

            DirectX::PackedVector::XMUBYTEN4 packed_base_color_factor;
            DirectX::XMFLOAT4 const gltf_base_color_factor(1.0F, 1.0F, 1.0F, 1.0F);
            DirectX::PackedVector::XMStoreUByteN4(&packed_base_color_factor, DirectX::XMLoadFloat4(&gltf_base_color_factor));
            out_base_color_factor = packed_base_color_factor.v;
        }
    }

    // roughness metallic
    {
        if (in_gltf_material->has_pbr_metallic_roughness)
        {
            {
                DirectX::PackedVector::XMUBYTEN4 packed_metallic_roughness_factor;
                DirectX::XMFLOAT4 const metallic_roughness_factor(0.0F, in_gltf_material->pbr_metallic_roughness.roughness_factor, in_gltf_material->pbr_metallic_roughness.metallic_factor, 0.0F);
                DirectX::PackedVector::XMStoreUByteN4(&packed_metallic_roughness_factor, DirectX::XMLoadFloat4(&metallic_roughness_factor));
                out_metallic_roughness_factor = packed_metallic_roughness_factor.v;
            }

            cgltf_texture const *const gltf_metallic_roughness_texture = in_gltf_material->pbr_metallic_roughness.metallic_roughness_texture.texture;
            cgltf_image const *const gltf_metallic_roughness_image = (NULL != gltf_metallic_roughness_texture) ? (gltf_metallic_roughness_texture->webp_image ? gltf_metallic_roughness_texture->webp_image : gltf_metallic_roughness_texture->image) : NULL;
            if (NULL != gltf_metallic_roughness_image)
            {
                if (NULL != gltf_metallic_roughness_image->buffer_view)
                {
                    assert(NULL == gltf_metallic_roughness_image->uri);

                    uint8_t const uri_scheme_string[] = {'d', 'a', 't', 'a', ':', '/', '/'};
                    size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                    uint8_t const *const buffer_view_data_base = cgltf_buffer_view_data(gltf_metallic_roughness_image->buffer_view);
                    size_t const buffer_view_data_size = gltf_metallic_roughness_image->buffer_view->size;

                    assert(out_metallic_roughness_image_url.empty());
                    out_metallic_roughness_image_url.resize(uri_scheme_length + buffer_view_data_size);
                    std::memcpy(out_metallic_roughness_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                    std::memcpy(out_metallic_roughness_image_url.data() + uri_scheme_length, buffer_view_data_base, sizeof(uint8_t) * buffer_view_data_size);
                }
                else if (NULL != gltf_metallic_roughness_image->uri)
                {
                    assert(NULL == gltf_metallic_roughness_image->buffer_view);

                    uint8_t const uri_scheme_string[] = {'f', 'i', 'l', 'e', ':', '/', '/'};
                    size_t const uri_scheme_length = (sizeof(uri_scheme_string) / sizeof(uri_scheme_string[0]));

                    cgltf_decode_uri(gltf_metallic_roughness_image->uri);

                    char const *const file_path_string = gltf_metallic_roughness_image->uri;
                    size_t const file_path_length = std::strlen(gltf_metallic_roughness_image->uri);

                    assert(out_metallic_roughness_image_url.empty());
                    out_metallic_roughness_image_url.resize(uri_scheme_length + file_path_length);
                    std::memcpy(out_metallic_roughness_image_url.data(), uri_scheme_string, sizeof(uint8_t) * uri_scheme_length);
                    std::memcpy(out_metallic_roughness_image_url.data() + uri_scheme_length, file_path_string, sizeof(char) * file_path_length);
                }
                else
                {
                    assert(false);
                    assert(out_metallic_roughness_image_url.empty());
                }
            }
            else
            {
                assert(NULL == gltf_metallic_roughness_texture);
                assert(out_metallic_roughness_image_url.empty());
            }
        }
        else
        {
            DirectX::PackedVector::XMUBYTEN4 packed_metallic_roughness_factor;
            DirectX::XMFLOAT4 const metallic_roughness_factor(0.0F, 1.0F, 1.0F, 0.0F);
            DirectX::PackedVector::XMStoreUByteN4(&packed_metallic_roughness_factor, DirectX::XMLoadFloat4(&metallic_roughness_factor));
            out_metallic_roughness_factor = packed_metallic_roughness_factor.v;
        }
    }
}

static inline void internal_scene_depth_first_search_traverse(cgltf_data const *in_gltf_data, void (*pfn_user_callback)(cgltf_data const *in_gltf_data, cgltf_node const *in_current_gltf_node, cgltf_node const *in_parent_gltf_node, void *user_data), void *user_data)
{
    static constexpr size_t INTERNAL_NODE_INDEX_INVALID = static_cast<size_t>(~static_cast<size_t>(0U));

    struct depth_first_search_stack_element_type
    {
        size_t parent_node_index;
        size_t current_node_index;
    };

    mcrt_vector<depth_first_search_stack_element_type> depth_first_search_stack;

    mcrt_vector<bool> node_visited_flags(static_cast<size_t>(in_gltf_data->nodes_count), false);
    mcrt_vector<bool> node_pushed_flags(static_cast<size_t>(in_gltf_data->nodes_count), false);

    for (size_t scene_node_index_index = in_gltf_data->scene->nodes_count; scene_node_index_index > 0U; --scene_node_index_index)
    {
        size_t const scene_node_index = cgltf_node_index(in_gltf_data, in_gltf_data->scene->nodes[scene_node_index_index - 1U]);
        assert(!node_pushed_flags[scene_node_index]);
        node_pushed_flags[scene_node_index] = true;
        depth_first_search_stack.push_back({INTERNAL_NODE_INDEX_INVALID, scene_node_index});
    }

    while (!depth_first_search_stack.empty())
    {
        depth_first_search_stack_element_type current_stack_element = depth_first_search_stack.back();
        depth_first_search_stack.pop_back();

        assert(!node_visited_flags[current_stack_element.current_node_index]);
        node_visited_flags[current_stack_element.current_node_index] = true;

        pfn_user_callback(
            in_gltf_data,
            &in_gltf_data->nodes[current_stack_element.current_node_index],
            (INTERNAL_NODE_INDEX_INVALID != current_stack_element.parent_node_index) ? &in_gltf_data->nodes[current_stack_element.parent_node_index] : NULL,
            user_data);

        for (size_t child_node_index_index = in_gltf_data->nodes[current_stack_element.current_node_index].children_count; child_node_index_index > 0U; --child_node_index_index)
        {
            size_t const child_node_index = cgltf_node_index(in_gltf_data, in_gltf_data->nodes[current_stack_element.current_node_index].children[child_node_index_index - 1U]);
            if ((!node_visited_flags[child_node_index]) && (!node_pushed_flags[child_node_index]))
            {
                node_pushed_flags[child_node_index] = true;
                depth_first_search_stack.push_back({current_stack_element.current_node_index, child_node_index});
            }
            else
            {
                assert(false);
#ifndef NDEBUG
                std::cout << "Multiple Parents Exist for Node: " << static_cast<int>(current_stack_element.current_node_index) << std::endl;
#endif
            }
        }
    }

#ifndef NDEBUG
    for (bool node_visited_flag : node_visited_flags)
    {
        assert(node_visited_flag);
    }

    for (bool node_pushed_flag : node_pushed_flags)
    {
        assert(node_pushed_flag);
    }
#endif
}

// glTF
// RH
// Up +Y
// Forward Z
// Right -X

// VRM
// RH
// Up +Y
// Forward -Z
// Right +X

// VRMC_vrm
// RH
// Up +Y
// Forward Z
// Right -X

static inline DirectX::XMFLOAT3 internal_transform_translation(DirectX::XMFLOAT3 const &v, bool found_vrmc_vrm, bool found_vrm)
{
    if (found_vrm)
    {
        return DirectX::XMFLOAT3{-v.x, v.y, -v.z};
    }
    else
    {
        return v;
    }
}

static inline DirectX::XMFLOAT3 internal_transform_normal(DirectX::XMFLOAT3 const &v, bool found_vrmc_vrm, bool found_vrm)
{
    if (found_vrm)
    {
        return DirectX::XMFLOAT3{-v.x, v.y, -v.z};
    }
    else
    {
        return v;
    }
}

static inline DirectX::XMFLOAT4 internal_transform_tangent(DirectX::XMFLOAT4 const &v, bool found_vrmc_vrm, bool found_vrm)
{
    if (found_vrm)
    {
        // TODO: do we need to change the w component???

        return DirectX::XMFLOAT4{-v.x, v.y, -v.z, v.w};
    }
    else
    {
        return v;
    }
}

static inline DirectX::XMFLOAT4 internal_transform_rotation(DirectX::XMFLOAT4 const &q, bool found_vrmc_vrm, bool found_vrm)
{
    if (found_vrm)
    {
        DirectX::XMFLOAT4 out_q;
        {
            DirectX::XMMATRIX r = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&q));

            DirectX::XMMATRIX z = DirectX::XMMatrixRotationY(DirectX::XM_PI);

            DirectX::XMStoreFloat4(&out_q, DirectX::XMQuaternionNormalize(DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(z, r), z))));
        }

        return out_q;
    }
    else
    {
        return q;
    }
}

static cgltf_result internal_cgltf_read_file(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *file_options, const char *path, cgltf_size *size, void **in_gltf_data)
{
    void *(*const memory_alloc)(void *, cgltf_size) = memory_options->alloc_func;
    assert(NULL != memory_alloc);

    void (*const memory_free)(void *, void *) = memory_options->free_func;
    assert(NULL != memory_free);

    assert(NULL == path);

    internal_cgltf_read_file_context *const read_file_context = static_cast<internal_cgltf_read_file_context *>(file_options->user_data);
    assert(NULL != read_file_context);

    cgltf_size file_size = (NULL != size) ? (*size) : 0;

    if (0 == file_size)
    {
        file_size = read_file_context->m_data_size;
    }

    char *file_data = (char *)memory_alloc(memory_options->user_data, file_size);
    if (NULL == file_data)
    {
        assert(false);
        return cgltf_result_out_of_memory;
    }

    if ((read_file_context->m_offset + file_size) > read_file_context->m_data_size)
    {
        assert(false);
        memory_free(memory_options->user_data, file_data);
        return cgltf_result_io_error;
    }

    std::memcpy(file_data, reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(read_file_context->m_data_base) + read_file_context->m_offset), file_size);
    read_file_context->m_offset += file_size;

    if (NULL != size)
    {
        *size = file_size;
    }

    if (NULL != in_gltf_data)
    {
        *in_gltf_data = file_data;
    }

    return cgltf_result_success;
}

static void internal_cgltf_file_release(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, void *in_gltf_data, cgltf_size size)
{
    void (*const memory_free)(void *, void *) = memory_options->free_func;
    assert(NULL != memory_free);

    memory_free(memory_options->user_data, in_gltf_data);
}

static void *internal_cgltf_alloc(void *, cgltf_size size)
{
    return mcrt_malloc(size, 16U);
}

static void internal_cgltf_free(void *, void *ptr)
{
    mcrt_free(ptr);
}
