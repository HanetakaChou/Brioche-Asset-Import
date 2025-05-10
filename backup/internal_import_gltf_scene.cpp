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

#include "internal_import_gltf_scene.h"
#include "../../McRT-Malloc/include/mcrt_deque.h"
#include "../../McRT-Malloc/include/mcrt_unordered_set.h"
#include "../thirdparty/cgltf/cgltf.h"
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../thirdparty/DirectXMesh/DirectXMesh/DirectXMesh.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <iostream>
#include <cassert>

extern cgltf_result cgltf_custom_read_file(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, const char *path, cgltf_size *size, void **data);

extern void cgltf_custom_file_release(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, void *data);

extern void *cgltf_custom_alloc(void *, cgltf_size size);

extern void cgltf_custom_free(void *, void *ptr);

// TODO: change to static
extern void internal_import_skeleton(cgltf_data const *data, mcrt_vector<DirectX::XMFLOAT4X4> &out_internal_node_world_transforms, mcrt_vector<mcrt_vector<cgltf_node const *>> &out_internal_mesh_instance_nodes, mcrt_vector<uint32_t> &out_internal_node_index_to_skeleton_joint_index, mcrt_vector<mcrt_string> &out_skeleton_joint_names, mcrt_vector<uint32_t> &out_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_rigid_transform> &out_skeleton_bind_pose_joint_transforms, uint32_t *out_vrm_skeleton_joint_names);

static inline void internal_import_surface(cgltf_data const *data, mcrt_vector<DirectX::XMFLOAT4X4> const &internal_node_world_transforms, mcrt_vector<mcrt_vector<cgltf_node const *>> const &internal_mesh_instances, mcrt_vector<uint32_t> const &internal_node_index_to_skeleton_joint_index, mcrt_vector<brx_asset_import_mesh_surface> &out_surfaces);

// TODO: change to static
// TODO: change name "out_skeleton_joint_names" to "animation channel name"
extern void internal_import_skeleton_animation(cgltf_data const *data, cgltf_animation const *animation, uint32_t animation_frame_rate, mcrt_vector<mcrt_string> &out_skeleton_joint_names, mcrt_vector<brx_asset_import_rigid_transform> &out_skeleton_animation_joint_transforms);

static inline mcrt_string internal_import_skeleton_joint_name(cgltf_data const *data, cgltf_node const *node);

static inline void internal_scene_depth_first_search_traverse(cgltf_data const *data, void (*pfn_user_callback)(cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n), void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n);

static inline void internal_scene_breadth_first_search_traverse(cgltf_data const *data, void (*pfn_user_callback)(cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n), void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n);

extern bool internal_import_gltf_scene(brx_asset_import_input_stream_factory *input_stream_factory, char const *file_name, mcrt_vector<brx_asset_import_mesh_surface_group> &out_surface_groups)
{
    cgltf_data *data = NULL;
    {
        cgltf_options options = {};
        options.memory.alloc_func = cgltf_custom_alloc;
        options.memory.free_func = cgltf_custom_free;
        options.file.read = cgltf_custom_read_file;
        options.file.release = cgltf_custom_file_release;
        options.file.user_data = input_stream_factory;

        cgltf_result result_parse_file = cgltf_parse_file(&options, file_name, &data);
        if (cgltf_result_success != result_parse_file)
        {
            return false;
        }

        cgltf_result result_load_buffers = cgltf_load_buffers(&options, data, file_name);
        if (cgltf_result_success != result_load_buffers)
        {
            cgltf_free(data);
            return false;
        }
    }

    mcrt_vector<DirectX::XMFLOAT4X4> internal_node_world_transforms;
    mcrt_vector<mcrt_vector<cgltf_node const *>> internal_mesh_instances;
    mcrt_vector<mcrt_string> skeleton_joint_names;
    mcrt_vector<uint32_t> skeleton_joint_parent_indices;
    mcrt_vector<brx_asset_import_rigid_transform> skeleton_bind_pose_joint_transforms;
    uint32_t vrm_skeleton_joint_indices[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT];
    mcrt_vector<uint32_t> internal_node_index_to_skeleton_joint_index;
    internal_import_skeleton(data, internal_node_world_transforms, internal_mesh_instances, internal_node_index_to_skeleton_joint_index, skeleton_joint_names, skeleton_joint_parent_indices, skeleton_bind_pose_joint_transforms, &vrm_skeleton_joint_indices[0]);

    cgltf_free(data);
    return true;
}

