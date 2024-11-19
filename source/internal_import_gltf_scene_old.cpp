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

#include "../include/import_scene_asset.h"
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <cmath>
#include <algorithm>
#include <assert.h>
#include "../../Packed-Vector/include/brx_packed_vector.h"
#include "../../Environment-Lighting/include/brx_octahedral_mapping.h"
#include "../../McRT-Malloc/include/mcrt_unordered_map.h"
#include "../thirdparty/cgltf/cgltf.h"
#include "../thirdparty/DirectXMesh/DirectXMesh/DirectXMesh.h"
#include <iostream>
#include "../../McRT-Malloc/include/mcrt_deque.h"
#include "internal_import_gltf_scene.h"

extern cgltf_result cgltf_custom_read_file(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, const char *path, cgltf_size *size, void **data);

extern void cgltf_custom_file_release(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, void *data);

extern void *cgltf_custom_alloc(void *, cgltf_size size);

extern void cgltf_custom_free(void *, void *ptr);

static inline uint32_t sqrt_ceil(uint32_t x);

static inline void decode_morton2(uint32_t const v, uint32_t &x, uint32_t &y);

static inline void import_gltf_scene_mesh_asset(scene_mesh_data *out_mesh_data, int32_t *out_max_joint_index, cgltf_data const *data, cgltf_mesh const *mesh);

static inline void import_gltf_scene_mesh_instance_asset(mcrt_vector<cgltf_node const *> &out_mesh_instance_nodes, mcrt_vector<DirectX::XMFLOAT4X4> &out_mesh_instance_node_world_transforms, cgltf_data const *data, cgltf_mesh const *mesh);

static inline void import_gltf_scene_animation_asset(scene_animation_skeleton *out_animated_skeleton, float frame_rate, cgltf_data const *data, cgltf_skin const *skin, cgltf_animation const *animation);

static inline void internal_scene_depth_first_search_traverse(cgltf_data const *data, void (*pfn_user_callback)(cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n), void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n);

static inline void internal_scene_breadth_first_search_traverse(cgltf_data const *data, void (*pfn_user_callback)(cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n), void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n);

extern void internal_import_skeleton(cgltf_data const *data, mcrt_vector<DirectX::XMFLOAT4X4> &out_internal_node_world_transforms, mcrt_vector<mcrt_vector<cgltf_node const *>> &out_internal_mesh_instance_nodes, mcrt_vector<uint32_t> &out_internal_node_index_to_skeleton_joint_index, mcrt_vector<mcrt_string> &out_skeleton_joint_names, mcrt_vector<uint32_t> &out_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_skeleton_animation_joint_transform> &out_inverse_bind_pose_skeleton_joint_transforms, uint32_t *out_vrm_skeleton_joint_names);

mcrt_vector<uint32_t> internal_node_index_to_skeleton_joint_index;

extern bool import_gltf_scene_asset(mcrt_vector<scene_mesh_data> &out_total_mesh_data, float frame_rate, brx_asset_import_input_stream_factory *input_stream_factory, char const *path)
{
    // TODO: merge primitives with the same material from different meshes (consider multiple instances)

    cgltf_data *data = NULL;
    {
        cgltf_options options = {};
        options.memory.alloc_func = cgltf_custom_alloc;
        options.memory.free_func = cgltf_custom_free;
        options.file.read = cgltf_custom_read_file;
        options.file.release = cgltf_custom_file_release;
        options.file.user_data = input_stream_factory;

        cgltf_result result_parse_file = cgltf_parse_file(&options, path, &data);
        if (cgltf_result_success != result_parse_file)
        {
            return false;
        }

        cgltf_result result_load_buffers = cgltf_load_buffers(&options, data, path);
        if (cgltf_result_success != result_load_buffers)
        {
            cgltf_free(data);
            return false;
        }
    }

    mcrt_vector<DirectX::XMFLOAT4X4> internal_node_world_transforms;
    mcrt_vector<mcrt_vector<cgltf_node const *>> internal_mesh_instance_nodes;
    mcrt_vector<mcrt_string> skeleton_joint_names;
    mcrt_vector<uint32_t> skeleton_joint_parent_indices;
    mcrt_vector<brx_asset_import_skeleton_animation_joint_transform> inverse_bind_pose_skeleton_joint_transforms;
    uint32_t vrm_skeleton_joint_indices[BRX_ASSET_IMPORT_VRM_SKELETON_JOINT_NAME_COUNT];
    mcrt_vector<uint32_t> internal_node_index_to_skeleton_joint_index;
    internal_import_skeleton(data, internal_node_world_transforms, internal_mesh_instance_nodes, internal_node_index_to_skeleton_joint_index, skeleton_joint_names, skeleton_joint_parent_indices, inverse_bind_pose_skeleton_joint_transforms, &vrm_skeleton_joint_indices[0]);

    out_total_mesh_data.resize(1);
    size_t mesh_index = 0;

    int32_t max_joint_index;
    import_gltf_scene_mesh_asset(&out_total_mesh_data[mesh_index], &max_joint_index, data, &data->meshes[mesh_index]);

    mcrt_vector<cgltf_node const *> mesh_instance_nodes;
    mcrt_vector<DirectX::XMFLOAT4X4> mesh_instance_node_world_transforms;
    import_gltf_scene_mesh_instance_asset(mesh_instance_nodes, mesh_instance_node_world_transforms, data, &data->meshes[mesh_index]);

    out_total_mesh_data[mesh_index].m_instances.resize(1U);
    size_t mesh_instance_index = 0;

    cgltf_animation const *animation = ((data->animations_count > 0) ? (&data->animations[0]) : NULL);

#ifndef NDEBUG
    // glTF Validator
    // NODE_SKINNED_MESH_NON_ROOT
    // NODE_SKINNED_MESH_LOCAL_TRANSFORMS
    DirectX::XMVECTOR out_instance_node_world_scale;
    DirectX::XMVECTOR out_instance_node_world_rotation;
    DirectX::XMVECTOR out_instance_node_world_translation;
    DirectX::XMMatrixDecompose(&out_instance_node_world_scale, &out_instance_node_world_rotation, &out_instance_node_world_translation, DirectX::XMLoadFloat4x4(&mesh_instance_node_world_transforms[mesh_instance_index]));

    // FLT_EPSILON
    constexpr float const scale_epsilon = 1E-5F;
    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(scale_epsilon))));
    constexpr float const rotation_epsilon = 1E-5F;
    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_rotation, DirectX::XMQuaternionIdentity())), DirectX::XMVectorReplicate(rotation_epsilon))));
    constexpr float const translation_epsilon = 1E-5F;
    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_translation, DirectX::XMVectorZero())), DirectX::XMVectorReplicate(translation_epsilon))));
#endif
    DirectX::XMStoreFloat4x4(&out_total_mesh_data[mesh_index].m_instances[mesh_instance_index].m_model_transform, DirectX::XMMatrixIdentity());

    import_gltf_scene_animation_asset(&out_total_mesh_data[mesh_index].m_instances[mesh_instance_index].m_animation_skeleton, frame_rate, data, mesh_instance_nodes[0]->skin, animation);

#if 0
    out_total_mesh_data.resize(data->meshes_count);

    for (size_t mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index)
    {
        scene_mesh_data &out_mesh_data = out_total_mesh_data[mesh_index];
        int32_t max_joint_index;
        import_gltf_scene_mesh_asset(&out_mesh_data, &max_joint_index, data, &data->meshes[mesh_index]);

        mcrt_vector<cgltf_node const *> mesh_instance_nodes;
        mcrt_vector<DirectX::XMFLOAT4X4> mesh_instance_node_world_transforms;
        import_gltf_scene_mesh_instance_asset(mesh_instance_nodes, mesh_instance_node_world_transforms, data, &data->meshes[mesh_index]);

        size_t const mesh_instance_count = mesh_instance_nodes.size();
        assert(mesh_instance_node_world_transforms.size() == mesh_instance_count);

        mcrt_vector<scene_mesh_instance_data> &out_mesh_instance_data = out_mesh_data.m_instances;
        if (!out_mesh_data.m_skinned)
        {
            assert(max_joint_index < 0);

            out_mesh_instance_data.resize(mesh_instance_count);

            for (size_t mesh_instance_index = 0; mesh_instance_index < mesh_instance_count; ++mesh_instance_index)
            {
                assert(NULL == mesh_instance_nodes[mesh_instance_index]->skin);
                out_mesh_instance_data[mesh_instance_index].m_model_transform = mesh_instance_node_world_transforms[mesh_instance_index];
            }
        }
        else
        {
            assert(max_joint_index >= 0);

            out_mesh_instance_data.resize(mesh_instance_count);

            for (size_t mesh_instance_index = 0; mesh_instance_index < mesh_instance_count; ++mesh_instance_index)
            {
                cgltf_skin const *skin = mesh_instance_nodes[mesh_instance_index]->skin;
                if (NULL != skin)
                {
                    // TODO: support multiple animations
                    cgltf_animation const *animation = ((data->animations_count > 0) ? (&data->animations[0]) : NULL);

#ifndef NDEBUG
                    // glTF Validator
                    // NODE_SKINNED_MESH_NON_ROOT
                    // NODE_SKINNED_MESH_LOCAL_TRANSFORMS
                    DirectX::XMVECTOR out_instance_node_world_scale;
                    DirectX::XMVECTOR out_instance_node_world_rotation;
                    DirectX::XMVECTOR out_instance_node_world_translation;
                    DirectX::XMMatrixDecompose(&out_instance_node_world_scale, &out_instance_node_world_rotation, &out_instance_node_world_translation, DirectX::XMLoadFloat4x4(&mesh_instance_node_world_transforms[mesh_instance_index]));

                    // FLT_EPSILON
                    constexpr float const scale_epsilon = 1E-5F;
                    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(scale_epsilon))));
                    constexpr float const rotation_epsilon = 1E-5F;
                    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_rotation, DirectX::XMQuaternionIdentity())), DirectX::XMVectorReplicate(rotation_epsilon))));
                    constexpr float const translation_epsilon = 1E-5F;
                    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_translation, DirectX::XMVectorZero())), DirectX::XMVectorReplicate(translation_epsilon))));