// FLT_EPSILON
static inline constexpr float const INTERNAL_SCALE_EPSILON = 7E-5F;

extern void internal_import_skeleton(cgltf_data const *data, mcrt_vector<DirectX::XMFLOAT4X4> &out_internal_node_world_transforms, mcrt_vector<mcrt_vector<cgltf_node const *>> &out_internal_mesh_instance_nodes, mcrt_vector<uint32_t> &out_internal_node_index_to_skeleton_joint_index, mcrt_vector<mcrt_string> &out_skeleton_joint_names, mcrt_vector<uint32_t> &out_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_rigid_transform> &out_skeleton_bind_pose_joint_transforms, uint32_t *out_vrm_skeleton_joint_names)
{
    // TODO: support
    static_assert(-1 == static_cast<int32_t>(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID), "");
    std::memset(out_vrm_skeleton_joint_names, 0XFF, sizeof(uint32_t) * BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT);

    assert(out_internal_node_world_transforms.empty());
    assert(out_internal_mesh_instance_nodes.empty());
    assert(out_internal_node_index_to_skeleton_joint_index.empty());
    assert(out_skeleton_joint_names.empty());
    assert(out_skeleton_joint_parent_indices.empty());
    assert(out_skeleton_bind_pose_joint_transforms.empty());

    mcrt_unordered_set<uint32_t> node_index_in_skeleton;

    mcrt_vector<brx_asset_import_rigid_transform> node_bind_pose_transforms(static_cast<size_t>(data->nodes_count));

    uint32_t root_node_index;

    {
        mcrt_vector<uint32_t> node_parent_indices(static_cast<size_t>(data->nodes_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

        // some nodes may be NOT included in the scene // we initialize by the identity transform
        {
            DirectX::XMFLOAT4X4 identity_transform;
            DirectX::XMStoreFloat4x4(&identity_transform, DirectX::XMMatrixIdentity());
            out_internal_node_world_transforms.assign(static_cast<size_t>(data->nodes_count), identity_transform);
        }

        out_internal_mesh_instance_nodes.resize(data->meshes_count);

        mcrt_unordered_set<cgltf_skin const *> skins;

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

            internal_scene_depth_first_search_traverse(
                data,
                [](cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n) -> void
                {
                    mcrt_vector<uint32_t> &node_parent_indices = *static_cast<mcrt_vector<uint32_t> *>(user_data_x);
                    mcrt_vector<DirectX::XMFLOAT4X4> &node_local_transforms = *static_cast<mcrt_vector<DirectX::XMFLOAT4X4> *>(user_data_y);
                    mcrt_vector<DirectX::XMFLOAT4X4> &out_internal_node_world_transforms = *static_cast<mcrt_vector<DirectX::XMFLOAT4X4> *>(user_data_z);
                    mcrt_vector<mcrt_vector<cgltf_node const *>> &out_internal_mesh_instance_nodes = *static_cast<mcrt_vector<mcrt_vector<cgltf_node const *>> *>(user_data_u);
                    mcrt_unordered_set<cgltf_skin const *> &skins = *static_cast<mcrt_unordered_set<cgltf_skin const *> *>(user_data_v);
                    assert(NULL == user_data_w);
                    assert(NULL == user_data_l);
                    assert(NULL == user_data_m);
                    assert(NULL == user_data_n);

                    DirectX::XMMATRIX local_transform = DirectX::XMLoadFloat4x4(&node_local_transforms[cgltf_node_index(data, current_node)]);

                    DirectX::XMMATRIX parent_world_transform = (NULL != parent_node) ? DirectX::XMLoadFloat4x4(&out_internal_node_world_transforms[cgltf_node_index(data, parent_node)]) : DirectX::XMMatrixIdentity();

                    DirectX::XMMATRIX world_transform = DirectX::XMMatrixMultiply(local_transform, parent_world_transform);

                    DirectX::XMStoreFloat4x4(&out_internal_node_world_transforms[cgltf_node_index(data, current_node)], world_transform);

                    assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == node_parent_indices[cgltf_node_index(data, current_node)]);
                    node_parent_indices[cgltf_node_index(data, current_node)] = ((NULL != parent_node) ? cgltf_node_index(data, parent_node) : BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

                    assert(NULL == current_node->skin || NULL != current_node->mesh);

                    if (NULL != current_node->mesh)
                    {
                        assert(out_internal_mesh_instance_nodes[cgltf_mesh_index(data, current_node->mesh)].end() == std::find(out_internal_mesh_instance_nodes[cgltf_mesh_index(data, current_node->mesh)].begin(), out_internal_mesh_instance_nodes[cgltf_mesh_index(data, current_node->mesh)].end(), current_node));
                        out_internal_mesh_instance_nodes[cgltf_mesh_index(data, current_node->mesh)].push_back(current_node);

                        if (NULL != current_node->skin)
                        {
                            skins.insert(current_node->skin);
                        }
                    }
                },
                &node_parent_indices,
                &node_local_transforms,
                &out_internal_node_world_transforms,
                &out_internal_mesh_instance_nodes,
                &skins,
                NULL,
                NULL,
                NULL,
                NULL);
        }

        // there may be skins which are NOT included in the scene
        for (cgltf_size skin_index = 0U; skin_index < data->skins_count; ++skin_index)
        {
            if (skins.end() == std::find(skins.begin(), skins.end(), &data->skins[skin_index]))
            {
                std::cout << "Skeleton: \"" << data->skins[skin_index].name << "\" is not included in the scene and has been ignored." << std::endl;
            }
        }

        if (skins.empty())
        {
            // no skeleton
            return;
        }

        if (skins.size() > 1U)
        {
            std::cout << "Multiple skeletons found in the scene:";
            for (cgltf_skin const *skin : skins)
            {
                std::cout << " [" << ((NULL != skin->name) ? skin->name : "Name N/A") << " ]";
            }
            std::cout << ". The bind pose of the first skeleton will prevail over the others in case of duplication." << std::endl;
        }

        mcrt_unordered_set<uint32_t> root_node_index_in_skeleton;

        mcrt_vector<bool> node_bind_pose_transform_initilzed_flags(static_cast<size_t>(data->nodes_count), false);

        for (cgltf_skin const *skin : skins)
        {
            cgltf_accessor const *skin_inverse_bind_matrix_accessor = skin->inverse_bind_matrices;

            assert(cgltf_type_mat4 == skin_inverse_bind_matrix_accessor->type);
            assert(cgltf_component_type_r_32f == skin_inverse_bind_matrix_accessor->component_type);
            assert((sizeof(float) * 16) == skin_inverse_bind_matrix_accessor->stride);

            for (size_t joint_index = 0; joint_index < skin->joints_count; ++joint_index)
            {
                mcrt_unordered_set<uint32_t>::iterator found_node_index;
                size_t node_index = cgltf_node_index(data, skin->joints[joint_index]);
                while (node_index_in_skeleton.end() == (found_node_index = node_index_in_skeleton.find(node_index)))
                {
                    node_index_in_skeleton.insert(found_node_index, node_index);

                    if (!node_bind_pose_transform_initilzed_flags[node_index])
                    {
                        node_bind_pose_transform_initilzed_flags[node_index] = true;

                        DirectX::XMFLOAT4X4 inverse_bind_matrix;
                        {
                            cgltf_bool result_accessor_read_float_inverse_bind_matrix = cgltf_accessor_read_float(skin_inverse_bind_matrix_accessor, joint_index, reinterpret_cast<float *>(&inverse_bind_matrix), 16);
                            assert(result_accessor_read_float_inverse_bind_matrix);
                        }

                        DirectX::XMMATRIX node_bind_pose_transform = DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&inverse_bind_matrix));

                        DirectX::XMVECTOR node_bind_pose_scale;
                        DirectX::XMVECTOR node_bind_pose_rotation;
                        DirectX::XMVECTOR node_bind_pose_translation;
                        DirectX::XMMatrixDecompose(&node_bind_pose_scale, &node_bind_pose_rotation, &node_bind_pose_translation, node_bind_pose_transform);

                        assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(node_bind_pose_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON))));

                        node_bind_pose_rotation = DirectX::XMVector4Normalize(node_bind_pose_rotation);

                        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4 *>(&node_bind_pose_transforms[node_index].m_rotation[0]), node_bind_pose_rotation);
                        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3 *>(&node_bind_pose_transforms[node_index].m_translation[0]), node_bind_pose_translation);
                    }

                    if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != node_parent_indices[node_index])
                    {
                        node_index = node_parent_indices[node_index];
                    }
                    else
                    {
                        root_node_index_in_skeleton.insert(node_index);
                        break;
                    }
                }
            }
        }

        for (cgltf_size node_index = 0U; node_index < data->nodes_count; ++node_index)
        {
            if (!node_bind_pose_transform_initilzed_flags[node_index])
            {
                node_bind_pose_transform_initilzed_flags[node_index] = true;

                DirectX::XMMATRIX node_world_transform = DirectX::XMLoadFloat4x4(&out_internal_node_world_transforms[node_index]);

                DirectX::XMVECTOR node_world_transform_scale;
                DirectX::XMVECTOR node_world_transform_rotation;
                DirectX::XMVECTOR node_world_transform_translation;
                DirectX::XMMatrixDecompose(&node_world_transform_scale, &node_world_transform_rotation, &node_world_transform_translation, node_world_transform);

                assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(node_world_transform_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON))));

                node_world_transform_rotation = DirectX::XMVector4Normalize(node_world_transform_rotation);

                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4 *>(&node_bind_pose_transforms[node_index].m_rotation[0]), node_world_transform_rotation);
                DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3 *>(&node_bind_pose_transforms[node_index].m_translation[0]), node_world_transform_translation);
            }
        }

        if (root_node_index_in_skeleton.size() == 1U)
        {
            root_node_index = (*root_node_index_in_skeleton.begin());
        }
        else
        {
            assert(root_node_index_in_skeleton.size() > 1U);

            root_node_index = BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;
        }
    }

    out_internal_node_index_to_skeleton_joint_index.assign(static_cast<size_t>(data->nodes_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

    // the zero-th joint is always the root
    if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != root_node_index)
    {
        out_skeleton_joint_names.reserve(node_index_in_skeleton.size());
        out_skeleton_joint_parent_indices.reserve(node_index_in_skeleton.size());
        out_skeleton_bind_pose_joint_transforms.reserve(node_index_in_skeleton.size());

        out_skeleton_joint_names.push_back(internal_import_skeleton_joint_name(data, &data->nodes[root_node_index]));

        out_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
        out_internal_node_index_to_skeleton_joint_index[root_node_index] = 0U;

        out_skeleton_bind_pose_joint_transforms.push_back(node_bind_pose_transforms[root_node_index]);
    }
    else
    {
        out_skeleton_joint_names.reserve(node_index_in_skeleton.size() + 1U);
        out_skeleton_joint_parent_indices.reserve(node_index_in_skeleton.size() + 1U);
        out_skeleton_bind_pose_joint_transforms.reserve(node_index_in_skeleton.size() + 1U);

        out_skeleton_joint_names.push_back("GLTF | Internal | Root Node");

        out_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);

        out_skeleton_bind_pose_joint_transforms.push_back({{0.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 0.0F}});
    }

    // use BFS to be more consistent with MMD (PMX)?
    internal_scene_depth_first_search_traverse(
        data,
        [](cgltf_data const *data, cgltf_node const *current_node, cgltf_node const *parent_node, void *user_data_x, void *user_data_y, void *user_data_z, void *user_data_u, void *user_data_v, void *user_data_w, void *user_data_l, void *user_data_m, void *user_data_n) -> void
        {
            mcrt_vector<mcrt_string> &out_skeleton_joint_names = *static_cast<mcrt_vector<mcrt_string> *>(user_data_x);
            mcrt_vector<uint32_t> &out_skeleton_joint_parent_indices = *static_cast<mcrt_vector<uint32_t> *>(user_data_y);
            mcrt_vector<brx_asset_import_rigid_transform> &out_skeleton_bind_pose_joint_transforms = *static_cast<mcrt_vector<brx_asset_import_rigid_transform> *>(user_data_z);
            mcrt_vector<uint32_t> &out_internal_node_index_to_skeleton_joint_index = *static_cast<mcrt_vector<uint32_t> *>(user_data_u);
            mcrt_unordered_set<uint32_t> &node_index_in_skeleton = *static_cast<mcrt_unordered_set<uint32_t> *>(user_data_v);
            mcrt_vector<brx_asset_import_rigid_transform> &node_bind_pose_transforms = *static_cast<mcrt_vector<brx_asset_import_rigid_transform> *>(user_data_w);
            uint32_t const root_node_index = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(user_data_l));
            assert(NULL == user_data_m);
            assert(NULL == user_data_n);

            if ((cgltf_node_index(data, current_node) != root_node_index) && (node_index_in_skeleton.end() != node_index_in_skeleton.find(cgltf_node_index(data, current_node))))
            {
                out_skeleton_joint_names.push_back(internal_import_skeleton_joint_name(data, current_node));

                {
                    uint32_t const parent_joint_index = (NULL != parent_node) ? out_internal_node_index_to_skeleton_joint_index[cgltf_node_index(data, parent_node)] : 0U;
                    assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != parent_joint_index);

                    out_skeleton_joint_parent_indices.push_back(parent_joint_index);
                    uint32_t const current_joint_index = (out_skeleton_joint_parent_indices.size() - 1U);
                    assert(parent_joint_index < current_joint_index);

                    assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_internal_node_index_to_skeleton_joint_index[cgltf_node_index(data, current_node)]);
                    out_internal_node_index_to_skeleton_joint_index[cgltf_node_index(data, current_node)] = current_joint_index;
                }

                out_skeleton_bind_pose_joint_transforms.push_back(node_bind_pose_transforms[cgltf_node_index(data, current_node)]);
            }
        },
        &out_skeleton_joint_names,
        &out_skeleton_joint_parent_indices,
        &out_skeleton_bind_pose_joint_transforms,
        &out_internal_node_index_to_skeleton_joint_index,
        &node_index_in_skeleton,
        &node_bind_pose_transforms,
        reinterpret_cast<void *>(static_cast<uintptr_t>(root_node_index)),
        NULL,
        NULL);