#endif
                    DirectX::XMStoreFloat4x4(&out_mesh_instance_data[mesh_instance_index].m_model_transform, DirectX::XMMatrixIdentity());

                    import_gltf_scene_animation_asset(&out_mesh_instance_data[mesh_instance_index].m_animation_skeleton, frame_rate, data, skin, animation);
                }
                else
                {
                    // TODO: This should NOT happen
                    assert(0);

                    out_mesh_instance_data[mesh_instance_index].m_model_transform = mesh_instance_node_world_transforms[mesh_instance_index];

                    out_mesh_instance_data[mesh_instance_index].m_animation_skeleton.init(1, (max_joint_index + 1));

                    DirectX::XMFLOAT4 quaternion;
                    DirectX::XMFLOAT3 translation;
                    DirectX::XMStoreFloat4(&quaternion, DirectX::XMQuaternionIdentity());
                    DirectX::XMStoreFloat3(&translation, DirectX::XMVectorZero());

                    for (int32_t joint_index = 0; joint_index < (max_joint_index + 1); ++joint_index)
                    {
                        out_mesh_instance_data[mesh_instance_index].m_animation_skeleton.set_transform(0, joint_index, quaternion, translation);
                    }
                }
            }
        }
    }
#endif

    cgltf_free(data);

    return true;
}

static void import_gltf_scene_mesh_asset(scene_mesh_data *out_mesh_data, int32_t *out_max_joint_index, cgltf_data const *data, cgltf_mesh const *mesh)
{
    (*out_max_joint_index) = -1;

    assert(1U == data->skins_count);

    mcrt_unordered_map<size_t, size_t> subset_material_indices;
    subset_material_indices.reserve(mesh->primitives_count);

    out_mesh_data->m_subsets.reserve(mesh->primitives_count);

    for (size_t primitive_index = 0; primitive_index < mesh->primitives_count; ++primitive_index)
    {
        cgltf_primitive const *primitive = &mesh->primitives[primitive_index];

        if (cgltf_primitive_type_triangles == primitive->type)
        {
            cgltf_accessor const *position_accessor = NULL;
            cgltf_accessor const *normal_accessor = NULL;
            cgltf_accessor const *tangent_accessor = NULL;
            cgltf_accessor const *texcoord_accessor = NULL;
            cgltf_accessor const *joints_accessor = NULL;
            cgltf_accessor const *weights_accessor = NULL;
            {
                cgltf_int position_index = -1;
                cgltf_int normal_index = -1;
                cgltf_int tangent_index = -1;
                cgltf_int texcoord_index = -1;
                cgltf_int joints_index = -1;
                cgltf_int weights_index = -1;

                for (size_t vertex_attribute_index = 0; vertex_attribute_index < primitive->attributes_count; ++vertex_attribute_index)
                {
                    cgltf_attribute const *vertex_attribute = &primitive->attributes[vertex_attribute_index];

                    switch (vertex_attribute->type)
                    {
                    case cgltf_attribute_type_position:
                    {
                        assert(cgltf_attribute_type_position == vertex_attribute->type);

                        if (NULL == position_accessor || vertex_attribute->index < position_index)
                        {
                            position_accessor = vertex_attribute->data;
                            position_index = vertex_attribute->index;
                        }
                    }
                    break;
                    case cgltf_attribute_type_normal:
                    {
                        assert(cgltf_attribute_type_normal == vertex_attribute->type);

                        if (NULL == normal_accessor || vertex_attribute->index < normal_index)
                        {
                            normal_accessor = vertex_attribute->data;
                            normal_index = vertex_attribute->index;
                        }
                    }
                    break;
                    case cgltf_attribute_type_tangent:
                    {
                        assert(cgltf_attribute_type_tangent == vertex_attribute->type);

                        if (NULL == tangent_accessor || vertex_attribute->index < tangent_index)
                        {
                            tangent_accessor = vertex_attribute->data;
                            tangent_index = vertex_attribute->index;
                        }
                    }
                    break;
                    case cgltf_attribute_type_texcoord:
                    {
                        assert(cgltf_attribute_type_texcoord == vertex_attribute->type);

                        if (NULL == texcoord_accessor || vertex_attribute->index < texcoord_index)
                        {
                            texcoord_accessor = vertex_attribute->data;
                            texcoord_index = vertex_attribute->index;
                        }
                    }
                    break;
                    case cgltf_attribute_type_joints:
                    {
                        assert(cgltf_attribute_type_joints == vertex_attribute->type);

                        if (NULL == joints_accessor || vertex_attribute->index < joints_index)
                        {
                            joints_accessor = vertex_attribute->data;
                            joints_index = vertex_attribute->index;
                        }
                    }
                    break;
                    case cgltf_attribute_type_weights:
                    {
                        assert(cgltf_attribute_type_weights == vertex_attribute->type);

                        if (NULL == weights_accessor || vertex_attribute->index < weights_index)
                        {
                            weights_accessor = vertex_attribute->data;
                            weights_index = vertex_attribute->index;
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

            if (NULL != position_accessor)
            {
                cgltf_accessor const *const index_accessor = primitive->indices;

                size_t const vertex_count = position_accessor->count;
                size_t const index_count = (NULL != index_accessor) ? index_accessor->count : vertex_count;

                uint32_t raw_max_index = 0U;
                mcrt_vector<uint32_t> raw_indices(index_count);
                mcrt_vector<DirectX::XMFLOAT3> raw_positions(vertex_count);
                mcrt_vector<DirectX::XMFLOAT3> raw_normals(vertex_count);
                mcrt_vector<DirectX::XMFLOAT2> raw_texcoords(vertex_count);
                mcrt_vector<DirectX::XMFLOAT4> raw_tangents(vertex_count);
                {
                    // TODO: support strip and fan
                    assert(0U == (index_count % 3U));
                    size_t const face_count = index_count / 3U;

                    if (NULL != index_accessor)
                    {
                        uintptr_t index_base = -1;
                        size_t index_stride = -1;
                        {
                            cgltf_buffer_view const *const index_buffer_view = index_accessor->buffer_view;
                            index_base = reinterpret_cast<uintptr_t>(index_buffer_view->buffer->data) + index_buffer_view->offset + index_accessor->offset;
                            index_stride = (0 != index_buffer_view->stride) ? index_buffer_view->stride : index_accessor->stride;
                        }

                        assert(cgltf_type_scalar == index_accessor->type);

                        switch (index_accessor->component_type)
                        {
                        case cgltf_component_type_r_8u:
                        {
                            assert(cgltf_component_type_r_8u == index_accessor->component_type);

                            for (size_t index_index = 0; index_index < index_accessor->count; ++index_index)
                            {
                                uint8_t const *const index_ubyte = reinterpret_cast<uint8_t const *>(index_base + index_stride * index_index);

                                uint32_t const raw_index = static_cast<uint32_t>(*index_ubyte);

                                raw_max_index = std::max(raw_max_index, raw_index);

                                raw_indices[index_index] = raw_index;
                            }
                        }
                        break;
                        case cgltf_component_type_r_16u:
                        {
                            assert(cgltf_component_type_r_16u == index_accessor->component_type);

                            for (size_t index_index = 0; index_index < index_accessor->count; ++index_index)
                            {
                                uint16_t const *const index_ushort = reinterpret_cast<uint16_t const *>(index_base + index_stride * index_index);

                                uint32_t const raw_index = static_cast<uint32_t>(*index_ushort);

                                raw_max_index = std::max(raw_max_index, raw_index);

                                raw_indices[index_index] = raw_index;
                            }
                        }
                        break;
                        case cgltf_component_type_r_32u:
                        {
                            assert(cgltf_component_type_r_32u == index_accessor->component_type);

                            for (size_t index_index = 0; index_index < index_accessor->count; ++index_index)
                            {
                                uint32_t const *const index_uint = reinterpret_cast<uint32_t const *>(index_base + index_stride * index_index);

                                uint32_t const raw_index = (*index_uint);

                                raw_max_index = std::max(raw_max_index, raw_index);

                                raw_indices[index_index] = raw_index;
                            }
                        }
                        break;
                        default:
                            assert(0);
                        }
                    }
                    else
                    {
                        for (size_t index_index = 0; index_index < index_count; ++index_index)
                        {
                            uint32_t const raw_index = static_cast<uint32_t>(index_index);

                            raw_max_index = std::max(raw_max_index, raw_index);

                            raw_indices[index_index] = raw_index;
                        }
                    }

                    assert(NULL != position_accessor);
                    {
                        uintptr_t position_base = -1;
                        size_t position_stride = -1;
                        {
                            cgltf_buffer_view const *const position_buffer_view = position_accessor->buffer_view;
                            position_base = reinterpret_cast<uintptr_t>(position_buffer_view->buffer->data) + position_buffer_view->offset + position_accessor->offset;
                            position_stride = (0 != position_buffer_view->stride) ? position_buffer_view->stride : position_accessor->stride;
                        }

                        assert(cgltf_type_vec3 == position_accessor->type);
                        assert(cgltf_component_type_r_32f == position_accessor->component_type);

                        for (size_t vertex_index = 0; vertex_index < position_accessor->count; ++vertex_index)
                        {
                            float const *const position_float3 = reinterpret_cast<float const *>(position_base + position_stride * vertex_index);

                            raw_positions[vertex_index] = DirectX::XMFLOAT3(position_float3[0], position_float3[1], position_float3[2]);
                        }
                    }

                    if (NULL != normal_accessor)
                    {
                        uintptr_t normal_base = -1;
                        size_t normal_stride = -1;
                        {
                            cgltf_buffer_view const *const normal_buffer_view = normal_accessor->buffer_view;
                            normal_base = reinterpret_cast<uintptr_t>(normal_buffer_view->buffer->data) + normal_buffer_view->offset + normal_accessor->offset;
                            normal_stride = (0 != normal_buffer_view->stride) ? normal_buffer_view->stride : normal_accessor->stride;
                        }

                        assert(normal_accessor->count == vertex_count);

                        assert(cgltf_type_vec3 == normal_accessor->type);
                        assert(cgltf_component_type_r_32f == normal_accessor->component_type);

                        for (size_t vertex_index = 0; vertex_index < normal_accessor->count; ++vertex_index)
                        {
                            float const *const normal_float3 = reinterpret_cast<float const *>(normal_base + normal_stride * vertex_index);

                            raw_normals[vertex_index] = DirectX::XMFLOAT3(normal_float3[0], normal_float3[1], normal_float3[2]);
                        }
                    }
                    else
                    {
                        DirectX::ComputeNormals(raw_indices.data(), face_count, raw_positions.data(), vertex_count, DirectX::CNORM_DEFAULT, raw_normals.data());
                    }

                    if (NULL != texcoord_accessor)
                    {
                        uintptr_t texcoord_base = -1;
                        size_t texcoord_stride = -1;
                        {
                            cgltf_buffer_view const *const texcoord_buffer_view = texcoord_accessor->buffer_view;
                            texcoord_base = reinterpret_cast<uintptr_t>(texcoord_buffer_view->buffer->data) + texcoord_buffer_view->offset + texcoord_accessor->offset;
                            texcoord_stride = (0 != texcoord_buffer_view->stride) ? texcoord_buffer_view->stride : texcoord_accessor->stride;
                        }

                        assert(texcoord_accessor->count == vertex_count);

                        assert(cgltf_type_vec2 == texcoord_accessor->type);

                        switch (texcoord_accessor->component_type)
                        {
                        case cgltf_component_type_r_8u:
                        {
                            assert(cgltf_component_type_r_8u == texcoord_accessor->component_type);

                            for (size_t vertex_index = 0; vertex_index < texcoord_accessor->count; ++vertex_index)
                            {
                                uint8_t const *const texcoord_ubyte2 = reinterpret_cast<uint8_t const *>(texcoord_base + texcoord_stride * vertex_index);

                                DirectX::PackedVector::XMUBYTEN2 packed_vector_ubyten2(texcoord_ubyte2[0], texcoord_ubyte2[1]);

                                DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN2(&packed_vector_ubyten2);

                                DirectX::XMStoreFloat2(&raw_texcoords[vertex_index], unpacked_vector);
                            }
                        }
                        break;
                        case cgltf_component_type_r_16u:
                        {
                            assert(cgltf_component_type_r_16u == texcoord_accessor->component_type);

                            for (size_t vertex_index = 0; vertex_index < texcoord_accessor->count; ++vertex_index)
                            {
                                uint16_t const *const texcoord_ushortn2 = reinterpret_cast<uint16_t const *>(texcoord_base + texcoord_stride * vertex_index);

                                DirectX::PackedVector::XMUSHORTN2 packed_vector_ushortn2(texcoord_ushortn2[0], texcoord_ushortn2[1]);

                                DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN2(&packed_vector_ushortn2);

                                DirectX::XMStoreFloat2(&raw_texcoords[vertex_index], unpacked_vector);
                            }
                        }
                        break;
                        case cgltf_component_type_r_32f:
                        {
                            assert(cgltf_component_type_r_32f == texcoord_accessor->component_type);

                            for (size_t vertex_index = 0; vertex_index < texcoord_accessor->count; ++vertex_index)
                            {
                                float const *const texcoord_float2 = reinterpret_cast<float const *>(texcoord_base + texcoord_stride * vertex_index);

                                raw_texcoords[vertex_index] = DirectX::XMFLOAT2(texcoord_float2[0], texcoord_float2[1]);
                            }
                        }
                        break;
                        default:
                            assert(0);
                        }
                    }
                    else
                    {
                        // TODO: uv may overlap since we merge different primitives based on the material
                        assert(vertex_count <= static_cast<size_t>(UINT32_MAX));
                        uint32_t sqrt_ceil_vertex_count = sqrt_ceil(static_cast<uint32_t>(vertex_count));

                        for (size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
                        {
                            assert(vertex_index <= static_cast<size_t>(UINT32_MAX));

                            uint32_t x;
                            uint32_t y;
                            decode_morton2(static_cast<uint32_t>(vertex_index), x, y);

                            assert(x <= sqrt_ceil_vertex_count);
                            assert(y <= sqrt_ceil_vertex_count);
                            raw_texcoords[vertex_index] = DirectX::XMFLOAT2(static_cast<float>(x) / static_cast<float>(sqrt_ceil_vertex_count), static_cast<float>(y) / static_cast<float>(sqrt_ceil_vertex_count));
                        }
                    }

                    if (NULL != tangent_accessor)
                    {
                        uintptr_t tangent_base = -1;
                        size_t tangent_stride = -1;
                        {
                            cgltf_buffer_view const *const tangent_buffer_view = tangent_accessor->buffer_view;
                            tangent_base = reinterpret_cast<uintptr_t>(tangent_buffer_view->buffer->data) + tangent_buffer_view->offset + tangent_accessor->offset;
                            tangent_stride = (0 != tangent_buffer_view->stride) ? tangent_buffer_view->stride : tangent_accessor->stride;
                        }

                        assert(tangent_accessor->count == vertex_count);

                        assert(cgltf_type_vec4 == tangent_accessor->type);
                        assert(cgltf_component_type_r_32f == tangent_accessor->component_type);

                        for (size_t vertex_index = 0; vertex_index < tangent_accessor->count; ++vertex_index)
                        {
                            float const *const tangent_float4 = reinterpret_cast<float const *>(tangent_base + tangent_stride * vertex_index);

                            raw_tangents[vertex_index] = DirectX::XMFLOAT4(tangent_float4[0], tangent_float4[1], tangent_float4[2], tangent_float4[3]);
                        }
                    }
                    else
                    {
                        DirectX::ComputeTangentFrame(raw_indices.data(), face_count, raw_positions.data(), raw_normals.data(), raw_texcoords.data(), vertex_count, raw_tangents.data());
                    }
                }

                uint16_t raw_max_joint_index = 0U;
                mcrt_vector<DirectX::PackedVector::XMUSHORT4> raw_joint_indices((NULL != joints_accessor && NULL != weights_accessor) ? vertex_count : static_cast<size_t>(0U));
                mcrt_vector<DirectX::XMFLOAT4> raw_joint_weights((NULL != joints_accessor && NULL != weights_accessor) ? vertex_count : static_cast<size_t>(0U));
                {
                    if (NULL != joints_accessor && NULL != weights_accessor)
                    {
                        // Joint Indices
                        {
                            uintptr_t joints_base = -1;
                            size_t joints_stride = -1;
                            {
                                cgltf_buffer_view const *const joint_indices_buffer_view = joints_accessor->buffer_view;
                                joints_base = reinterpret_cast<uintptr_t>(joint_indices_buffer_view->buffer->data) + joint_indices_buffer_view->offset + joints_accessor->offset;
                                joints_stride = (0 != joint_indices_buffer_view->stride) ? joint_indices_buffer_view->stride : joints_accessor->stride;
                            }

                            assert(joints_accessor->count == vertex_count);

                            assert(cgltf_type_vec4 == joints_accessor->type);

                            switch (joints_accessor->component_type)
                            {
                            case cgltf_component_type_r_8u:
                            {
                                assert(cgltf_component_type_r_8u == joints_accessor->component_type);

                                for (size_t vertex_index = 0; vertex_index < joints_accessor->count; ++vertex_index)
                                {
                                    uint8_t const *const joint_indices_ubyte4 = reinterpret_cast<uint8_t const *>(joints_base + joints_stride * vertex_index);

                                    uint16_t raw_joint_index_x = static_cast<uint16_t>(joint_indices_ubyte4[0]);
                                    uint16_t raw_joint_index_y = static_cast<uint16_t>(joint_indices_ubyte4[1]);
                                    uint16_t raw_joint_index_z = static_cast<uint16_t>(joint_indices_ubyte4[2]);
                                    uint16_t raw_joint_index_w = static_cast<uint16_t>(joint_indices_ubyte4[3]);

                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_x);
                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_y);
                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_z);
                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_w);

                                    raw_joint_indices[vertex_index] = DirectX::PackedVector::XMUSHORT4(raw_joint_index_x, raw_joint_index_y, raw_joint_index_z, raw_joint_index_w);
                                }
                            }
                            break;
                            case cgltf_component_type_r_16u:
                            {
                                assert(cgltf_component_type_r_16u == joints_accessor->component_type);

                                for (size_t vertex_index = 0; vertex_index < joints_accessor->count; ++vertex_index)
                                {
                                    uint16_t const *const joint_indices_ushort4 = reinterpret_cast<uint16_t const *>(joints_base + joints_stride * vertex_index);

                                    uint16_t raw_joint_index_x = joint_indices_ushort4[0];
                                    uint16_t raw_joint_index_y = joint_indices_ushort4[1];
                                    uint16_t raw_joint_index_z = joint_indices_ushort4[2];
                                    uint16_t raw_joint_index_w = joint_indices_ushort4[3];

                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_x);
                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_y);
                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_z);
                                    raw_max_joint_index = std::max(raw_max_joint_index, raw_joint_index_w);

                                    raw_joint_indices[vertex_index] = DirectX::PackedVector::XMUSHORT4(raw_joint_index_x, raw_joint_index_y, raw_joint_index_z, raw_joint_index_w);
                                }
                            }
                            break;
                            default:
                                assert(0);
                            }
                        }

                        // Joint Weights
                        {
                            uintptr_t weights_base = -1;
                            size_t weights_stride = -1;
                            {
                                cgltf_buffer_view const *const joint_weights_buffer_view = weights_accessor->buffer_view;
                                weights_base = reinterpret_cast<uintptr_t>(joint_weights_buffer_view->buffer->data) + joint_weights_buffer_view->offset + weights_accessor->offset;
                                weights_stride = (0 != joint_weights_buffer_view->stride) ? joint_weights_buffer_view->stride : weights_accessor->stride;
                            }

                            assert(weights_accessor->count == vertex_count);

                            assert(cgltf_type_vec4 == weights_accessor->type);

                            switch (weights_accessor->component_type)
                            {
                            case cgltf_component_type_r_8u:
                            {
                                assert(cgltf_component_type_r_8u == weights_accessor->component_type);

                                for (size_t vertex_index = 0; vertex_index < weights_accessor->count; ++vertex_index)
                                {
                                    uint8_t const *const joint_weights_ubyte4 = reinterpret_cast<uint8_t const *>(weights_base + weights_stride * vertex_index);

                                    DirectX::PackedVector::XMUBYTEN4 packed_vector_ubyten4(joint_weights_ubyte4[0], joint_weights_ubyte4[1], joint_weights_ubyte4[2], joint_weights_ubyte4[3]);

                                    DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUByteN4(&packed_vector_ubyten4);

                                    DirectX::XMStoreFloat4(&raw_joint_weights[vertex_index], unpacked_vector);
                                }
                            }
                            break;
                            case cgltf_component_type_r_16u:
                            {
                                assert(cgltf_component_type_r_16u == weights_accessor->component_type);

                                for (size_t vertex_index = 0; vertex_index < weights_accessor->count; ++vertex_index)
                                {
                                    uint16_t const *const joint_weights_ushortn4 = reinterpret_cast<uint16_t const *>(weights_base + weights_stride * vertex_index);

                                    DirectX::PackedVector::XMUSHORTN4 packed_vector_ushortn4(joint_weights_ushortn4[0], joint_weights_ushortn4[1], joint_weights_ushortn4[2], joint_weights_ushortn4[3]);

                                    DirectX::XMVECTOR unpacked_vector = DirectX::PackedVector::XMLoadUShortN4(&packed_vector_ushortn4);

                                    DirectX::XMStoreFloat4(&raw_joint_weights[vertex_index], unpacked_vector);
                                }
                            }
                            break;
                            case cgltf_component_type_r_32f:
                            {
                                assert(cgltf_component_type_r_32f == weights_accessor->component_type);

                                for (size_t vertex_index = 0; vertex_index < weights_accessor->count; ++vertex_index)
                                {
                                    float const *const joint_weights_float4 = reinterpret_cast<float const *>(weights_base + weights_stride * vertex_index);

                                    raw_joint_weights[vertex_index] = DirectX::XMFLOAT4(joint_weights_float4[0], joint_weights_float4[1], joint_weights_float4[2], joint_weights_float4[3]);
                                }
                            }
                            break;
                            default:
                                assert(0);
                            }
                        }
                    }
                }

                // We merge the primitives, of which the material is the same, within the same mesh
                // TODO: shall we merge the primitives within different meshes (consider multiple instances)
                scene_mesh_subset_data *out_subset_data;
                uint32_t out_subset_vertex_index_offset;
                uint32_t out_subset_index_index_offset;
                {
                    cgltf_material const *const primitive_material = primitive->material;

                    size_t const primitive_material_index = cgltf_material_index(data, primitive_material);

                    auto found = subset_material_indices.find(primitive_material_index);
                    if (subset_material_indices.end() != found)
                    {
                        size_t const subset_data_index = found->second;
                        assert(subset_data_index < out_mesh_data->m_subsets.size());
                        out_subset_data = &out_mesh_data->m_subsets[subset_data_index];
                        out_subset_vertex_index_offset = static_cast<uint32_t>(out_subset_data->m_vertex_position_binding.size());
                        out_subset_index_index_offset = static_cast<uint32_t>(out_subset_data->m_indices.size());
                    }
                    else
                    {
                        size_t const subset_data_index = out_mesh_data->m_subsets.size();
                        out_mesh_data->m_subsets.push_back({});
                        subset_material_indices.emplace_hint(found, primitive_material_index, subset_data_index);
                        out_subset_data = &out_mesh_data->m_subsets.back();
                        out_subset_data->m_max_index = 0U;
                        out_subset_vertex_index_offset = 0U;
                        out_subset_index_index_offset = 0U;

                        if (NULL != primitive_material->normal_texture.texture)
                        {
                            cgltf_image const *const normal_texture_image = primitive_material->normal_texture.texture->image;
                            assert(NULL != normal_texture_image);
                            assert(NULL == normal_texture_image->buffer_view);
                            assert(NULL != normal_texture_image->uri);

                            out_subset_data->m_normal_texture_image_uri = normal_texture_image->uri;
                            cgltf_decode_uri(&out_subset_data->m_normal_texture_image_uri[0]);
                            size_t null_terminator_pos = out_subset_data->m_normal_texture_image_uri.find('\0');
                            if (std::string::npos != null_terminator_pos)
                            {
                                out_subset_data->m_normal_texture_image_uri.resize(null_terminator_pos);
                            }

                            out_subset_data->m_normal_texture_scale = primitive_material->normal_texture.scale;
                        }
                        else
                        {
                            out_subset_data->m_normal_texture_scale = 1.0;
                        }

                        if (primitive_material->has_emissive_strength)
                        {
                            out_subset_data->m_emissive_factor = DirectX::XMFLOAT3(primitive_material->emissive_factor[0] * primitive_material->emissive_strength.emissive_strength, primitive_material->emissive_factor[1] * primitive_material->emissive_strength.emissive_strength, primitive_material->emissive_factor[2] * primitive_material->emissive_strength.emissive_strength);
                        }
                        else
                        {
                            out_subset_data->m_emissive_factor = DirectX::XMFLOAT3(primitive_material->emissive_factor[0], primitive_material->emissive_factor[1], primitive_material->emissive_factor[2]);
                        }

                        if (NULL != primitive_material->emissive_texture.texture)
                        {
                            cgltf_image const *const emissive_texture_image = primitive_material->emissive_texture.texture->image;
                            assert(NULL != emissive_texture_image);
                            assert(NULL == emissive_texture_image->buffer_view);
                            assert(NULL != emissive_texture_image->uri);

                            out_subset_data->m_emissive_texture_image_uri = emissive_texture_image->uri;
                            cgltf_decode_uri(&out_subset_data->m_emissive_texture_image_uri[0]);
                            size_t null_terminator_pos = out_subset_data->m_emissive_texture_image_uri.find('\0');
                            if (std::string::npos != null_terminator_pos)
                            {
                                out_subset_data->m_emissive_texture_image_uri.resize(null_terminator_pos);
                            }
                        }

                        if (primitive_material->has_pbr_metallic_roughness)
                        {
                            out_subset_data->m_base_color_factor = DirectX::XMFLOAT3(primitive_material->pbr_metallic_roughness.base_color_factor[0], primitive_material->pbr_metallic_roughness.base_color_factor[1], primitive_material->pbr_metallic_roughness.base_color_factor[2]);

                            if (NULL != primitive_material->pbr_metallic_roughness.base_color_texture.texture)
                            {
                                cgltf_image const *const base_color_texture_image = primitive_material->pbr_metallic_roughness.base_color_texture.texture->image;
                                assert(NULL != base_color_texture_image);
                                assert(NULL == base_color_texture_image->buffer_view);
                                assert(NULL != base_color_texture_image->uri);

                                out_subset_data->m_base_color_texture_image_uri = base_color_texture_image->uri;
                                cgltf_decode_uri(&out_subset_data->m_base_color_texture_image_uri[0]);
                                size_t null_terminator_pos = out_subset_data->m_base_color_texture_image_uri.find('\0');
                                if (std::string::npos != null_terminator_pos)
                                {
                                    out_subset_data->m_base_color_texture_image_uri.resize(null_terminator_pos);
                                }
                            }

                            out_subset_data->m_metallic_factor = primitive_material->pbr_metallic_roughness.metallic_factor;

                            out_subset_data->m_roughness_factor = primitive_material->pbr_metallic_roughness.roughness_factor;

                            if (NULL != primitive_material->pbr_metallic_roughness.metallic_roughness_texture.texture)
                            {
                                cgltf_image const *const metallic_roughness_texture_image = primitive_material->pbr_metallic_roughness.metallic_roughness_texture.texture->image;
                                assert(NULL != metallic_roughness_texture_image);
                                assert(NULL == metallic_roughness_texture_image->buffer_view);
                                assert(NULL != metallic_roughness_texture_image->uri);

                                out_subset_data->m_metallic_roughness_texture_image_uri = metallic_roughness_texture_image->uri;
                                cgltf_decode_uri(&out_subset_data->m_metallic_roughness_texture_image_uri[0]);
                                size_t null_terminator_pos = out_subset_data->m_metallic_roughness_texture_image_uri.find('\0');
                                if (std::string::npos != null_terminator_pos)
                                {
                                    out_subset_data->m_metallic_roughness_texture_image_uri.resize(null_terminator_pos);
                                }
                            }
                        }
                    }
                }

                // Indices
                {
                    // The vertex buffer for each primitive is independent
                    // Index for each primitive is from zero
                    out_subset_data->m_max_index = (out_subset_vertex_index_offset + raw_max_index);

                    assert(out_subset_index_index_offset == out_subset_data->m_indices.size());
                    out_subset_data->m_indices.resize(out_subset_index_index_offset + index_count);

                    uint32_t *const out_indices = &out_subset_data->m_indices[out_subset_index_index_offset];

                    for (size_t index_index = 0; index_index < index_count; ++index_index)
                    {
                        out_indices[index_index] = (out_subset_vertex_index_offset + raw_indices[index_index]);
                    }
                }

                // Vertex Position Binding
                {
                    assert(out_subset_vertex_index_offset == out_subset_data->m_vertex_position_binding.size());
                    out_subset_data->m_vertex_position_binding.resize(out_subset_vertex_index_offset + vertex_count);
                    scene_mesh_vertex_position_binding *const out_vertex_position_binding = &out_subset_data->m_vertex_position_binding[out_subset_vertex_index_offset];

                    for (size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
                    {
                        out_vertex_position_binding[vertex_index].m_position = raw_positions[vertex_index];
                    }
                }

                // Vertex Varying Binding
                {
                    assert(out_subset_vertex_index_offset == out_subset_data->m_vertex_varying_binding.size());
                    out_subset_data->m_vertex_varying_binding.resize(out_subset_vertex_index_offset + vertex_count);
                    scene_mesh_vertex_varying_binding *const out_vertex_varying_binding = &out_subset_data->m_vertex_varying_binding[out_subset_vertex_index_offset];

                    for (size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
                    {
                        DirectX::XMFLOAT2 const mapped_normal = brx_octahedral_map(raw_normals[vertex_index]);

                        DirectX::PackedVector::XMSHORTN2 packed_normal;
                        DirectX::PackedVector::XMStoreShortN2(&packed_normal, DirectX::XMLoadFloat2(&mapped_normal));

                        out_vertex_varying_binding[vertex_index].m_normal = packed_normal.v;

                        DirectX::XMFLOAT3 tangent_xyz(raw_tangents[vertex_index].x, raw_tangents[vertex_index].y, raw_tangents[vertex_index].z);
                        float tangent_w = raw_tangents[vertex_index].w;

                        out_vertex_varying_binding[vertex_index].m_tangent = brx_FLOAT3_to_R15G15B2_SNORM(brx_octahedral_map(tangent_xyz), tangent_w);

                        DirectX::PackedVector::XMUSHORTN2 packed_texcoord;
                        DirectX::PackedVector::XMStoreUShortN2(&packed_texcoord, DirectX::XMLoadFloat2(&raw_texcoords[vertex_index]));

                        out_vertex_varying_binding[vertex_index].m_texcoord = packed_texcoord.v;
                    }
                }

                // Vertex Joint Binding
                if ((!raw_joint_indices.empty()) && (!raw_joint_weights.empty()))
                {
                    (*out_max_joint_index) = std::max((*out_max_joint_index), static_cast<int32_t>(raw_max_joint_index));

                    assert(out_subset_vertex_index_offset == out_subset_data->m_vertex_joint_binding.size());
                    out_subset_data->m_vertex_joint_binding.resize(out_subset_vertex_index_offset + vertex_count);
                    scene_mesh_vertex_joint_binding *const out_vertex_joint_binding = &out_subset_data->m_vertex_joint_binding[out_subset_vertex_index_offset];

                    for (size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
                    {
                        (*reinterpret_cast<uint64_t *>(&out_vertex_joint_binding[vertex_index].m_indices_xy)) = raw_joint_indices[vertex_index].v;

                        DirectX::PackedVector::XMUBYTEN4 packed_weights;
                        DirectX::PackedVector::XMStoreUByteN4(&packed_weights, DirectX::XMLoadFloat4(&raw_joint_weights[vertex_index]));

                        out_vertex_joint_binding[vertex_index].m_weights = packed_weights.v;
                    }
                }
                else
                {
                    assert(out_subset_data->m_vertex_joint_binding.empty());
                }
            }
        }
    }

    if ((*out_max_joint_index) < 0)
    {
        out_mesh_data->m_skinned = false;

        for (scene_mesh_subset_data &out_subset_data : out_mesh_data->m_subsets)
        {
            assert(out_subset_data.m_vertex_joint_binding.empty());
        }
    }
    else
    {
        out_mesh_data->m_skinned = true;
    }
}

static void import_gltf_scene_mesh_instance_asset(mcrt_vector<cgltf_node const *> &out_mesh_instance_nodes, mcrt_vector<DirectX::XMFLOAT4X4> &out_mesh_instance_node_world_transforms, cgltf_data const *data, cgltf_mesh const *mesh)
{
    mcrt_vector<DirectX::XMFLOAT4X4> node_local_transforms(static_cast<size_t>(data->nodes_count));

    for (size_t node_index = 0; node_index < data->nodes_count; ++node_index)
    {
        cgltf_node const *node = &data->nodes[node_index];

        if (node->has_matrix)
        {
            node_local_transforms[node_index] = (*reinterpret_cast<DirectX::XMFLOAT4X4 const *>(&node->matrix));
        }
        else
        {
            DirectX::XMVECTOR node_local_scale;
            if (node->has_scale)
            {
                node_local_scale = DirectX::XMLoadFloat3(reinterpret_cast<DirectX::XMFLOAT3 const *>(&node->scale));
            }
            else
            {
                node_local_scale = DirectX::XMVectorSplatOne();
            }

            DirectX::XMVECTOR node_local_rotation;
            if (node->has_rotation)
            {
                node_local_rotation = DirectX::XMLoadFloat4(reinterpret_cast<DirectX::XMFLOAT4 const *>(&node->rotation));
            }
            else
            {
                node_local_rotation = DirectX::XMQuaternionIdentity();
            }

            DirectX::XMVECTOR node_local_translation;
            if (node->has_translation)
            {
                node_local_translation = DirectX::XMLoadFloat3(reinterpret_cast<DirectX::XMFLOAT3 const *>(&node->translation));
            }
            else
            {
                node_local_translation = DirectX::XMVectorZero();
            }

            DirectX::XMStoreFloat4x4(&node_local_transforms[node_index], DirectX::XMMatrixMultiply(DirectX::XMMatrixScalingFromVector(node_local_scale), DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationQuaternion(node_local_rotation), DirectX::XMMatrixTranslationFromVector(node_local_translation))));
        }
    }

    mcrt_vector<DirectX::XMFLOAT4X4> node_world_transforms(static_cast<size_t>(data->nodes_count));

    internal_scene_depth_first_search_traverse(
        data,
        [](cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n) -> void
        {
            cgltf_mesh const *const mesh = static_cast<cgltf_mesh *>(user_data_x);
            mcrt_vector<DirectX::XMFLOAT4X4> &node_local_transforms = *static_cast<mcrt_vector<DirectX::XMFLOAT4X4> *>(user_data_y);
            mcrt_vector<DirectX::XMFLOAT4X4> &node_world_transforms = *static_cast<mcrt_vector<DirectX::XMFLOAT4X4> *>(user_data_z);
            mcrt_vector<cgltf_node const *> &out_mesh_instance_nodes = *static_cast<mcrt_vector<cgltf_node const *> *>(user_data_u);
            mcrt_vector<DirectX::XMFLOAT4X4> &out_mesh_instance_node_world_transforms = *static_cast<mcrt_vector<DirectX::XMFLOAT4X4> *>(user_data_v);
            assert(NULL == user_data_w);
            assert(NULL == user_data_l);
            assert(NULL == user_data_m);
            assert(NULL == user_data_n);

            DirectX::XMMATRIX local_transform = DirectX::XMLoadFloat4x4(&node_local_transforms[cgltf_node_index(data, current_node)]);

            DirectX::XMMATRIX parent_world_transform = (NULL != parent_node) ? DirectX::XMLoadFloat4x4(&node_world_transforms[cgltf_node_index(data, parent_node)]) : DirectX::XMMatrixIdentity();

            DirectX::XMMATRIX world_transform = DirectX::XMMatrixMultiply(local_transform, parent_world_transform);

            DirectX::XMStoreFloat4x4(&node_world_transforms[cgltf_node_index(data, current_node)], world_transform);

            if (current_node->mesh == mesh)
            {
                out_mesh_instance_nodes.push_back(current_node);
                out_mesh_instance_node_world_transforms.push_back(node_world_transforms[cgltf_node_index(data, current_node)]);
            }
        },
        const_cast<cgltf_mesh *>(mesh),
        &node_local_transforms,
        &node_world_transforms,
        &out_mesh_instance_nodes,
        &out_mesh_instance_node_world_transforms,
        NULL,
        NULL,
        NULL,
        NULL);
}

static void import_gltf_scene_animation_asset(scene_animation_skeleton *out_animated_skeleton, float frame_rate, cgltf_data const *data, cgltf_skin const *skin, cgltf_animation const *animation)
{
    // Skin
    // https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/main/source/Renderer/shaders/animation.glsl
    // defined(HAS_WEIGHTS_0_VEC4) && defined(HAS_JOINTS_0_VEC4)
    // defined(HAS_WEIGHTS_1_VEC4) && defined(HAS_JOINTS_1_VEC4)
    size_t const joint_count = skin->joints_count;

    // Due to the hierarchy, we can NOT know which nodes are really influencing the skeleton joints. We have to consider all nodes.
    mcrt_vector<DirectX::XMFLOAT3> node_local_scales(static_cast<size_t>(data->nodes_count));
    mcrt_vector<DirectX::XMFLOAT4> node_local_rotations(static_cast<size_t>(data->nodes_count));
    mcrt_vector<DirectX::XMFLOAT3> node_local_translations(static_cast<size_t>(data->nodes_count));

    for (size_t node_index = 0; node_index < data->nodes_count; ++node_index)
    {
        cgltf_node const *node = &data->nodes[node_index];

        if (node->has_matrix)
        {
            DirectX::XMVECTOR out_node_local_scale;
            DirectX::XMVECTOR out_node_local_rotation;
            DirectX::XMVECTOR out_node_local_translation;
            DirectX::XMMatrixDecompose(&out_node_local_scale, &out_node_local_rotation, &out_node_local_translation, DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 const *>(&node->matrix)));

            DirectX::XMStoreFloat3(&node_local_scales[node_index], out_node_local_scale);
            DirectX::XMStoreFloat4(&node_local_rotations[node_index], out_node_local_rotation);
            DirectX::XMStoreFloat3(&node_local_translations[node_index], out_node_local_translation);
        }
        else
        {
            if (node->has_scale)
            {
                node_local_scales[node_index].x = node->scale[0];
                node_local_scales[node_index].y = node->scale[1];
                node_local_scales[node_index].z = node->scale[2];
            }
            else
            {
                DirectX::XMStoreFloat3(&node_local_scales[node_index], DirectX::XMVectorSplatOne());
            }

            if (node->has_rotation)
            {
                node_local_rotations[node_index].x = node->rotation[0];
                node_local_rotations[node_index].y = node->rotation[1];
                node_local_rotations[node_index].z = node->rotation[2];
                node_local_rotations[node_index].w = node->rotation[3];
            }
            else
            {
                DirectX::XMStoreFloat4(&node_local_rotations[node_index], DirectX::XMQuaternionIdentity());
            }

            if (node->has_translation)
            {
                node_local_translations[node_index].x = node->translation[0];
                node_local_translations[node_index].y = node->translation[1];
                node_local_translations[node_index].z = node->translation[2];
            }
            else
            {
                DirectX::XMStoreFloat3(&node_local_translations[node_index], DirectX::XMVectorZero());
            }
        }
    }

    // Animation
    // https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/main/source/gltf/animation.js
    // gltfAnimation.advance
    // https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/main/source/gltf/scene.js
    // applyTransformHierarchy
    // https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/main/source/gltf/skin.js
    // gltfSkin.computeJoints
    size_t const channel_count = (NULL != animation) ? animation->channels_count : 0;

    float animation_max_time = -1.0;
    mcrt_vector<float> channel_min_key_times(channel_count, -1.0F);
    mcrt_vector<float> channel_max_key_times(channel_count, -1.0F);
    for (size_t channel_index = 0; channel_index < channel_count; ++channel_index)
    {
        float channel_min_key_time = -1.0;
        float channel_max_key_time = -1.0;
        {
            cgltf_accessor const *channel_time_accessor = animation->channels[channel_index].sampler->input;

            assert(cgltf_type_scalar == channel_time_accessor->type);
            assert(cgltf_component_type_r_32f == channel_time_accessor->component_type);
            assert(sizeof(float) == channel_time_accessor->stride);

            cgltf_bool result_accessor_read_float_min = cgltf_accessor_read_float(channel_time_accessor, 0, &channel_min_key_time, 1);
            assert(result_accessor_read_float_min);

            cgltf_bool result_accessor_read_float_max = cgltf_accessor_read_float(channel_time_accessor, channel_time_accessor->count - 1, &channel_max_key_time, 1);
            assert(result_accessor_read_float_max);
        }

        channel_min_key_times[channel_index] = channel_min_key_time;
        channel_max_key_times[channel_index] = channel_max_key_time;
        animation_max_time = std::max(channel_max_key_time, animation_max_time);
    }

    size_t const frame_count = (NULL != animation) ? static_cast<size_t>(frame_rate * animation_max_time) : 1;

    out_animated_skeleton->init(frame_count, joint_count);

    mcrt_vector<float> channel_previous_sample_times(channel_count, 0.0F);
    mcrt_vector<size_t> channel_previous_key_indices(channel_count, 0);
    for (size_t frame_index = 0; frame_index < frame_count; ++frame_index)
    {
        if (NULL != animation)
        {
            float const frame_sample_time = static_cast<float>((0.5 / static_cast<double>(frame_rate)) + (1.0 / static_cast<double>(frame_rate)) * static_cast<double>(frame_index));
            assert(frame_sample_time < animation_max_time);

            for (size_t channel_index = 0; channel_index < channel_count; ++channel_index)
            {
                cgltf_animation_channel const *channel = &animation->channels[channel_index];

                cgltf_accessor const *channel_time_accessor = channel->sampler->input;

                assert(cgltf_type_scalar == channel_time_accessor->type);
                assert(cgltf_component_type_r_32f == channel_time_accessor->component_type);
                assert(sizeof(float) == channel_time_accessor->stride);

                size_t const channel_key_count = channel_time_accessor->count;
                assert(channel_key_count >= 1);

                float const channel_sample_time = std::min(std::max(frame_sample_time, channel_min_key_times[channel_index]), channel_max_key_times[channel_index]);

                if (channel_previous_sample_times[channel_index] > channel_sample_time)
                {
                    assert(0);
                    channel_previous_key_indices[channel_index] = 0;
                }

                channel_previous_sample_times[channel_index] = channel_sample_time;

                float channel_previous_key_time = -1.0;
                float channel_next_key_time = channel_max_key_times[channel_index];
                size_t next_key_index = -1;
                for (size_t channel_key_index = channel_previous_key_indices[channel_index]; channel_key_index < channel_key_count; ++channel_key_index)
                {
                    float channel_key_time = -1.0;
                    {
                        cgltf_bool result_accessor_read_float = cgltf_accessor_read_float(channel_time_accessor, channel_key_index, &channel_key_time, 1);
                        assert(result_accessor_read_float);
                    }

                    if (channel_sample_time <= channel_key_time)
                    {
                        assert(channel_previous_key_time >= 0.0);
                        assert(channel_max_key_times[channel_index] == channel_next_key_time);
                        assert(-1 == next_key_index);
                        channel_next_key_time = channel_key_time;
                        next_key_index = std::min(std::max(static_cast<size_t>(1), channel_key_index), channel_key_count - 1);
                        channel_previous_key_indices[channel_index] = (next_key_index > 1) ? (next_key_index - 1) : 0;
                        break;
                    }

                    channel_previous_key_time = channel_key_time;
                }
                assert(channel_previous_key_time <= channel_sample_time);
                assert(channel_sample_time <= channel_next_key_time);

                float const channel_delta_key_time = channel_next_key_time - channel_previous_key_time;
                assert(channel_delta_key_time >= 0.0);

                float const channel_time_normalized = (channel_next_key_time > channel_previous_key_time && channel_delta_key_time > 0.0) ? ((channel_sample_time - channel_previous_key_time) / channel_delta_key_time) : static_cast<float>(0.0);

                cgltf_accessor const *channel_animated_property_accessor = channel->sampler->output;

                if (NULL != channel->target_node)
                {
                    size_t const target_node_index = cgltf_node_index(data, channel->target_node);

                    switch (channel->target_path)
                    {
                    case cgltf_animation_path_type_scale:
                    {
                        assert(cgltf_animation_path_type_scale == channel->target_path);

                        assert(cgltf_type_vec3 == channel_animated_property_accessor->type);
                        assert(cgltf_component_type_r_32f == channel_animated_property_accessor->component_type);
                        assert((sizeof(float) * 3) == channel_animated_property_accessor->stride);

                        switch (channel->sampler->interpolation)
                        {
                        case cgltf_interpolation_type_step:
                        case cgltf_interpolation_type_linear:
                        {
                            // NOTE: We promote STEP to LINEAR.
                            assert(cgltf_interpolation_type_step == channel->sampler->interpolation || cgltf_interpolation_type_linear == channel->sampler->interpolation);

                            DirectX::XMFLOAT3 channel_previous_key_scale;
                            DirectX::XMFLOAT3 channel_next_key_scale;
                            {
                                cgltf_bool result_accessor_read_float_previous_key = cgltf_accessor_read_float(channel_animated_property_accessor, channel_previous_key_indices[channel_index], reinterpret_cast<float *>(&channel_previous_key_scale), 3);
                                assert(result_accessor_read_float_previous_key);

                                cgltf_bool result_accessor_read_float_next_key = cgltf_accessor_read_float(channel_animated_property_accessor, next_key_index, reinterpret_cast<float *>(&channel_next_key_scale), 3);
                                assert(result_accessor_read_float_next_key);
                            }

                            DirectX::XMStoreFloat3(&node_local_scales[target_node_index], DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&channel_previous_key_scale), DirectX::XMLoadFloat3(&channel_next_key_scale), channel_time_normalized));
                        }
                        break;
                        case cgltf_interpolation_type_cubic_spline:
                        default:
                        {
                            assert(0);
                        }
                        }
                    }
                    break;
                    case cgltf_animation_path_type_rotation:
                    {
                        assert(cgltf_animation_path_type_rotation == channel->target_path);

                        assert(cgltf_type_vec4 == channel_animated_property_accessor->type);
                        assert(cgltf_component_type_r_32f == channel_animated_property_accessor->component_type);
                        assert((sizeof(float) * 4) == channel_animated_property_accessor->stride);

                        switch (channel->sampler->interpolation)
                        {
                        case cgltf_interpolation_type_step:
                        case cgltf_interpolation_type_linear:
                        {
                            // NOTE: We promote STEP to LINEAR.
                            assert(cgltf_interpolation_type_step == channel->sampler->interpolation || cgltf_interpolation_type_linear == channel->sampler->interpolation);

                            DirectX::XMFLOAT4 channel_previous_key_rotation;
                            DirectX::XMFLOAT4 channel_next_key_rotation;
                            {
                                cgltf_bool result_accessor_read_float_previous_key = cgltf_accessor_read_float(channel_animated_property_accessor, channel_previous_key_indices[channel_index], reinterpret_cast<float *>(&channel_previous_key_rotation), 4);
                                assert(result_accessor_read_float_previous_key);

                                cgltf_bool result_accessor_read_float_next_key = cgltf_accessor_read_float(channel_animated_property_accessor, next_key_index, reinterpret_cast<float *>(&channel_next_key_rotation), 4);
                                assert(result_accessor_read_float_next_key);
                            }

                            DirectX::XMStoreFloat4(&node_local_rotations[target_node_index], DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&channel_previous_key_rotation), DirectX::XMLoadFloat4(&channel_next_key_rotation), channel_time_normalized));
                        }
                        break;
                        case cgltf_interpolation_type_cubic_spline:
                        default:
                        {
                            assert(0);
                        }
                        }
                    }
                    break;
                    case cgltf_animation_path_type_translation:
                    {
                        assert(cgltf_animation_path_type_translation == channel->target_path);

                        assert(cgltf_type_vec3 == channel_animated_property_accessor->type);
                        assert(cgltf_component_type_r_32f == channel_animated_property_accessor->component_type);
                        assert((sizeof(float) * 3) == channel_animated_property_accessor->stride);

                        switch (channel->sampler->interpolation)
                        {
                        case cgltf_interpolation_type_step:
                        case cgltf_interpolation_type_linear:
                        {
                            // NOTE: We promote STEP to LINEAR.
                            assert(cgltf_interpolation_type_step == channel->sampler->interpolation || cgltf_interpolation_type_linear == channel->sampler->interpolation);

                            DirectX::XMFLOAT3 channel_previous_key_translation;
                            DirectX::XMFLOAT3 channel_next_key_translation;
                            {
                                cgltf_bool result_accessor_read_float_previous_key = cgltf_accessor_read_float(channel_animated_property_accessor, channel_previous_key_indices[channel_index], reinterpret_cast<float *>(&channel_previous_key_translation), 3);
                                assert(result_accessor_read_float_previous_key);

                                cgltf_bool result_accessor_read_float_next_key = cgltf_accessor_read_float(channel_animated_property_accessor, next_key_index, reinterpret_cast<float *>(&channel_next_key_translation), 3);
                                assert(result_accessor_read_float_next_key);
                            }

                            DirectX::XMStoreFloat3(&node_local_translations[target_node_index], DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&channel_previous_key_translation), DirectX::XMLoadFloat3(&channel_next_key_translation), channel_time_normalized));
                        }
                        break;
                        case cgltf_interpolation_type_cubic_spline:
                        default:
                        {
                            assert(0);
                        }
                        }
                    }
                    break;
                    case cgltf_animation_path_type_weights:
                    default:
                    {
                        assert(0);
                    }
                    }
                }
                else
                {
                    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#animations
                    // When **node** is NOT defined, channel **SHOULD** be ignored.
                    assert(0);
                }
            }
        }

        mcrt_vector<DirectX::XMFLOAT4X4> node_world_transforms(static_cast<size_t>(data->nodes_count));

        internal_scene_depth_first_search_traverse(
            data,
            [](cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n) -> void
            {
                mcrt_vector<DirectX::XMFLOAT3> &node_local_scales = *static_cast<mcrt_vector<DirectX::XMFLOAT3> *>(user_data_x);
                mcrt_vector<DirectX::XMFLOAT4> &node_local_rotations = *static_cast<mcrt_vector<DirectX::XMFLOAT4> *>(user_data_y);
                mcrt_vector<DirectX::XMFLOAT3> &node_local_translations = *static_cast<mcrt_vector<DirectX::XMFLOAT3> *>(user_data_z);
                mcrt_vector<DirectX::XMFLOAT4X4> &node_world_transforms = *static_cast<mcrt_vector<DirectX::XMFLOAT4X4> *>(user_data_u);
                assert(NULL == user_data_v);
                assert(NULL == user_data_w);
                assert(NULL == user_data_l);
                assert(NULL == user_data_m);
                assert(NULL == user_data_n);

                DirectX::XMMATRIX local_transform = DirectX::XMMatrixMultiply(DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&node_local_scales[cgltf_node_index(data, current_node)])), DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node_local_rotations[cgltf_node_index(data, current_node)])), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&node_local_translations[cgltf_node_index(data, current_node)]))));

                DirectX::XMMATRIX parent_world_transform = (NULL != parent_node) ? DirectX::XMLoadFloat4x4(&node_world_transforms[cgltf_node_index(data, parent_node)]) : DirectX::XMMatrixIdentity();

                DirectX::XMMATRIX world_transform = DirectX::XMMatrixMultiply(local_transform, parent_world_transform);

                DirectX::XMStoreFloat4x4(&node_world_transforms[cgltf_node_index(data, current_node)], world_transform);
            },
            &node_local_scales,
            &node_local_rotations,
            &node_local_translations,
            &node_world_transforms,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);

        cgltf_accessor const *skin_inverse_bind_matrix_accessor = skin->inverse_bind_matrices;

        assert(cgltf_type_mat4 == skin_inverse_bind_matrix_accessor->type);
        assert(cgltf_component_type_r_32f == skin_inverse_bind_matrix_accessor->component_type);
        assert((sizeof(float) * 16) == skin_inverse_bind_matrix_accessor->stride);

        for (size_t joint_index = 0; joint_index < joint_count; ++joint_index)
        {
            size_t const joint_node_index = cgltf_node_index(data, skin->joints[joint_index]);

            DirectX::XMFLOAT4X4 inverse_bind_matrix;
            {
                cgltf_bool result_accessor_read_float_inverse_bind_matrix = cgltf_accessor_read_float(skin_inverse_bind_matrix_accessor, joint_index, reinterpret_cast<float *>(&inverse_bind_matrix), 16);
                assert(result_accessor_read_float_inverse_bind_matrix);
            }

            DirectX::XMMATRIX joint_transform = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&inverse_bind_matrix), DirectX::XMLoadFloat4x4(&node_world_transforms[joint_node_index]));

            DirectX::XMVECTOR out_joint_scale;
            DirectX::XMVECTOR out_joint_rotation;
            DirectX::XMVECTOR out_joint_translation;
            DirectX::XMMatrixDecompose(&out_joint_scale, &out_joint_rotation, &out_joint_translation, joint_transform);

            // FLT_EPSILON
            constexpr float const scale_epsilon = 7E-5F;
            assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_joint_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(scale_epsilon))));

            DirectX::XMFLOAT4 joint_rotation;
            DirectX::XMFLOAT3 joint_translation;
            DirectX::XMStoreFloat4(&joint_rotation, out_joint_rotation);
            DirectX::XMStoreFloat3(&joint_translation, out_joint_translation);

            out_animated_skeleton->set_transform(frame_index, joint_index, joint_rotation, joint_translation);
        }
    }
}