#ifndef NDEBUG
    for (uint32_t skeleton_joint_index = 0U; skeleton_joint_index < out_skeleton_joint_parent_indices.size(); ++skeleton_joint_index)
    {
        assert((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_skeleton_joint_parent_indices[skeleton_joint_index]) || (out_skeleton_joint_parent_indices[skeleton_joint_index] < skeleton_joint_index));
    }
#endif

    assert(out_skeleton_joint_names.size() == ((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != root_node_index) ? node_index_in_skeleton.size() : (node_index_in_skeleton.size() + 1U)));
    assert(out_skeleton_joint_parent_indices.size() == ((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != root_node_index) ? node_index_in_skeleton.size() : (node_index_in_skeleton.size() + 1U)));
    assert(out_skeleton_bind_pose_joint_transforms.size() == ((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != root_node_index) ? node_index_in_skeleton.size() : (node_index_in_skeleton.size() + 1U)));
}

static inline void internal_import_surface(cgltf_data const *data, mcrt_vector<DirectX::XMFLOAT4X4> const &internal_node_world_transforms, mcrt_vector<mcrt_vector<cgltf_node const *>> const &internal_mesh_instances, mcrt_vector<uint32_t> const &internal_node_index_to_skeleton_joint_index, mcrt_vector<brx_asset_import_mesh_surface> &out_surfaces)
{
    // [glTF Validator: NODE_SKINNED_MESH_NON_ROOT](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L444)
    // [glTF Validator: NODE_SKINNED_MESH_LOCAL_TRANSFORMS](https://github.com/KhronosGroup/glTF-Validator/blob/main/lib/src/errors.dart#L450)
    // skin (or skin and morph) mesh can NOT be affected by the node transform (this means the instances of skinned (or skinned and morphed) mesh are meaningless)
    // but the morph (but not skin) mesh can be affected by the node transform

#if 0
    // glTF Validator
    // NODE_SKINNED_MESH_NON_ROOT
    // NODE_SKINNED_MESH_LOCAL_TRANSFORMS
    DirectX::XMVECTOR out_instance_node_world_scale;
    DirectX::XMVECTOR out_instance_node_world_rotation;
    DirectX::XMVECTOR out_instance_node_world_translation;
    DirectX::XMMatrixDecompose(&out_instance_node_world_scale, &out_instance_node_world_rotation, &out_instance_node_world_translation, DirectX::XMLoadFloat4x4(&mesh_instance_node_world_transforms[mesh_instance_index]));

            out_instance_node_world_rotation = DirectX::XMVector4Normalize(out_instance_node_world_rotation);

    // FLT_EPSILON
    constexpr float const scale_epsilon = 1E-5F;
    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(scale_epsilon))));
    constexpr float const rotation_epsilon = 1E-5F;
    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_rotation, DirectX::XMQuaternionIdentity())), DirectX::XMVectorReplicate(rotation_epsilon))));
    constexpr float const translation_epsilon = 1E-5F;
    assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(out_instance_node_world_translation, DirectX::XMVectorZero())), DirectX::XMVectorReplicate(translation_epsilon))));