static inline void internal_scene_depth_first_search_traverse(cgltf_data const *data, void (*pfn_user_callback)(cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n), void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n)
{
    static constexpr size_t INTERNAL_NODE_INDEX_INVALID = static_cast<size_t>(~static_cast<size_t>(0U));

    struct depth_first_search_stack_element_type
    {
        size_t parent_node_index;
        size_t current_node_index;
    };

    mcrt_vector<depth_first_search_stack_element_type> depth_first_search_stack;

    mcrt_vector<bool> node_visited_flags(static_cast<size_t>(data->nodes_count), false);
    mcrt_vector<bool> node_pushed_flags(static_cast<size_t>(data->nodes_count), false);

    for (size_t scene_node_index_index = data->scene->nodes_count; scene_node_index_index > 0U; --scene_node_index_index)
    {
        size_t const scene_node_index = cgltf_node_index(data, data->scene->nodes[scene_node_index_index - 1U]);
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
            data,
            &data->nodes[current_stack_element.current_node_index],
            (INTERNAL_NODE_INDEX_INVALID != current_stack_element.parent_node_index) ? &data->nodes[current_stack_element.parent_node_index] : NULL,
            user_data_x,
            user_data_y,
            user_data_z,
            user_data_u,
            user_data_v,
            user_data_w,
            user_data_l,
            user_data_m,
            user_data_n);

        for (size_t child_node_index_index = data->nodes[current_stack_element.current_node_index].children_count; child_node_index_index > 0U; --child_node_index_index)
        {
            size_t const child_node_index = cgltf_node_index(data, data->nodes[current_stack_element.current_node_index].children[child_node_index_index - 1U]);
            if ((!node_visited_flags[child_node_index]) && (!node_pushed_flags[child_node_index]))
            {
                node_pushed_flags[child_node_index] = true;
                depth_first_search_stack.push_back({current_stack_element.current_node_index, child_node_index});
            }
            else
            {
                assert(false);
                std::cout << "Multiple Parents Exist for Node: " << static_cast<int>(current_stack_element.current_node_index) << std::endl;
            }
        }
    }
}