#endif

    mcrt_vector<brx_asset_import_mesh_surface> immutable_surface;
    mcrt_vector<brx_asset_import_mesh_surface> morph_deform_surfaces;
    mcrt_vector<brx_asset_import_mesh_surface> skin_deform_surfaces;
    mcrt_vector<brx_asset_import_mesh_surface> morph_and_skin_deform_surfaces;

    // std::cout << "Multiple skeletons bound to the same mesh: \"" << " [" << ((NULL != current_node->mesh->name) ? current_node->mesh->name : "Name N/A") << " ]. Only the joint indices of the first skeleton will be used; all others will be ignored." << std::endl;
    for (size_t mesh_index = 0U; mesh_index < data->meshes_count; ++mesh_index)
    {
    }
}

extern void internal_import_skeleton_animation(cgltf_data const *data, cgltf_animation const *animation, uint32_t animation_frame_rate, mcrt_vector<mcrt_string> &out_skeleton_joint_names, mcrt_vector<brx_asset_import_rigid_transform> &out_skeleton_animation_joint_transforms)
{
    assert(NULL != animation);
    assert(out_skeleton_joint_names.empty());
    assert(out_skeleton_animation_joint_transforms.empty());

    uint32_t const skeleton_joint_count = data->nodes_count;

    out_skeleton_joint_names.resize(skeleton_joint_count);

    for (size_t node_index = 0; node_index < data->nodes_count; ++node_index)
    {
        uint32_t const skeleton_joint_index = node_index;

        out_skeleton_joint_names[skeleton_joint_index] = internal_import_skeleton_joint_name(data, &data->nodes[node_index]);
    }

    // Skin
    // https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/main/source/Renderer/shaders/animation.glsl
    // defined(HAS_WEIGHTS_0_VEC4) && defined(HAS_JOINTS_0_VEC4)
    // defined(HAS_WEIGHTS_1_VEC4) && defined(HAS_JOINTS_1_VEC4)
    // size_t const joint_count = skin->joints_count;

    // Due to the hierarchy, we can NOT know which nodes are really influencing the skeleton joints. We have to consider all nodes.
    mcrt_vector<DirectX::XMFLOAT3> node_local_scales(static_cast<size_t>(data->nodes_count));
    mcrt_vector<DirectX::XMFLOAT4> node_local_rotations(static_cast<size_t>(data->nodes_count));
    mcrt_vector<DirectX::XMFLOAT3> node_local_translations(static_cast<size_t>(data->nodes_count));

    for (size_t node_index = 0; node_index < data->nodes_count; ++node_index)
    {
        cgltf_node const *node = &data->nodes[node_index];

        if (node->has_matrix)
        {
            DirectX::XMVECTOR node_local_scale;
            DirectX::XMVECTOR node_local_rotation;
            DirectX::XMVECTOR node_local_translation;
            DirectX::XMMatrixDecompose(&node_local_scale, &node_local_rotation, &node_local_translation, DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 const *>(&node->matrix)));

            node_local_rotation = DirectX::XMVector4Normalize(node_local_rotation);

            DirectX::XMStoreFloat3(&node_local_scales[node_index], node_local_scale);
            DirectX::XMStoreFloat4(&node_local_rotations[node_index], node_local_rotation);
            DirectX::XMStoreFloat3(&node_local_translations[node_index], node_local_translation);
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
    size_t const channel_count = animation->channels_count;

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

    // Frame Rate = 2
    // Max Time = 2
    //
    // Time  0.0 0.5 1.0 1.5 2.0
    // Frame 0   1   2   3   4
    // Frame Count = 2 * 2 + 1 = 5
    //
    // Our Method
    // Time  0.0 0.5 1.0 1.5 2.0
    // Frame    0   1   2   3
    // Frame Count = 2 * 2 = 4

    uint32_t const animation_frame_count = static_cast<uint32_t>(static_cast<double>(animation_frame_rate) * static_cast<double>(animation_max_time));

    out_skeleton_animation_joint_transforms.assign(static_cast<size_t>(skeleton_joint_count) * static_cast<size_t>(animation_frame_count), {{0.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 0.0F}});

    mcrt_vector<float> channel_previous_sample_times(channel_count, 0.0F);
    mcrt_vector<size_t> channel_previous_key_indices(channel_count, 0);
    for (uint32_t animation_frame_index = 0; animation_frame_index < animation_frame_count; ++animation_frame_index)
    {
        float const frame_sample_time = static_cast<float>((0.5 / static_cast<double>(animation_frame_rate)) + (1.0 / static_cast<double>(animation_frame_rate)) * static_cast<double>(animation_frame_index));
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

        for (size_t node_index = 0U; node_index < data->nodes_count; ++node_index)
        {
            uint32_t const skeleton_joint_index = node_index;

            DirectX::XMVECTOR node_animation_scale;
            DirectX::XMVECTOR node_animation_rotation;
            DirectX::XMVECTOR node_animation_translation;
            DirectX::XMMatrixDecompose(&node_animation_scale, &node_animation_rotation, &node_animation_translation, DirectX::XMLoadFloat4x4(&node_world_transforms[node_index]));

            assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(node_animation_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON))));

            node_animation_rotation = DirectX::XMVector4Normalize(node_animation_rotation);

            assert(0.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_rotation[0]);
            assert(0.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_rotation[1]);
            assert(0.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_rotation[2]);
            assert(1.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_rotation[3]);

            DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4 *>(&out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_rotation[0]), node_animation_rotation);

            assert(0.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_translation[0]);
            assert(0.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_translation[1]);
            assert(0.0F == out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_translation[2]);

            DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3 *>(&out_skeleton_animation_joint_transforms[skeleton_joint_count * animation_frame_index + skeleton_joint_index].m_translation[0]), node_animation_translation);
        }
    }
}

static inline mcrt_string internal_import_skeleton_joint_name(cgltf_data const *data, cgltf_node const *node)
{
    if (NULL != node->name)
    {
        cgltf_decode_string(node->name);
        return (mcrt_string("GLTF | Asset | ") + node->name);
    }
    else
    {
        char text_index[] = {"18446744073709551615"};
        std::snprintf(text_index, sizeof(text_index) / sizeof(text_index[0]), "%llu", static_cast<long long unsigned>(cgltf_node_index(data, node)));
        text_index[(sizeof(text_index) / sizeof(text_index[0])) - 1] = '\0';

        return (mcrt_string("GLTF | Empty | Index ") + text_index);
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
        // !!! low performance !!!
        // !!! avoid BFS !!!
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