static inline void internal_scene_breadth_first_search_traverse(cgltf_data const *data, void (*pfn_user_callback)(cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n), void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n)
{
    static constexpr size_t INTERNAL_NODE_INDEX_INVALID = static_cast<size_t>(~static_cast<size_t>(0U));

    struct breadth_first_search_queue_element_type
    {
        size_t parent_node_index;
        size_t current_node_index;
    };

    mcrt_deque<breadth_first_search_queue_element_type> breadth_first_search_queue;

    mcrt_vector<bool> node_visited_flags(static_cast<size_t>(data->nodes_count), false);
    mcrt_vector<bool> node_pushed_flags(static_cast<size_t>(data->nodes_count), false);

    for (size_t scene_node_index_index = 0; scene_node_index_index < data->scene->nodes_count; ++scene_node_index_index)
    {
        size_t const scene_node_index = cgltf_node_index(data, data->scene->nodes[scene_node_index_index]);
        assert(!node_pushed_flags[scene_node_index]);
        node_pushed_flags[scene_node_index] = true;
        breadth_first_search_queue.push_back({INTERNAL_NODE_INDEX_INVALID, scene_node_index});
    }

    while (!breadth_first_search_queue.empty())
    {
        breadth_first_search_queue_element_type current_queue_element = breadth_first_search_queue.front();
        breadth_first_search_queue.pop_front();

        assert(!node_visited_flags[current_queue_element.current_node_index]);
        node_visited_flags[current_queue_element.current_node_index] = true;

        pfn_user_callback(
            data,
            &data->nodes[current_queue_element.current_node_index],
            (INTERNAL_NODE_INDEX_INVALID != current_queue_element.parent_node_index) ? &data->nodes[current_queue_element.parent_node_index] : NULL,
            user_data_x,
            user_data_y,
            user_data_z,
            user_data_u,
            user_data_v,
            user_data_w,
            user_data_l,
            user_data_m,
            user_data_n);

        for (size_t child_node_index_index = 0U; child_node_index_index < data->nodes[current_queue_element.current_node_index].children_count; ++child_node_index_index)
        {
            size_t const child_node_index = cgltf_node_index(data, data->nodes[current_queue_element.current_node_index].children[child_node_index_index]);
            if ((!node_visited_flags[child_node_index]) && (!node_pushed_flags[child_node_index]))
            {
                node_pushed_flags[child_node_index] = true;
                breadth_first_search_queue.push_back({current_queue_element.current_node_index, child_node_index});
            }
            else
            {
                assert(false);
                std::cout << "Multiple Parents Exist for Node: " << static_cast<int>(current_queue_element.current_node_index) << std::endl;
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

#include "../include/brx_asset_import_input_stream.h"

extern cgltf_result cgltf_custom_read_file(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *file_options, const char *path, cgltf_size *size, void **data)
{
    void *(*const memory_alloc)(void *, cgltf_size) = memory_options->alloc_func;
    assert(NULL != memory_alloc);

    void (*const memory_free)(void *, void *) = memory_options->free_func;
    assert(NULL != memory_free);

    brx_asset_import_input_stream_factory *const input_stream_factory = static_cast<brx_asset_import_input_stream_factory *>(file_options->user_data);

    brx_asset_import_input_stream *file;
    if (NULL == (file = input_stream_factory->create_instance(path)))
    {
        return cgltf_result_file_not_found;
    }

    cgltf_size file_size = (NULL != size) ? (*size) : 0;
    if (file_size == 0)
    {
        int64_t length;
        if (-1 == file->stat_size(&length))
        {
            input_stream_factory->destory_instance(file);
            return cgltf_result_io_error;
        }

        file_size = length;
    }

    void *file_data = memory_alloc(memory_options->user_data, file_size);
    if (NULL == file_data)
    {
        input_stream_factory->destory_instance(file);
        return cgltf_result_out_of_memory;
    }

    intptr_t read_size = file->read(file_data, file_size);

    input_stream_factory->destory_instance(file);

    if (-1 == read_size || read_size != file_size)
    {
        memory_free(memory_options->user_data, file_data);
        return cgltf_result_io_error;
    }

    if (NULL != size)
    {
        *size = file_size;
    }
    if (NULL != data)
    {
        *data = file_data;
    }

    return cgltf_result_success;
}

extern void cgltf_custom_file_release(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, void *data)
{
    void (*const memory_free)(void *, void *) = memory_options->free_func;
    assert(NULL != memory_free);

    memory_free(memory_options->user_data, data);
}

#include "../../McRT-Malloc/include/mcrt_malloc.h"

extern void *cgltf_custom_alloc(void *, cgltf_size size)
{
    return mcrt_malloc(size, alignof(size_t));
}

extern void cgltf_custom_free(void *, void *ptr)
{
    mcrt_free(ptr);
}

#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#include <arm_acle.h>
#else
#error Unknown Architecture
#endif
#elif defined(_MSC_VER)
#if defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
#elif defined(_M_ARM64) || defined(_M_ARM)
#include <intrin.h>
#else
#error Unknown Architecture
#endif
#else
#error Unknown Compiler
#endif

static inline uint32_t sqrt_ceil(uint32_t x)
{
    uint32_t y;
    if (x > 1U)
    {
#if defined(__GNUC__)
        unsigned int index = (31U - static_cast<unsigned int>(__builtin_clz(static_cast<unsigned int>(x - 1U))));
        y = (static_cast<uint32_t>(1U) << ((static_cast<uint32_t>(index) + static_cast<uint32_t>(2U)) / static_cast<uint32_t>(2U)));
#elif defined(_MSC_VER)
#if defined(_M_X64) || defined(_M_IX86)
        unsigned long index;
        _BitScanReverse(&index, static_cast<unsigned long>(x - 1U));
        y = (static_cast<uint32_t>(1U) << ((static_cast<uint32_t>(index) + static_cast<uint32_t>(2U)) / static_cast<uint32_t>(2U)));
#elif defined(_M_ARM64) || defined(_M_ARM)
        unsigned int index = _CountLeadingZeros(static_cast<unsigned long>(x - 1U));
        y = (static_cast<uint32_t>(1U) << ((static_cast<uint32_t>(index) + static_cast<uint32_t>(2U)) / static_cast<uint32_t>(2U)));
#else
#error Unknown Architecture
#endif
#else
#error Unknown Compiler
#endif
    }
    else
    {
        y = 1U;
    }
    return y;
}

static inline void decode_morton2(uint32_t const v, uint32_t &x, uint32_t &y)
{
    // PBRT-V4: [DecodeMorton2](https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/math.h#L141)

#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__i386__)
    x = _pext_u32(v, 0x55555555);
    y = _pext_u32((v >> 1), 0x55555555);
#elif defined(__aarch64__) || defined(__arm__)
    x = (v & 0x55555555);
    x = (x ^ (x >> 1)) & 0x33333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff;

    y = ((v >> 1) & 0x55555555);
    y = (y ^ (y >> 1)) & 0x33333333;
    y = (y ^ (y >> 2)) & 0x0f0f0f0f;
    y = (y ^ (y >> 4)) & 0x00ff00ff;
    y = (y ^ (y >> 8)) & 0x0000ffff;
#else
#error Unknown Architecture
#endif
#elif defined(_MSC_VER)
#if defined(_M_X64) || defined(_M_IX86)
    x = _pext_u32(v, 0x55555555);
    y = _pext_u32((v >> 1), 0x55555555);
#elif defined(_M_ARM64) || defined(_M_ARM)
    x = (v & 0x5555555555555555);
    x = (x ^ (x >> 1)) & 0x3333333333333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff0000ffff;
    x = (x ^ (x >> 16)) & 0xffffffff;

    y = ((v >> 1) & 0x5555555555555555);
    y = (y ^ (y >> 1)) & 0x3333333333333333;
    y = (y ^ (y >> 2)) & 0x0f0f0f0f0f0f0f0f;
    y = (y ^ (y >> 4)) & 0x00ff00ff00ff00ff;
    y = (y ^ (y >> 8)) & 0x0000ffff0000ffff;
    y = (y ^ (y >> 16)) & 0xffffffff;
#else
#error Unknown Architecture
#endif
#else
#error Unknown Compiler
#endif
}
