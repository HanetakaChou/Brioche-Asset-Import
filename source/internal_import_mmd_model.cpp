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

#include "internal_import_mmd_model.h"
#include "internal_mmd_pmx.h"
#include "internal_mmd_name.h"
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
#include <cstring>
#include <cassert>

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

struct internal_mmd_vertex_t
{
	float m_position[3];
	float m_normal[3];
	float m_texcoord[2];
	uint32_t m_indices[4];
	float m_weights[4];
};

struct internal_mmd_morph_target_vertex_t
{
	float m_position[3];
	float m_texcoord[2];
};

struct internal_mmd_mesh_section_t
{
	mcrt_vector<internal_mmd_vertex_t> m_vertices;

	mcrt_vector<mcrt_vector<internal_mmd_morph_target_vertex_t>> m_morph_targets_vertices;

	mcrt_vector<uint32_t> m_indices;

	bool m_is_double_sided;

	float m_diffuse[4];

	mcrt_string m_texture_path;
};

static inline mmd_pmx_vec3_t internal_transform_translation(mmd_pmx_vec3_t const &v);

static inline mmd_pmx_vec3_t internal_transform_normal(mmd_pmx_vec3_t const &v);

static inline mmd_pmx_vec4_t internal_transform_rotation(mmd_pmx_vec3_t const &v);

static inline mmd_pmx_vec3_t internal_transform_translation_limit(mmd_pmx_vec3_t const &v);

static inline mmd_pmx_vec3_t internal_transform_rotation_limit(mmd_pmx_vec3_t const &v);

static inline mmd_pmx_vec3_t internal_transform_shape_size(mmd_pmx_vec3_t const &v);

static inline float internal_asin(float y, float z);

static inline void internal_import_morph_targets(mcrt_vector<mmd_pmx_vertex_t> const &in_vertices, mcrt_vector<mmd_pmx_face_t> const &in_faces, mcrt_vector<mmd_pmx_material_t> const &in_materials, mcrt_vector<mmd_pmx_morph_t> const &in_mmd_morphs, mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> &out_morph_target_names, mcrt_vector<mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t>> &out_morph_targets_vertices);

static inline void internal_import_animation_skeleton(mcrt_vector<mmd_pmx_bone_t> const &in_mmd_model_nodes, mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_animation_skeleton_joint_names, mcrt_vector<uint32_t> &out_animation_skeleton_joint_parent_indices, mcrt_vector<uint32_t> &out_model_node_to_animation_skeleton_joint_map, mcrt_vector<uint32_t> &out_animation_skeleton_joint_to_model_node_map, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_local_space, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_model_space, mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> &out_animation_skeleton_joint_constraint_names, mcrt_vector<brx_asset_import_skeleton_joint_constraint> &out_animation_skeleton_joint_constraints, mcrt_vector<mcrt_vector<uint32_t>> &out_animation_skeleton_joint_constraints_storages);

static inline void internal_import_ragdoll_physics(mcrt_vector<mmd_pmx_rigid_body_t> const &in_mmd_rigid_bodies, mcrt_vector<mmd_pmx_constraint_t> const &in_mmd_constraints, mcrt_vector<uint32_t> const &in_model_node_to_animation_skeleton_joint_map, mcrt_vector<DirectX::XMFLOAT4X4> const &in_animation_skeleton_bind_pose_model_space, mcrt_vector<brx_asset_import_physics_rigid_body> &out_ragdoll_skeleton_rigid_bodies, mcrt_vector<brx_asset_import_physics_constraint> &out_ragdoll_skeleton_constraints, mcrt_vector<uint32_t> &out_ragdoll_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_animation_to_ragdoll_direct_mapping, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_ragdoll_to_animation_direct_mapping);

static inline void internal_import_mesh_sections(mcrt_vector<mmd_pmx_vertex_t> const &in_vertices, mcrt_vector<mmd_pmx_face_t> const &in_faces, mcrt_vector<mmd_pmx_texture_t> const &in_textures, mcrt_vector<mmd_pmx_material_t> const &in_materials, mcrt_vector<mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t>> const &in_morph_targets_vertices, mcrt_vector<uint32_t> const &in_model_node_to_animation_skeleton_joint_map, mcrt_vector<internal_mmd_mesh_section_t> &out_mesh_sections);

extern bool internal_import_mmd_model(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_mesh_surface_group> &out_surface_groups)
{
	mcrt_vector<brx_asset_import_mesh_surface> surfaces;

	mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> animation_skeleton_joint_names;
	mcrt_vector<uint32_t> animation_skeleton_joint_parent_indices;
	mcrt_vector<brx_asset_import_rigid_transform> animation_skeleton_joint_transforms_bind_pose_local_space;

	mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> animation_skeleton_joint_constraint_names;
	mcrt_vector<brx_asset_import_skeleton_joint_constraint> animation_skeleton_joint_constraints;
	mcrt_vector<mcrt_vector<uint32_t>> animation_skeleton_joint_constraints_storages;

	mcrt_vector<brx_asset_import_physics_rigid_body> ragdoll_skeleton_rigid_bodies;
	mcrt_vector<brx_asset_import_physics_constraint> ragdoll_skeleton_constraints;

	mcrt_vector<brx_asset_import_ragdoll_direct_mapping> animation_to_ragdoll_direct_mappings;
	mcrt_vector<brx_asset_import_ragdoll_direct_mapping> ragdoll_to_animation_direct_mappings;

	{
		mmd_pmx_t mmd_pmx;
		if (internal_unlikely(!internal_data_read_mmd_pmx(data_base, data_size, &mmd_pmx)))
		{
			return false;
		}

		mcrt_vector<uint32_t> model_node_to_animation_skeleton_joint_map;
		mcrt_vector<uint32_t> animation_skeleton_joint_to_model_node_map;
		mcrt_vector<DirectX::XMFLOAT4X4> animation_skeleton_bind_pose_local_space_matrices;
		mcrt_vector<DirectX::XMFLOAT4X4> animation_skeleton_bind_pose_model_space_matrices;
		internal_import_animation_skeleton(mmd_pmx.m_bones, animation_skeleton_joint_names, animation_skeleton_joint_parent_indices, model_node_to_animation_skeleton_joint_map, animation_skeleton_joint_to_model_node_map, animation_skeleton_bind_pose_local_space_matrices, animation_skeleton_bind_pose_model_space_matrices, animation_skeleton_joint_constraint_names, animation_skeleton_joint_constraints, animation_skeleton_joint_constraints_storages);

		{
			uint32_t const animation_skeleton_joint_count = animation_skeleton_bind_pose_local_space_matrices.size();

			animation_skeleton_joint_transforms_bind_pose_local_space.resize(animation_skeleton_joint_count);

			for (uint32_t animation_skeleton_joint_index = 0U; animation_skeleton_joint_index < animation_skeleton_joint_count; ++animation_skeleton_joint_index)
			{
				constexpr float const INTERNAL_SCALE_EPSILON = 9E-5F;

				DirectX::XMVECTOR simd_animation_skeleton_joint_transform_bind_pose_local_space_translation;
				DirectX::XMVECTOR simd_animation_skeleton_joint_transform_bind_pose_local_space_scale;
				DirectX::XMVECTOR simd_animation_skeleton_joint_transform_bind_pose_local_space_rotation;
				bool const directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&simd_animation_skeleton_joint_transform_bind_pose_local_space_scale, &simd_animation_skeleton_joint_transform_bind_pose_local_space_rotation, &simd_animation_skeleton_joint_transform_bind_pose_local_space_translation, DirectX::XMLoadFloat4x4(&animation_skeleton_bind_pose_local_space_matrices[animation_skeleton_joint_index]));
				assert(directx_xm_matrix_decompose);

				assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(simd_animation_skeleton_joint_transform_bind_pose_local_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON))));

				DirectX::XMFLOAT3 animation_skeleton_joint_transform_bind_pose_local_space_translation;
				DirectX::XMStoreFloat3(&animation_skeleton_joint_transform_bind_pose_local_space_translation, simd_animation_skeleton_joint_transform_bind_pose_local_space_translation);

				DirectX::XMFLOAT4 animation_skeleton_joint_transform_bind_pose_local_space_rotation;
				DirectX::XMStoreFloat4(&animation_skeleton_joint_transform_bind_pose_local_space_rotation, simd_animation_skeleton_joint_transform_bind_pose_local_space_rotation);

				animation_skeleton_joint_transforms_bind_pose_local_space[animation_skeleton_joint_index] = brx_asset_import_rigid_transform{{animation_skeleton_joint_transform_bind_pose_local_space_rotation.x, animation_skeleton_joint_transform_bind_pose_local_space_rotation.y, animation_skeleton_joint_transform_bind_pose_local_space_rotation.z, animation_skeleton_joint_transform_bind_pose_local_space_rotation.w}, {animation_skeleton_joint_transform_bind_pose_local_space_translation.x, animation_skeleton_joint_transform_bind_pose_local_space_translation.y, animation_skeleton_joint_transform_bind_pose_local_space_translation.z}};
			}
		}

		mcrt_vector<uint32_t> ragdoll_skeleton_joint_parent_indices;
		internal_import_ragdoll_physics(mmd_pmx.m_rigid_bodies, mmd_pmx.m_constraints, model_node_to_animation_skeleton_joint_map, animation_skeleton_bind_pose_model_space_matrices, ragdoll_skeleton_rigid_bodies, ragdoll_skeleton_constraints, ragdoll_skeleton_joint_parent_indices, animation_to_ragdoll_direct_mappings, ragdoll_to_animation_direct_mappings);

		mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> morph_target_names;
		mcrt_vector<mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t>> morph_targets_vertices;
		internal_import_morph_targets(mmd_pmx.m_vertices, mmd_pmx.m_faces, mmd_pmx.m_materials, mmd_pmx.m_morphs, morph_target_names, morph_targets_vertices);

		mcrt_vector<internal_mmd_mesh_section_t> mesh_sections;
		internal_import_mesh_sections(mmd_pmx.m_vertices, mmd_pmx.m_faces, mmd_pmx.m_textures, mmd_pmx.m_materials, morph_targets_vertices, model_node_to_animation_skeleton_joint_map, mesh_sections);

		{
			surfaces.reserve(mesh_sections.size());

			for (internal_mmd_mesh_section_t const &mesh_section : mesh_sections)
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
				brx_asset_import_vec3 emissive_factor{1.0F, 1.0F, 1.0F};
				mcrt_vector<uint8_t> normal_image_url;
				float normal_scale = 1.0F;
				mcrt_vector<uint8_t> base_color_image_url;
				brx_asset_import_vec4 base_color_factor{1.0F, 1.0F, 1.0F, 1.0F};
				mcrt_vector<uint8_t> metallic_roughness_image_url;
				float metallic_factor = 0.0F;
				float roughness_factor = 1.0F;
				{
					uint32_t const vertex_count = mesh_section.m_vertices.size();

					mcrt_vector<DirectX::XMFLOAT3> raw_positions(static_cast<size_t>(vertex_count));
					mcrt_vector<DirectX::XMFLOAT3> raw_normals(static_cast<size_t>(vertex_count));
					mcrt_vector<DirectX::XMFLOAT2> raw_texcoords(static_cast<size_t>(vertex_count));
					mcrt_vector<DirectX::XMUINT4> raw_joint_indices(static_cast<size_t>(vertex_count));
					mcrt_vector<DirectX::XMFLOAT4> raw_joint_weights(static_cast<size_t>(vertex_count));
					for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
					{
						internal_mmd_vertex_t const &mesh_section_vertex = mesh_section.m_vertices[vertex_index];

						raw_positions[vertex_index] = DirectX::XMFLOAT3(mesh_section_vertex.m_position[0], mesh_section_vertex.m_position[1], mesh_section_vertex.m_position[2]);
						raw_normals[vertex_index] = DirectX::XMFLOAT3(mesh_section_vertex.m_normal[0], mesh_section_vertex.m_normal[1], mesh_section_vertex.m_normal[2]);
						raw_texcoords[vertex_index] = DirectX::XMFLOAT2(mesh_section_vertex.m_texcoord[0], mesh_section_vertex.m_texcoord[1]);
						raw_joint_indices[vertex_index] = DirectX::XMUINT4(mesh_section_vertex.m_indices[0], mesh_section_vertex.m_indices[1], mesh_section_vertex.m_indices[2], mesh_section_vertex.m_indices[3]);
						raw_joint_weights[vertex_index] = DirectX::XMFLOAT4(mesh_section_vertex.m_weights[0], mesh_section_vertex.m_weights[1], mesh_section_vertex.m_weights[2], mesh_section_vertex.m_weights[3]);
					}

					uint32_t const raw_index_count = mesh_section.m_indices.size();

					mcrt_vector<uint32_t> raw_indices(static_cast<size_t>(raw_index_count));
					for (uint32_t index_index = 0U; index_index < raw_index_count; ++index_index)
					{
						raw_indices[index_index] = mesh_section.m_indices[index_index];
					}

					mcrt_vector<DirectX::XMFLOAT4> raw_tangents(static_cast<size_t>(vertex_count));
					{
						assert(0U == (raw_index_count % 3U));
						size_t const raw_face_count = raw_index_count / 3U;

						bool const directx_compute_tangent_frame = DirectX::ComputeTangentFrame(raw_indices.data(), raw_face_count, raw_positions.data(), raw_normals.data(), raw_texcoords.data(), vertex_count, raw_tangents.data());
						assert(directx_compute_tangent_frame);
					}

					{
						vertex_positions.resize(vertex_count);

						for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
						{
							vertex_positions[vertex_index].m_position[0] = raw_positions[vertex_index].x;
							vertex_positions[vertex_index].m_position[1] = raw_positions[vertex_index].y;
							vertex_positions[vertex_index].m_position[2] = raw_positions[vertex_index].z;
						}
					}

					{
						vertex_varyings.resize(vertex_count);

						for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
						{
							vertex_varyings[vertex_index].m_normal[0] = raw_normals[vertex_index].x;
							vertex_varyings[vertex_index].m_normal[1] = raw_normals[vertex_index].y;
							vertex_varyings[vertex_index].m_normal[2] = raw_normals[vertex_index].z;

							vertex_varyings[vertex_index].m_tangent[0] = raw_tangents[vertex_index].x;
							vertex_varyings[vertex_index].m_tangent[1] = raw_tangents[vertex_index].y;
							vertex_varyings[vertex_index].m_tangent[2] = raw_tangents[vertex_index].z;
							vertex_varyings[vertex_index].m_tangent[3] = raw_tangents[vertex_index].w;

							vertex_varyings[vertex_index].m_texcoord[0] = raw_texcoords[vertex_index].x;
							vertex_varyings[vertex_index].m_texcoord[1] = raw_texcoords[vertex_index].y;
						}
					}

					{
						vertex_blendings.resize(vertex_count);

						for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
						{
							vertex_blendings[vertex_index].m_indices[0] = raw_joint_indices[vertex_index].x;
							vertex_blendings[vertex_index].m_indices[1] = raw_joint_indices[vertex_index].y;
							vertex_blendings[vertex_index].m_indices[2] = raw_joint_indices[vertex_index].z;
							vertex_blendings[vertex_index].m_indices[3] = raw_joint_indices[vertex_index].w;

							vertex_blendings[vertex_index].m_weights[0] = raw_joint_weights[vertex_index].x;
							vertex_blendings[vertex_index].m_weights[1] = raw_joint_weights[vertex_index].y;
							vertex_blendings[vertex_index].m_weights[2] = raw_joint_weights[vertex_index].z;
							vertex_blendings[vertex_index].m_weights[3] = raw_joint_weights[vertex_index].w;
						}
					}

					uint32_t const morph_target_count = mesh_section.m_morph_targets_vertices.size();
					assert(0U == morph_target_count || morph_target_names.size() == morph_target_count);

					if (morph_target_count > 0U)
					{
						mesh_section_morph_target_names = morph_target_names;

						morph_targets_vertex_positions.resize(morph_target_count);
						morph_targets_vertex_varyings.resize(morph_target_count);

						for (uint32_t morph_target_index = 0U; morph_target_index < morph_target_count; ++morph_target_index)
						{
							mcrt_vector<internal_mmd_morph_target_vertex_t> const &morph_target_vertices = mesh_section.m_morph_targets_vertices[morph_target_index];
							assert(vertex_count == morph_target_vertices.size());

							mcrt_vector<DirectX::XMFLOAT3> morph_target_raw_positions(static_cast<size_t>(vertex_count));
							mcrt_vector<DirectX::XMFLOAT2> morph_target_raw_texcoords(static_cast<size_t>(vertex_count));
							for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
							{
								internal_mmd_morph_target_vertex_t const &morph_target_vertex = morph_target_vertices[vertex_index];

								morph_target_raw_positions[vertex_index] = DirectX::XMFLOAT3(morph_target_vertex.m_position[0], morph_target_vertex.m_position[1], morph_target_vertex.m_position[2]);
								morph_target_raw_texcoords[vertex_index] = DirectX::XMFLOAT2(morph_target_vertex.m_texcoord[0], morph_target_vertex.m_texcoord[1]);
							}

							{
								mcrt_vector<brx_asset_import_surface_vertex_position> &morph_target_vertex_positions = morph_targets_vertex_positions[morph_target_index];
								morph_target_vertex_positions.resize(vertex_count);

								for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
								{
									morph_target_vertex_positions[vertex_index].m_position[0] = morph_target_raw_positions[vertex_index].x;
									morph_target_vertex_positions[vertex_index].m_position[1] = morph_target_raw_positions[vertex_index].y;
									morph_target_vertex_positions[vertex_index].m_position[2] = morph_target_raw_positions[vertex_index].z;
								}
							}

							{
								mcrt_vector<brx_asset_import_surface_vertex_varying> &morph_target_vertex_varyings = morph_targets_vertex_varyings[morph_target_index];
								morph_target_vertex_varyings.resize(vertex_count);

								for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
								{
									morph_target_vertex_varyings[vertex_index].m_normal[0] = 0.0F;
									morph_target_vertex_varyings[vertex_index].m_normal[1] = 0.0F;
									morph_target_vertex_varyings[vertex_index].m_normal[2] = 0.0F;

									morph_target_vertex_varyings[vertex_index].m_tangent[0] = 0.0F;
									morph_target_vertex_varyings[vertex_index].m_tangent[1] = 0.0F;
									morph_target_vertex_varyings[vertex_index].m_tangent[2] = 0.0F;
									morph_target_vertex_varyings[vertex_index].m_tangent[3] = 0.0F;

									morph_target_vertex_varyings[vertex_index].m_texcoord[0] = morph_target_raw_texcoords[vertex_index].x;
									morph_target_vertex_varyings[vertex_index].m_texcoord[1] = morph_target_raw_texcoords[vertex_index].y;
								}
							}
						}
					}
					else
					{
						assert(mesh_section_morph_target_names.empty());
						assert(morph_targets_vertex_positions.empty());
						assert(morph_targets_vertex_varyings.empty());
					}

					{
						uint32_t const index_count = raw_index_count;
						indices.resize(index_count);

						for (uint32_t index_index = 0U; index_index < index_count; ++index_index)
						{
							indices[index_index] = raw_indices[index_index];
						}
					}

					is_double_sided = mesh_section.m_is_double_sided;

					size_t const base_name_end_pos = mesh_section.m_texture_path.find_last_of('.');
					if ((mcrt_string::npos != base_name_end_pos) && (base_name_end_pos >= 2U) && ('e' == mesh_section.m_texture_path[base_name_end_pos - 1U]) && ('_' == mesh_section.m_texture_path[base_name_end_pos - 2U]))
					{
						{
							mcrt_string texture_url;
							texture_url += "file://";
							texture_url += mesh_section.m_texture_path;

							static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(emissive_image_url[0])), "");
							emissive_image_url.resize(texture_url.length() + 1U);
							std::memcpy(emissive_image_url.data(), texture_url.data(), texture_url.length());
							emissive_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');

							emissive_factor.m_x = mesh_section.m_diffuse[0];
							emissive_factor.m_y = mesh_section.m_diffuse[1];
							emissive_factor.m_z = mesh_section.m_diffuse[2];
							assert(1.0F == mesh_section.m_diffuse[3]);
						}

						{
							assert(base_color_image_url.empty());

							base_color_factor.m_x = 0.0F;
							base_color_factor.m_y = 0.0F;
							base_color_factor.m_z = 0.0F;
							base_color_factor.m_w = 1.0F;
						}

						{
							assert(metallic_roughness_image_url.empty());

							metallic_factor = 0.0F;
							roughness_factor = 1.0F;
						}

						{
							mcrt_string texture_url;
							texture_url += "file://";
							texture_url += mesh_section.m_texture_path.substr(0U, (base_name_end_pos - 2U));
							texture_url += "_n";
							texture_url += mesh_section.m_texture_path.substr(base_name_end_pos);

							static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(normal_image_url[0])), "");
							normal_image_url.resize(texture_url.length() + 1U);
							std::memcpy(normal_image_url.data(), texture_url.data(), texture_url.length());
							normal_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');
						}
					}
					else if ((mcrt_string::npos != base_name_end_pos) && (base_name_end_pos >= 2U) && ('d' == mesh_section.m_texture_path[base_name_end_pos - 1U]) && ('_' == mesh_section.m_texture_path[base_name_end_pos - 2U]))
					{
						{
							mcrt_string texture_url;
							texture_url += "file://";
							texture_url += mesh_section.m_texture_path;

							static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(base_color_image_url[0])), "");
							base_color_image_url.resize(texture_url.length() + 1U);
							std::memcpy(base_color_image_url.data(), texture_url.data(), texture_url.length());
							base_color_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');

							base_color_factor.m_x = mesh_section.m_diffuse[0];
							base_color_factor.m_y = mesh_section.m_diffuse[1];
							base_color_factor.m_z = mesh_section.m_diffuse[2];
							base_color_factor.m_w = mesh_section.m_diffuse[3];
						}

						{
							assert(emissive_image_url.empty());

							emissive_factor.m_x = 0.0F;
							emissive_factor.m_y = 0.0F;
							emissive_factor.m_z = 0.0F;
						}

						{
							mcrt_string texture_url;
							texture_url += "file://";
							texture_url += mesh_section.m_texture_path.substr(0U, (base_name_end_pos - 2U));
							texture_url += "_s";
							texture_url += mesh_section.m_texture_path.substr(base_name_end_pos);

							static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(metallic_roughness_image_url[0])), "");
							metallic_roughness_image_url.resize(texture_url.length() + 1U);
							std::memcpy(metallic_roughness_image_url.data(), texture_url.data(), texture_url.length());
							metallic_roughness_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');

							metallic_factor = 1.0F;
							roughness_factor = 1.0F;
						}

						{
							mcrt_string texture_url;
							texture_url += "file://";
							texture_url += mesh_section.m_texture_path.substr(0U, (base_name_end_pos - 2U));
							texture_url += "_n";
							texture_url += mesh_section.m_texture_path.substr(base_name_end_pos);

							static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(normal_image_url[0])), "");
							normal_image_url.resize(texture_url.length() + 1U);
							std::memcpy(normal_image_url.data(), texture_url.data(), texture_url.length());
							normal_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');
						}
					}
					else
					{
						{
							if (!mesh_section.m_texture_path.empty())
							{
								mcrt_string texture_url;
								texture_url += "file://";
								texture_url += mesh_section.m_texture_path;

								static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(base_color_image_url[0])), "");
								base_color_image_url.resize(texture_url.length() + 1U);
								std::memcpy(base_color_image_url.data(), texture_url.data(), texture_url.length());
								base_color_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');
							}
							else
							{
								assert(base_color_image_url.empty());
							}

							base_color_factor.m_x = mesh_section.m_diffuse[0];
							base_color_factor.m_y = mesh_section.m_diffuse[1];
							base_color_factor.m_z = mesh_section.m_diffuse[2];
							base_color_factor.m_w = mesh_section.m_diffuse[3];
						}

						{
							assert(emissive_image_url.empty());

							emissive_factor.m_x = 0.0F;
							emissive_factor.m_y = 0.0F;
							emissive_factor.m_z = 0.0F;
						}

						{
							assert(metallic_roughness_image_url.empty());

							metallic_factor = 0.0F;
							roughness_factor = 1.0F;
						}

						if (!mesh_section.m_texture_path.empty())
						{
							if (mcrt_string::npos != base_name_end_pos)
							{
								mcrt_string texture_url;
								texture_url += "file://";
								texture_url += mesh_section.m_texture_path.substr(0U, base_name_end_pos);
								texture_url += "_n";
								texture_url += mesh_section.m_texture_path.substr(base_name_end_pos);

								static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(normal_image_url[0])), "");
								normal_image_url.resize(texture_url.length() + 1U);
								std::memcpy(normal_image_url.data(), texture_url.data(), texture_url.length());
								normal_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');
							}
							else
							{
								mcrt_string texture_url;
								texture_url += "file://";
								texture_url += mesh_section.m_texture_path;
								texture_url += "_n";

								static_assert(sizeof(decltype(texture_url[0])) == sizeof(decltype(normal_image_url[0])), "");
								normal_image_url.resize(texture_url.length() + 1U);
								std::memcpy(normal_image_url.data(), texture_url.data(), texture_url.length());
								normal_image_url.data()[texture_url.length()] = static_cast<uint8_t>('\0');
							}
						}
						else
						{
							assert(normal_image_url.empty());
						}
					}
				}

				surfaces.emplace_back(std::move(vertex_positions), std::move(vertex_varyings), std::move(vertex_blendings), std::move(mesh_section_morph_target_names), std::move(morph_targets_vertex_positions), std::move(morph_targets_vertex_varyings), std::move(indices), is_double_sided, std::move(emissive_image_url), emissive_factor, std::move(normal_image_url), normal_scale, std::move(base_color_image_url), base_color_factor, std::move(metallic_roughness_image_url), metallic_factor, roughness_factor);
			}

			assert(surfaces.size() == mesh_sections.size());
		}
	}

	assert(out_surface_groups.empty());
	out_surface_groups = {};

	out_surface_groups.reserve(1U);

	out_surface_groups.emplace_back(std::move(surfaces), std::move(animation_skeleton_joint_names), std::move(animation_skeleton_joint_parent_indices), std::move(animation_skeleton_joint_transforms_bind_pose_local_space), std::move(animation_skeleton_joint_constraint_names), std::move(animation_skeleton_joint_constraints), std::move(animation_skeleton_joint_constraints_storages), std::move(ragdoll_skeleton_rigid_bodies), std::move(ragdoll_skeleton_constraints), std::move(animation_to_ragdoll_direct_mappings), std::move(ragdoll_to_animation_direct_mappings));

	return true;
}

static inline void internal_import_morph_targets(mcrt_vector<mmd_pmx_vertex_t> const &in_vertices, mcrt_vector<mmd_pmx_face_t> const &in_faces, mcrt_vector<mmd_pmx_material_t> const &in_materials, mcrt_vector<mmd_pmx_morph_t> const &in_mmd_morphs, mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> &out_morph_target_names, mcrt_vector<mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t>> &out_morph_targets_vertices)
{
	mcrt_unordered_map<mcrt_string, mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t>> mmd_morph_targets;
	{
		// DAG
		// Topological Sort

		uint32_t const mmd_morph_count = in_mmd_morphs.size();

		mcrt_vector<uint32_t> mmd_morph_parent_count(static_cast<size_t>(mmd_morph_count), 0U);
		mcrt_vector<mcrt_vector<uint32_t>> mmd_morph_children_indices(static_cast<size_t>(mmd_morph_count));
		mcrt_vector<mcrt_vector<float>> mmd_morph_children_weights(static_cast<size_t>(mmd_morph_count));
		for (size_t mmd_morph_index = 0U; mmd_morph_index < mmd_morph_count; ++mmd_morph_index)
		{
			if (0U == in_mmd_morphs[mmd_morph_index].m_morph_type)
			{
				uint32_t const child_index = mmd_morph_index;
				uint32_t const mmd_morph_offset_count = in_mmd_morphs[mmd_morph_index].m_offsets.size();
				for (size_t mmd_morph_offset_index = 0U; mmd_morph_offset_index < mmd_morph_offset_count; ++mmd_morph_offset_index)
				{
					uint32_t const parent_index = in_mmd_morphs[mmd_morph_index].m_offsets[mmd_morph_offset_index].m_group.m_morph_index;
					++mmd_morph_parent_count[child_index];
					mmd_morph_children_indices[parent_index].push_back(child_index);
					mmd_morph_children_weights[parent_index].push_back(in_mmd_morphs[mmd_morph_index].m_offsets[mmd_morph_offset_index].m_group.m_morph_weight);
				}
			}
		}

		mcrt_vector<uint32_t> topological_sort_stack;
		for (uint32_t mmd_morph_index_plus_1 = mmd_morph_count; mmd_morph_index_plus_1 > 0U; --mmd_morph_index_plus_1)
		{
			uint32_t const mmd_morph_index = mmd_morph_index_plus_1 - 1U;
			assert(mmd_morph_children_indices[mmd_morph_index].size() == mmd_morph_children_weights[mmd_morph_index].size());
			if (0U == mmd_morph_parent_count[mmd_morph_index])
			{
				topological_sort_stack.push_back(mmd_morph_index);
			}
		}

		mcrt_vector<bool> mmd_morph_visited_flags(static_cast<size_t>(mmd_morph_count), false);
		while (!topological_sort_stack.empty())
		{
			uint32_t const mmd_morph_current_index = topological_sort_stack.back();
			topological_sort_stack.pop_back();

			assert(!mmd_morph_visited_flags[mmd_morph_current_index]);
			mmd_morph_visited_flags[mmd_morph_current_index] = true;

			mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t> &current_morph_target = mmd_morph_targets[in_mmd_morphs[mmd_morph_current_index].m_name];

			if (1U == in_mmd_morphs[mmd_morph_current_index].m_morph_type || 2U == in_mmd_morphs[mmd_morph_current_index].m_morph_type)
			{
				// TODO: duplicated morph name
				// assert(current_morph_target.empty());

				uint32_t const mmd_morph_offset_count = in_mmd_morphs[mmd_morph_current_index].m_offsets.size();
				for (size_t mmd_morph_offset_index = 0U; mmd_morph_offset_index < mmd_morph_offset_count; ++mmd_morph_offset_index)
				{
					internal_mmd_morph_target_vertex_t morph_target_vertex;
					if (1U == in_mmd_morphs[mmd_morph_current_index].m_morph_type)
					{
						mmd_pmx_vec3_t const mmd_morph_vertex_position = internal_transform_translation(in_mmd_morphs[mmd_morph_current_index].m_offsets[mmd_morph_offset_index].m_vertex_position.m_vertex_position);

						morph_target_vertex = internal_mmd_morph_target_vertex_t{{mmd_morph_vertex_position.m_x, mmd_morph_vertex_position.m_y, mmd_morph_vertex_position.m_z}, {0.0F, 0.0F}};
					}
					else
					{
						assert(2U == in_mmd_morphs[mmd_morph_current_index].m_morph_type);

						mmd_pmx_vec2_t const in_vertex_uv = in_mmd_morphs[mmd_morph_current_index].m_offsets[mmd_morph_offset_index].m_vertex_uv.m_vertex_uv;

						morph_target_vertex = internal_mmd_morph_target_vertex_t{{0.0F, 0.0F, 0.0F}, {in_vertex_uv.m_x, in_vertex_uv.m_y}};
					}

					uint32_t const vertex_index = in_mmd_morphs[mmd_morph_current_index].m_offsets[mmd_morph_offset_index].m_vertex_position.m_vertex_index;

					auto found_vertex_index = current_morph_target.find(vertex_index);
					if (current_morph_target.end() == found_vertex_index)
					{
						current_morph_target.emplace_hint(found_vertex_index, vertex_index, morph_target_vertex);
					}
					else
					{
						assert(current_morph_target[vertex_index].m_position[0] == morph_target_vertex.m_position[0]);
						assert(current_morph_target[vertex_index].m_position[1] == morph_target_vertex.m_position[1]);
						assert(current_morph_target[vertex_index].m_position[2] == morph_target_vertex.m_position[2]);
						assert(current_morph_target[vertex_index].m_texcoord[0] == morph_target_vertex.m_texcoord[0]);
						assert(current_morph_target[vertex_index].m_texcoord[1] == morph_target_vertex.m_texcoord[1]);
					}
				}
			}
			else if (0U == in_mmd_morphs[mmd_morph_current_index].m_morph_type)
			{
#ifndef NDEBUG
				if (current_morph_target.empty())
				{
					uint32_t const mmd_morph_offset_count = in_mmd_morphs[mmd_morph_current_index].m_offsets.size();
					for (size_t mmd_morph_offset_index = 0U; mmd_morph_offset_index < mmd_morph_offset_count; ++mmd_morph_offset_index)
					{
						uint32_t const mmd_morph_parent_index = in_mmd_morphs[mmd_morph_current_index].m_offsets[mmd_morph_offset_index].m_group.m_morph_index;
						assert(mmd_morph_targets[in_mmd_morphs[mmd_morph_parent_index].m_name].empty());
					}
				}
#endif
			}
			else
			{
				assert(3U == in_mmd_morphs[mmd_morph_current_index].m_morph_type);
			}

			assert(mmd_morph_children_indices[mmd_morph_current_index].size() == mmd_morph_children_weights[mmd_morph_current_index].size());
			for (uint32_t mmd_morph_child_index_index_plus_1 = static_cast<uint32_t>(mmd_morph_children_indices[mmd_morph_current_index].size()); mmd_morph_child_index_index_plus_1 > 0U; --mmd_morph_child_index_index_plus_1)
			{
				uint32_t const mmd_morph_child_index = mmd_morph_children_indices[mmd_morph_current_index][mmd_morph_child_index_index_plus_1 - 1U];
				float const mmd_morph_child_weight = mmd_morph_children_weights[mmd_morph_current_index][mmd_morph_child_index_index_plus_1 - 1U];

				mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t> &child_morph_target = mmd_morph_targets[in_mmd_morphs[mmd_morph_child_index].m_name];

				for (auto const &vertex_index_and_morph_target_vertex : current_morph_target)
				{
					uint32_t const vertex_index = vertex_index_and_morph_target_vertex.first;
					internal_mmd_morph_target_vertex_t const morph_target_vertex{
						{
							vertex_index_and_morph_target_vertex.second.m_position[0] * mmd_morph_child_weight,
							vertex_index_and_morph_target_vertex.second.m_position[1] * mmd_morph_child_weight,
							vertex_index_and_morph_target_vertex.second.m_position[2] * mmd_morph_child_weight,
						},
						{
							vertex_index_and_morph_target_vertex.second.m_texcoord[0] * mmd_morph_child_weight,
							vertex_index_and_morph_target_vertex.second.m_texcoord[1] * mmd_morph_child_weight,
						}};

					auto found_vertex_index = child_morph_target.find(vertex_index);
					if (child_morph_target.end() == found_vertex_index)
					{
						child_morph_target.emplace_hint(found_vertex_index, vertex_index, morph_target_vertex);
					}
					else
					{
						found_vertex_index->second.m_position[0] += morph_target_vertex.m_position[0];
						found_vertex_index->second.m_position[1] += morph_target_vertex.m_position[1];
						found_vertex_index->second.m_position[2] += morph_target_vertex.m_position[2];
						found_vertex_index->second.m_texcoord[0] += morph_target_vertex.m_texcoord[0];
						found_vertex_index->second.m_texcoord[1] += morph_target_vertex.m_texcoord[1];
					}
				}

				assert(mmd_morph_parent_count[mmd_morph_child_index] > 0U);
				--mmd_morph_parent_count[mmd_morph_child_index];
				if (0U == mmd_morph_parent_count[mmd_morph_child_index])
				{
					topological_sort_stack.push_back(mmd_morph_child_index);
				}
			}
		}

#ifndef NDEBUG
		for (bool mmd_morph_visited_flag : mmd_morph_visited_flags)
		{
			assert(mmd_morph_visited_flag);
		}
#endif
	}

	mcrt_vector<mcrt_vector<mcrt_string>> mmd_morph_target_name_strings(static_cast<size_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT));
	internal_fill_mmd_morph_target_name_strings(mmd_morph_target_name_strings);

	assert(out_morph_target_names.empty());
	out_morph_target_names = {};

	assert(out_morph_targets_vertices.empty());
	out_morph_targets_vertices = {};

	for (uint32_t mmd_morph_target_name = 0U; mmd_morph_target_name < BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT; ++mmd_morph_target_name)
	{
		for (mcrt_string const &mmd_morph_target_name_string : mmd_morph_target_name_strings[mmd_morph_target_name])
		{
			auto const found_mmd_name_and_morph_target = mmd_morph_targets.find(mmd_morph_target_name_string);
			if (mmd_morph_targets.end() != found_mmd_name_and_morph_target)
			{
				if (!found_mmd_name_and_morph_target->second.empty())
				{
					out_morph_target_names.push_back(static_cast<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>(mmd_morph_target_name));
					out_morph_targets_vertices.push_back(std::move(found_mmd_name_and_morph_target->second));
					break;
				}
				else
				{
					// TODO: support bone morph
					// assert(false);
				}
			}
		}
	}
}

static inline void internal_import_animation_skeleton(mcrt_vector<mmd_pmx_bone_t> const &in_mmd_model_nodes, mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> &out_animation_skeleton_joint_names, mcrt_vector<uint32_t> &out_animation_skeleton_joint_parent_indices, mcrt_vector<uint32_t> &out_model_node_to_animation_skeleton_joint_map, mcrt_vector<uint32_t> &out_animation_skeleton_joint_to_model_node_map, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_local_space, mcrt_vector<DirectX::XMFLOAT4X4> &out_animation_skeleton_bind_pose_model_space, mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> &out_animation_skeleton_joint_constraint_names, mcrt_vector<brx_asset_import_skeleton_joint_constraint> &out_animation_skeleton_joint_constraints, mcrt_vector<mcrt_vector<uint32_t>> &out_animation_skeleton_joint_constraints_storages)
{
	uint32_t const model_node_count = in_mmd_model_nodes.size();

	assert(out_animation_skeleton_joint_parent_indices.empty());
	out_animation_skeleton_joint_parent_indices = {};
	assert(out_model_node_to_animation_skeleton_joint_map.empty());
	out_model_node_to_animation_skeleton_joint_map = mcrt_vector<uint32_t>(static_cast<size_t>(model_node_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
	assert(out_animation_skeleton_joint_to_model_node_map.empty());
	out_animation_skeleton_joint_to_model_node_map = mcrt_vector<uint32_t>(static_cast<size_t>(model_node_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
	{
		mcrt_vector<uint32_t> model_node_parent_indices(static_cast<size_t>(model_node_count));
		for (size_t model_node_index = 0U; model_node_index < model_node_count; ++model_node_index)
		{
			model_node_parent_indices[model_node_index] = in_mmd_model_nodes[model_node_index].m_parent_index;
		}

		mcrt_vector<uint32_t> model_node_depth_first_search_stack;
		mcrt_vector<mcrt_vector<uint32_t>> model_node_children_indices(static_cast<size_t>(model_node_count));
		{
			mcrt_vector<uint32_t> model_node_depth_first_search_reverse_stack;
			for (uint32_t model_node_index = 0U; model_node_index < model_node_count; ++model_node_index)
			{
				uint32_t model_node_parent_index = model_node_parent_indices[model_node_index];
				if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != model_node_parent_index)
				{
					model_node_children_indices[model_node_parent_index].push_back(model_node_index);
				}
				else
				{
					model_node_depth_first_search_reverse_stack.push_back(model_node_index);
				}
			}
			assert(!model_node_depth_first_search_reverse_stack.empty());

			model_node_depth_first_search_stack.reserve(model_node_depth_first_search_reverse_stack.size());
			for (auto model_node_depth_first_search_reverse_stack_index = model_node_depth_first_search_reverse_stack.rbegin(); model_node_depth_first_search_reverse_stack_index != model_node_depth_first_search_reverse_stack.rend(); ++model_node_depth_first_search_reverse_stack_index)
			{
				uint32_t const model_root_node_index = (*model_node_depth_first_search_reverse_stack_index);
				model_node_depth_first_search_stack.push_back(model_root_node_index);
			}
		}

		mcrt_vector<bool> model_node_visited_flags(static_cast<size_t>(model_node_count), false);
		mcrt_vector<bool> model_node_pushed_flags(static_cast<size_t>(model_node_count), false);
		while (!model_node_depth_first_search_stack.empty())
		{
			uint32_t const model_node_current_index = model_node_depth_first_search_stack.back();
			model_node_depth_first_search_stack.pop_back();

			assert(!model_node_visited_flags[model_node_current_index]);
			model_node_visited_flags[model_node_current_index] = true;

			uint32_t const animation_skeleton_joint_current_index = out_animation_skeleton_joint_parent_indices.size();

			uint32_t const model_node_parent_index = model_node_parent_indices[model_node_current_index];

			if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == model_node_parent_index)
			{
				out_animation_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
			}
			else
			{
				assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != out_model_node_to_animation_skeleton_joint_map[model_node_parent_index]);
				out_animation_skeleton_joint_parent_indices.push_back(out_model_node_to_animation_skeleton_joint_map[model_node_parent_index]);
			}

			assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_animation_skeleton_joint_to_model_node_map[animation_skeleton_joint_current_index]);
			out_animation_skeleton_joint_to_model_node_map[animation_skeleton_joint_current_index] = model_node_current_index;

			assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == out_model_node_to_animation_skeleton_joint_map[model_node_current_index]);
			out_model_node_to_animation_skeleton_joint_map[model_node_current_index] = animation_skeleton_joint_current_index;

			for (uint32_t model_node_child_index_index_plus_1 = static_cast<uint32_t>(model_node_children_indices[model_node_current_index].size()); model_node_child_index_index_plus_1 > 0U; --model_node_child_index_index_plus_1)
			{
				uint32_t const model_node_child_index = model_node_children_indices[model_node_current_index][model_node_child_index_index_plus_1 - 1U];

				if ((!model_node_visited_flags[model_node_child_index]) && (!model_node_pushed_flags[model_node_child_index]))
				{
					model_node_pushed_flags[model_node_child_index] = true;
					model_node_depth_first_search_stack.push_back(model_node_child_index);
				}
				else
				{
					assert(false);
				}
			}
		}

		assert(out_animation_skeleton_joint_parent_indices.size() == model_node_count);
	}

	assert(out_animation_skeleton_joint_names.empty());
	out_animation_skeleton_joint_names = {};
	{
		out_animation_skeleton_joint_names.resize(model_node_count);

		mcrt_unordered_map<mcrt_string, uint32_t> mmd_skeleton_joint_indices;

		for (uint32_t current_animation_skeleton_joint_index = 0; current_animation_skeleton_joint_index < model_node_count; ++current_animation_skeleton_joint_index)
		{
			out_animation_skeleton_joint_names[current_animation_skeleton_joint_index] = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID;

			uint32_t const current_model_node_index = out_animation_skeleton_joint_to_model_node_map[current_animation_skeleton_joint_index];

			mcrt_string const &current_model_node_name = in_mmd_model_nodes[current_model_node_index].m_name;

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

	assert(out_animation_skeleton_bind_pose_local_space.empty());
	out_animation_skeleton_bind_pose_local_space = mcrt_vector<DirectX::XMFLOAT4X4>(static_cast<size_t>(model_node_count));
	assert(out_animation_skeleton_bind_pose_model_space.empty());
	out_animation_skeleton_bind_pose_model_space = mcrt_vector<DirectX::XMFLOAT4X4>(static_cast<size_t>(model_node_count));
	{

		for (size_t current_animation_skeleton_joint_index = 0; current_animation_skeleton_joint_index < model_node_count; ++current_animation_skeleton_joint_index)
		{
			uint32_t const current_model_node_index = out_animation_skeleton_joint_to_model_node_map[current_animation_skeleton_joint_index];

			{
				DirectX::XMFLOAT3 node_translation_model_space;
				{
					mmd_pmx_vec3_t const mmd_model_node_translation = internal_transform_translation(in_mmd_model_nodes[current_model_node_index].m_translation);

					node_translation_model_space = DirectX::XMFLOAT3(mmd_model_node_translation.m_x, mmd_model_node_translation.m_y, mmd_model_node_translation.m_z);
				}

				DirectX::XMStoreFloat4x4(&out_animation_skeleton_bind_pose_model_space[current_animation_skeleton_joint_index], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&node_translation_model_space)));
			}

			uint32_t const parent_animation_skeleton_joint_index = out_animation_skeleton_joint_parent_indices[current_animation_skeleton_joint_index];
			if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != parent_animation_skeleton_joint_index)
			{
				assert(parent_animation_skeleton_joint_index < current_animation_skeleton_joint_index);

				DirectX::XMVECTOR unused_determinant;
				DirectX::XMStoreFloat4x4(&out_animation_skeleton_bind_pose_local_space[current_animation_skeleton_joint_index], DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&out_animation_skeleton_bind_pose_model_space[current_animation_skeleton_joint_index]), DirectX::XMMatrixInverse(&unused_determinant, DirectX::XMLoadFloat4x4(&out_animation_skeleton_bind_pose_model_space[parent_animation_skeleton_joint_index]))));
			}
			else
			{
				out_animation_skeleton_bind_pose_local_space[current_animation_skeleton_joint_index] = out_animation_skeleton_bind_pose_model_space[current_animation_skeleton_joint_index];
			}
		}
	}

	assert(out_animation_skeleton_joint_constraint_names.empty());
	out_animation_skeleton_joint_constraint_names = {};
	assert(out_animation_skeleton_joint_constraints.empty());
	out_animation_skeleton_joint_constraints = {};
	assert(out_animation_skeleton_joint_constraints_storages.empty());
	out_animation_skeleton_joint_constraints_storages = {};
	{
		mcrt_vector<uint32_t> sorted_model_node_indices(static_cast<size_t>(model_node_count));
		for (uint32_t model_node_index = 0U; model_node_index < model_node_count; ++model_node_index)
		{
			sorted_model_node_indices[model_node_index] = model_node_index;
		}

		std::stable_sort(sorted_model_node_indices.begin(), sorted_model_node_indices.end(),
						 [&in_mmd_model_nodes](uint32_t x, uint32_t y)
						 {
							 if ((in_mmd_model_nodes[x].m_meta_physics && in_mmd_model_nodes[y].m_meta_physics) || ((!in_mmd_model_nodes[x].m_meta_physics) && (!in_mmd_model_nodes[y].m_meta_physics)))
							 {
								 return in_mmd_model_nodes[x].m_transformation_hierarchy < in_mmd_model_nodes[y].m_transformation_hierarchy;
							 }
							 else if ((!in_mmd_model_nodes[x].m_meta_physics) && in_mmd_model_nodes[y].m_meta_physics)
							 {
								 return true;
							 }
							 else
							 {
								 assert(in_mmd_model_nodes[x].m_meta_physics && (!in_mmd_model_nodes[y].m_meta_physics));
								 return false;
							 }
						 });

		constexpr uint32_t mmd_skeleton_joint_constraint_name_count = BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_COUNT;

		mcrt_vector<mcrt_vector<mcrt_string>> mmd_skeleton_joint_constraint_name_strings(static_cast<size_t>(mmd_skeleton_joint_constraint_name_count));
		internal_fill_mmd_skeleton_joint_constraint_name_strings(mmd_skeleton_joint_constraint_name_strings);

		for (uint32_t sorted_model_node_index = 0U; sorted_model_node_index < model_node_count; ++sorted_model_node_index)
		{
			uint32_t const model_node_index = sorted_model_node_indices[sorted_model_node_index];

			if (in_mmd_model_nodes[model_node_index].m_append_rotation || in_mmd_model_nodes[model_node_index].m_append_translation)
			{
				{
					out_animation_skeleton_joint_constraint_names.push_back(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_INVALID);
				}

				{
					brx_asset_import_skeleton_joint_constraint animation_skeleton_joint_copy_transform_constraint;
					animation_skeleton_joint_copy_transform_constraint.m_constraint_type = BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_COPY_TRANSFORM;
					animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_copy_rotation = in_mmd_model_nodes[model_node_index].m_append_rotation;
					animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_copy_translation = in_mmd_model_nodes[model_node_index].m_append_translation;
					animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_destination_joint_index = out_model_node_to_animation_skeleton_joint_map[model_node_index];

					if (!in_mmd_model_nodes[model_node_index].m_append_local)
					{
						mcrt_vector<uint32_t> ancestors;
						ancestors.push_back(model_node_index);
						for (uint32_t current_model_node_index = ((in_mmd_model_nodes[model_node_index].m_append_rotation || in_mmd_model_nodes[model_node_index].m_append_translation) ? in_mmd_model_nodes[model_node_index].m_append_parent_index : BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
							 BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != current_model_node_index;
							 current_model_node_index = (((in_mmd_model_nodes[current_model_node_index].m_append_rotation || in_mmd_model_nodes[current_model_node_index].m_append_translation) && (current_model_node_index != in_mmd_model_nodes[current_model_node_index].m_append_parent_index)) ? in_mmd_model_nodes[current_model_node_index].m_append_parent_index : BRX_ASSET_IMPORT_UINT32_INDEX_INVALID))
						{
							ancestors.push_back(current_model_node_index);
						}

						assert(!ancestors.empty());

						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_joint_index = out_model_node_to_animation_skeleton_joint_map[ancestors.back()];

						ancestors.pop_back();

						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weight_count = ancestors.size();

						out_animation_skeleton_joint_constraints_storages.emplace_back();
						out_animation_skeleton_joint_constraints_storages.back().resize(animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weight_count);
						static_assert(sizeof(float) == sizeof(uint32_t), "");
						static_assert(alignof(float) == alignof(uint32_t), "");
						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weights = reinterpret_cast<float *>(out_animation_skeleton_joint_constraints_storages.back().data());

						for (uint32_t source_weight_index = 0U; !ancestors.empty(); ++source_weight_index)
						{
							animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weights[source_weight_index] = in_mmd_model_nodes[ancestors.back()].m_append_rate;
							ancestors.pop_back();
						}
					}
					else
					{
						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_joint_index = out_model_node_to_animation_skeleton_joint_map[in_mmd_model_nodes[model_node_index].m_append_parent_index];
						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weight_count = 1U;

						out_animation_skeleton_joint_constraints_storages.emplace_back();
						out_animation_skeleton_joint_constraints_storages.back().resize(animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weight_count);
						static_assert(sizeof(float) == sizeof(uint32_t), "");
						static_assert(alignof(float) == alignof(uint32_t), "");
						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weights = reinterpret_cast<float *>(out_animation_skeleton_joint_constraints_storages.back().data());

						animation_skeleton_joint_copy_transform_constraint.m_copy_transform.m_source_weights[0] = in_mmd_model_nodes[model_node_index].m_append_rate;
					}

					out_animation_skeleton_joint_constraints.push_back(animation_skeleton_joint_copy_transform_constraint);
				}
			}

			if (in_mmd_model_nodes[model_node_index].m_ik)
			{
				{
					bool found_skeleton_joint_constraint_name = false;
					for (uint32_t mmd_skeleton_joint_constraint_name = 0U; mmd_skeleton_joint_constraint_name < mmd_skeleton_joint_constraint_name_count; ++mmd_skeleton_joint_constraint_name)
					{
						for (mcrt_string const &mmd_skeleton_joint_constraint_name_string : mmd_skeleton_joint_constraint_name_strings[mmd_skeleton_joint_constraint_name])
						{
							if (mmd_skeleton_joint_constraint_name_string == in_mmd_model_nodes[model_node_index].m_name)
							{

								out_animation_skeleton_joint_constraint_names.push_back(static_cast<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME>(mmd_skeleton_joint_constraint_name));
								found_skeleton_joint_constraint_name = true;
								break;
							}
						}
					}

					if (!found_skeleton_joint_constraint_name)
					{
						out_animation_skeleton_joint_constraint_names.push_back(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_INVALID);
					}
				}

				{
					brx_asset_import_skeleton_joint_constraint animation_skeleton_joint_inverse_kinematics_constraint;
					animation_skeleton_joint_inverse_kinematics_constraint.m_constraint_type = BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_INVERSE_KINEMATICS;
					animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_target_joint_index = out_model_node_to_animation_skeleton_joint_map[model_node_index];
					animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_end_effector_index = out_model_node_to_animation_skeleton_joint_map[in_mmd_model_nodes[model_node_index].m_ik_end_effector_index];
					animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_count = in_mmd_model_nodes[model_node_index].m_ik_link_indices.size();

					out_animation_skeleton_joint_constraints_storages.emplace_back();
					out_animation_skeleton_joint_constraints_storages.back().resize(animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_count);
					animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_indices = out_animation_skeleton_joint_constraints_storages.back().data();

					for (uint32_t ik_joint_index = 0U; ik_joint_index < animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_count; ++ik_joint_index)
					{
						animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_indices[ik_joint_index] = out_model_node_to_animation_skeleton_joint_map[in_mmd_model_nodes[model_node_index].m_ik_link_indices[ik_joint_index]];
					}

					if (2U == animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_count)
					{
						DirectX::XMFLOAT3 hinge_joint_normal_local_space;
						{
							DirectX::XMFLOAT4X4 const end_effector_transform_local_space = out_animation_skeleton_bind_pose_local_space[animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_end_effector_index];

							constexpr uint32_t const ball_and_socket_ik_joint_index = 0U;
							constexpr uint32_t const hinge_ik_joint_index = 1U;

							uint32_t const ball_and_socket_animation_skeleton_joint_index = animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_indices[ball_and_socket_ik_joint_index];
							uint32_t const hinge_animation_skeleton_joint_index = animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_joint_indices[hinge_ik_joint_index];

							DirectX::XMFLOAT4X4 const ball_and_socket_joint_transform_model_space = out_animation_skeleton_bind_pose_model_space[ball_and_socket_animation_skeleton_joint_index];

							DirectX::XMFLOAT4X4 const hinge_joint_transform_model_space = out_animation_skeleton_bind_pose_model_space[hinge_animation_skeleton_joint_index];

							constexpr float const INTERNAL_SCALE_EPSILON = 9E-5F;

							DirectX::XMVECTOR ball_and_socket_joint_hinge_joint_local_space_translation;
							{

								DirectX::XMVECTOR unused_determinant;
								DirectX::XMMATRIX ball_and_socket_joint_hinge_joint_local_space_transform = DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&ball_and_socket_joint_transform_model_space), DirectX::XMMatrixInverse(&unused_determinant, DirectX::XMLoadFloat4x4(&hinge_joint_transform_model_space)));

								DirectX::XMVECTOR ball_and_socket_joint_hinge_joint_local_space_scale;
								DirectX::XMVECTOR ball_and_socket_joint_hinge_joint_local_space_rotation;
								bool directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&ball_and_socket_joint_hinge_joint_local_space_scale, &ball_and_socket_joint_hinge_joint_local_space_rotation, &ball_and_socket_joint_hinge_joint_local_space_translation, ball_and_socket_joint_hinge_joint_local_space_transform);
								assert(directx_xm_matrix_decompose);

								assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(ball_and_socket_joint_hinge_joint_local_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON))));
							}

							DirectX::XMVECTOR end_effector_hinge_joint_local_space_translation;
							{
								DirectX::XMVECTOR end_effector_hinge_joint_local_space_scale;
								DirectX::XMVECTOR end_effector_hinge_joint_local_space_rotation;
								bool directx_xm_matrix_decompose = DirectX::XMMatrixDecompose(&end_effector_hinge_joint_local_space_scale, &end_effector_hinge_joint_local_space_rotation, &end_effector_hinge_joint_local_space_translation, DirectX::XMLoadFloat4x4(&end_effector_transform_local_space));
								assert(directx_xm_matrix_decompose);

								assert(DirectX::XMVector3EqualInt(DirectX::XMVectorTrueInt(), DirectX::XMVectorLess(DirectX::XMVectorAbs(DirectX::XMVectorSubtract(end_effector_hinge_joint_local_space_scale, DirectX::XMVectorSplatOne())), DirectX::XMVectorReplicate(INTERNAL_SCALE_EPSILON))));
							}

							DirectX::XMStoreFloat3(&hinge_joint_normal_local_space, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(ball_and_socket_joint_hinge_joint_local_space_translation, end_effector_hinge_joint_local_space_translation)));
						}

						DirectX::XMFLOAT3 hinge_joint_axis_local_space;
						float cosine_max_hinge_joint_angle;
						float cosine_min_hinge_joint_angle;
						if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle)
						{
							float rotation_limit_x = std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x));
							float rotation_limit_y = std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y));
							float rotation_limit_z = std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z));

							if ((rotation_limit_x >= rotation_limit_y) && (rotation_limit_x >= rotation_limit_z))
							{
								if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x >= 0.0F && in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x >= 0.0F)
								{
									hinge_joint_axis_local_space.x = 1.0F;
									hinge_joint_axis_local_space.y = 0.0F;
									hinge_joint_axis_local_space.z = 0.0F;

									cosine_max_hinge_joint_angle = std::cos(std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x)));
									cosine_min_hinge_joint_angle = std::cos(std::min(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x)));
								}
								else if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x <= 0.0F && in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x <= 0.0F)
								{
									hinge_joint_axis_local_space.x = -1.0F;
									hinge_joint_axis_local_space.y = 0.0F;
									hinge_joint_axis_local_space.z = 0.0F;

									cosine_max_hinge_joint_angle = std::cos(std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x)));
									cosine_min_hinge_joint_angle = std::cos(std::min(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x)));
								}
								else
								{
									float rotation_limit_min = std::min(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x, in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x);
									float rotation_limit_max = std::max(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_x, in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_x);
									assert(rotation_limit_min <= 0.0F);
									assert(rotation_limit_max >= 0.0F);

									if (std::abs(rotation_limit_max) >= std::abs(rotation_limit_min))
									{
										hinge_joint_axis_local_space.x = 1.0F;
										hinge_joint_axis_local_space.y = 0.0F;
										hinge_joint_axis_local_space.z = 0.0F;

										cosine_max_hinge_joint_angle = std::cos(std::abs(rotation_limit_max));
										cosine_min_hinge_joint_angle = 1.0F;
									}
									else
									{
										hinge_joint_axis_local_space.x = -1.0F;
										hinge_joint_axis_local_space.y = 0.0F;
										hinge_joint_axis_local_space.z = 0.0F;

										cosine_max_hinge_joint_angle = std::cos(std::abs(rotation_limit_min));
										cosine_min_hinge_joint_angle = 1.0F;
									}
								}
							}
							else if ((rotation_limit_y >= rotation_limit_z) && (rotation_limit_y >= rotation_limit_x))
							{
								if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y >= 0.0F && in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y >= 0.0F)
								{
									hinge_joint_axis_local_space.x = 0.0F;
									hinge_joint_axis_local_space.y = 1.0F;
									hinge_joint_axis_local_space.z = 0.0F;

									cosine_max_hinge_joint_angle = std::cos(std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y)));
									cosine_min_hinge_joint_angle = std::cos(std::min(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y)));
								}
								else if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y <= 0.0F && in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y <= 0.0F)
								{
									hinge_joint_axis_local_space.x = 0.0F;
									hinge_joint_axis_local_space.y = -1.0F;
									hinge_joint_axis_local_space.z = 0.0F;

									cosine_max_hinge_joint_angle = std::cos(std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y)));
									cosine_min_hinge_joint_angle = std::cos(std::min(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y)));
								}
								else
								{
									float rotation_limit_min = std::min(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y, in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y);
									float rotation_limit_max = std::max(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_y, in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_y);
									assert(rotation_limit_min <= 0.0F);
									assert(rotation_limit_max >= 0.0F);

									if (std::abs(rotation_limit_max) >= std::abs(rotation_limit_min))
									{
										hinge_joint_axis_local_space.x = 0.0F;
										hinge_joint_axis_local_space.y = 1.0F;
										hinge_joint_axis_local_space.z = 0.0F;

										cosine_max_hinge_joint_angle = std::cos(std::abs(rotation_limit_max));
										cosine_min_hinge_joint_angle = 1.0F;
									}
									else
									{
										hinge_joint_axis_local_space.x = 0.0F;
										hinge_joint_axis_local_space.y = -1.0F;
										hinge_joint_axis_local_space.z = 0.0F;

										cosine_max_hinge_joint_angle = std::cos(std::abs(rotation_limit_min));
										cosine_min_hinge_joint_angle = 1.0F;
									}
								}
							}
							else
							{
								assert((rotation_limit_z >= rotation_limit_x) && (rotation_limit_z >= rotation_limit_y));

								if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z >= 0.0F && in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z >= 0.0F)
								{
									hinge_joint_axis_local_space.x = 0.0F;
									hinge_joint_axis_local_space.y = 0.0F;
									hinge_joint_axis_local_space.z = 1.0F;

									cosine_max_hinge_joint_angle = std::cos(std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z)));
									cosine_min_hinge_joint_angle = std::cos(std::min(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z)));
								}
								else if (in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z <= 0.0F && in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z <= 0.0F)
								{
									hinge_joint_axis_local_space.x = 0.0F;
									hinge_joint_axis_local_space.y = 0.0F;
									hinge_joint_axis_local_space.z = -1.0F;

									cosine_max_hinge_joint_angle = std::cos(std::max(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z)));
									cosine_min_hinge_joint_angle = std::cos(std::min(std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z), std::abs(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z)));
								}
								else
								{
									float rotation_limit_min = std::min(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z, in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z);
									float rotation_limit_max = std::max(in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_min.m_z, in_mmd_model_nodes[model_node_index].m_ik_two_links_hinge_limit_angle_max.m_z);
									assert(rotation_limit_min <= 0.0F);
									assert(rotation_limit_max >= 0.0F);

									if (std::abs(rotation_limit_max) >= std::abs(rotation_limit_min))
									{
										hinge_joint_axis_local_space.x = 0.0F;
										hinge_joint_axis_local_space.y = 0.0F;
										hinge_joint_axis_local_space.z = 1.0F;

										cosine_max_hinge_joint_angle = std::cos(std::abs(rotation_limit_max));
										cosine_min_hinge_joint_angle = 1.0F;
									}
									else
									{
										hinge_joint_axis_local_space.x = 0.0F;
										hinge_joint_axis_local_space.y = 0.0F;
										hinge_joint_axis_local_space.z = -1.0F;

										cosine_max_hinge_joint_angle = std::cos(std::abs(rotation_limit_min));
										cosine_min_hinge_joint_angle = 1.0F;
									}
								}
							}

							if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&hinge_joint_axis_local_space), DirectX::XMLoadFloat3(&hinge_joint_normal_local_space))) < (1.0F - 1E-1F))
							{
								assert(false);
								hinge_joint_axis_local_space = hinge_joint_normal_local_space;
								cosine_max_hinge_joint_angle = -1.0F;
								cosine_min_hinge_joint_angle = 1.0F;
							}
						}
						else
						{
							hinge_joint_axis_local_space = hinge_joint_normal_local_space;
							cosine_max_hinge_joint_angle = -1.0F;
							cosine_min_hinge_joint_angle = 1.0F;
						}

						animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_two_joints_hinge_joint_axis_local_space[0] = hinge_joint_axis_local_space.x;
						animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_two_joints_hinge_joint_axis_local_space[1] = hinge_joint_axis_local_space.y;
						animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_ik_two_joints_hinge_joint_axis_local_space[2] = hinge_joint_axis_local_space.z;
						animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_cosine_max_ik_two_joints_hinge_joint_angle = cosine_max_hinge_joint_angle;

						animation_skeleton_joint_inverse_kinematics_constraint.m_inverse_kinematics.m_cosine_min_ik_two_joints_hinge_joint_angle = cosine_min_hinge_joint_angle;
					}

					out_animation_skeleton_joint_constraints.push_back(animation_skeleton_joint_inverse_kinematics_constraint);
				}
			}
		}
	}
}

static inline void internal_import_ragdoll_physics(mcrt_vector<mmd_pmx_rigid_body_t> const &in_mmd_rigid_bodies, mcrt_vector<mmd_pmx_constraint_t> const &in_mmd_constraints, mcrt_vector<uint32_t> const &in_model_node_to_animation_skeleton_joint_map, mcrt_vector<DirectX::XMFLOAT4X4> const &in_animation_skeleton_bind_pose_model_space, mcrt_vector<brx_asset_import_physics_rigid_body> &out_ragdoll_skeleton_rigid_bodies, mcrt_vector<brx_asset_import_physics_constraint> &out_ragdoll_skeleton_constraints, mcrt_vector<uint32_t> &out_ragdoll_skeleton_joint_parent_indices, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_animation_to_ragdoll_direct_mapping, mcrt_vector<brx_asset_import_ragdoll_direct_mapping> &out_ragdoll_to_animation_direct_mapping)
{
	uint32_t const rigid_body_count = in_mmd_rigid_bodies.size();

	mcrt_vector<uint32_t> ragdoll_skeleton_joint_to_rigid_body_map(static_cast<size_t>(rigid_body_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
	mcrt_vector<uint32_t> rigid_body_to_ragdoll_skeleton_joint_map(static_cast<size_t>(rigid_body_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
	{
		assert(out_ragdoll_skeleton_joint_parent_indices.empty());
		out_ragdoll_skeleton_joint_parent_indices = {};

		mcrt_vector<uint32_t> rigid_body_parent_indices(static_cast<size_t>(rigid_body_count), BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
		for (mmd_pmx_constraint_t const &mmd_constraint : in_mmd_constraints)
		{
			uint32_t const parent_index = mmd_constraint.m_rigid_body_a_index;
			uint32_t const child_index = mmd_constraint.m_rigid_body_b_index;

			if (parent_index < child_index)
			{
				if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == rigid_body_parent_indices[child_index])
				{
					rigid_body_parent_indices[child_index] = parent_index;
				}
				else
				{
					if (rigid_body_parent_indices[child_index] < parent_index)
					{
						rigid_body_parent_indices[child_index] = parent_index;
					}
				}
			}
		}

		mcrt_vector<uint32_t> rigid_body_depth_first_search_stack;
		mcrt_vector<mcrt_vector<uint32_t>> rigid_body_children_indices(static_cast<size_t>(rigid_body_count));
		for (uint32_t rigid_body_index_plus_1 = rigid_body_count; rigid_body_index_plus_1 > 0U; --rigid_body_index_plus_1)
		{
			uint32_t const rigid_body_index = rigid_body_index_plus_1 - 1U;
			uint32_t rigid_body_parent_index = rigid_body_parent_indices[rigid_body_index];
			if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != rigid_body_parent_index)
			{
				rigid_body_children_indices[rigid_body_parent_index].push_back(rigid_body_index);
			}
			else
			{
				rigid_body_depth_first_search_stack.push_back(rigid_body_index);
			}
		}
		assert((0U == rigid_body_count) || (!rigid_body_depth_first_search_stack.empty()));

		mcrt_vector<bool> rigid_body_visited_flags(static_cast<size_t>(rigid_body_count), false);
		mcrt_vector<bool> rigid_body_pushed_flags(static_cast<size_t>(rigid_body_count), false);
		while (!rigid_body_depth_first_search_stack.empty())
		{
			uint32_t const rigid_body_current_index = rigid_body_depth_first_search_stack.back();
			rigid_body_depth_first_search_stack.pop_back();

			assert(!rigid_body_visited_flags[rigid_body_current_index]);
			rigid_body_visited_flags[rigid_body_current_index] = true;

			uint32_t const ragdoll_skeleton_joint_current_index = out_ragdoll_skeleton_joint_parent_indices.size();

			uint32_t const rigid_body_parent_index = rigid_body_parent_indices[rigid_body_current_index];

			if (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == rigid_body_parent_index)
			{
				out_ragdoll_skeleton_joint_parent_indices.push_back(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID);
			}
			else
			{
				assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != rigid_body_to_ragdoll_skeleton_joint_map[rigid_body_parent_index]);
				out_ragdoll_skeleton_joint_parent_indices.push_back(rigid_body_to_ragdoll_skeleton_joint_map[rigid_body_parent_index]);
			}

			assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == ragdoll_skeleton_joint_to_rigid_body_map[ragdoll_skeleton_joint_current_index]);
			ragdoll_skeleton_joint_to_rigid_body_map[ragdoll_skeleton_joint_current_index] = rigid_body_current_index;

			assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == rigid_body_to_ragdoll_skeleton_joint_map[rigid_body_current_index]);
			rigid_body_to_ragdoll_skeleton_joint_map[rigid_body_current_index] = ragdoll_skeleton_joint_current_index;

			for (uint32_t rigid_body_child_index_index_plus_1 = static_cast<uint32_t>(rigid_body_children_indices[rigid_body_current_index].size()); rigid_body_child_index_index_plus_1 > 0U; --rigid_body_child_index_index_plus_1)
			{
				uint32_t const rigid_body_child_index = rigid_body_children_indices[rigid_body_current_index][rigid_body_child_index_index_plus_1 - 1U];

				if ((!rigid_body_visited_flags[rigid_body_child_index]) && (!rigid_body_pushed_flags[rigid_body_child_index]))
				{
					rigid_body_pushed_flags[rigid_body_child_index] = true;
					rigid_body_depth_first_search_stack.push_back(rigid_body_child_index);
				}
				else
				{
					assert(false);
				}
			}
		}

		assert(out_ragdoll_skeleton_joint_parent_indices.size() == rigid_body_count);
	}

	{
		assert(out_ragdoll_skeleton_rigid_bodies.empty());
		out_ragdoll_skeleton_rigid_bodies = {};

		assert(out_animation_to_ragdoll_direct_mapping.empty());
		out_animation_to_ragdoll_direct_mapping = {};

		assert(out_ragdoll_to_animation_direct_mapping.empty());
		out_ragdoll_to_animation_direct_mapping = {};

		out_ragdoll_skeleton_rigid_bodies.reserve(rigid_body_count);
		// out_animation_to_ragdoll_direct_mapping.reserve(rigid_body_count);
		// out_ragdoll_to_animation_direct_mapping.reserve(rigid_body_count);

		for (uint32_t ragdoll_skeleton_joint_index = 0U; ragdoll_skeleton_joint_index < rigid_body_count; ++ragdoll_skeleton_joint_index)
		{
			uint32_t const rigid_body_index = ragdoll_skeleton_joint_to_rigid_body_map[ragdoll_skeleton_joint_index];
			assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID != rigid_body_index);

			mmd_pmx_rigid_body_t const &mmd_rigid_body = in_mmd_rigid_bodies[rigid_body_index];

			DirectX::XMFLOAT4 ragdoll_rotation_model_space;
			{
				mmd_pmx_vec4_t const mmd_rigid_body_rotation = internal_transform_rotation(mmd_rigid_body.m_rotation);

				ragdoll_rotation_model_space = DirectX::XMFLOAT4(mmd_rigid_body_rotation.m_x, mmd_rigid_body_rotation.m_y, mmd_rigid_body_rotation.m_z, mmd_rigid_body_rotation.m_w);
			}

			DirectX::XMFLOAT3 ragdoll_translation_model_space;
			{
				mmd_pmx_vec3_t const mmd_rigid_body_translation = internal_transform_translation(mmd_rigid_body.m_translation);

				ragdoll_translation_model_space = DirectX::XMFLOAT3(mmd_rigid_body_translation.m_x, mmd_rigid_body_translation.m_y, mmd_rigid_body_translation.m_z);
			}

			mmd_pmx_vec3_t const mmd_shape_size = internal_transform_shape_size(mmd_rigid_body.m_shape_size);

			static_assert(0 == BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_SPHERE, "");
			static_assert(1 == BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_BOX, "");
			static_assert(2 == BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_CAPSULE, "");
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
					static_cast<BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_TYPE>(mmd_rigid_body.m_shape_type),
					{
						mmd_shape_size.m_x,
						mmd_shape_size.m_y,
						mmd_shape_size.m_z,
					},
					(0 == mmd_rigid_body.m_rigid_body_type) ? BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_KEYFRAME : BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_DYNAMIC,
					mmd_rigid_body.m_collision_filter_group,
					mmd_rigid_body.m_collision_filter_mask,
					mmd_rigid_body.m_mass,
					mmd_rigid_body.m_linear_damping,
					mmd_rigid_body.m_angular_damping,
					mmd_rigid_body.m_friction,
					mmd_rigid_body.m_restitution});

			if (mmd_rigid_body.m_bone_index < in_model_node_to_animation_skeleton_joint_map.size())
			{
				uint32_t const animation_skeleton_joint_index = in_model_node_to_animation_skeleton_joint_map[mmd_rigid_body.m_bone_index];

				DirectX::XMFLOAT4X4 ragdoll_transform_model_space;
				DirectX::XMStoreFloat4x4(&ragdoll_transform_model_space, DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&ragdoll_rotation_model_space)), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&ragdoll_translation_model_space))));

				if (0 == mmd_rigid_body.m_rigid_body_type)
				{
					DirectX::XMFLOAT4X4 animation_to_ragdoll_transform_model_space;
					{
						DirectX::XMVECTOR unused_determinant;
						DirectX::XMStoreFloat4x4(&animation_to_ragdoll_transform_model_space, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&ragdoll_transform_model_space), DirectX::XMMatrixInverse(&unused_determinant, DirectX::XMLoadFloat4x4(&in_animation_skeleton_bind_pose_model_space[animation_skeleton_joint_index]))));
					}
					animation_to_ragdoll_transform_model_space.m[3][3] = 1.0F;

					out_animation_to_ragdoll_direct_mapping.push_back(
						brx_asset_import_ragdoll_direct_mapping{
							animation_skeleton_joint_index,
							ragdoll_skeleton_joint_index,
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
				else
				{
					assert(1 == mmd_rigid_body.m_rigid_body_type || 2 == mmd_rigid_body.m_rigid_body_type);

					DirectX::XMFLOAT4X4 ragdoll_to_animation_transform_model_space;
					{
						DirectX::XMVECTOR unused_determinant;
						DirectX::XMStoreFloat4x4(&ragdoll_to_animation_transform_model_space, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&in_animation_skeleton_bind_pose_model_space[animation_skeleton_joint_index]), DirectX::XMMatrixInverse(&unused_determinant, DirectX::XMLoadFloat4x4(&ragdoll_transform_model_space))));
					}
					ragdoll_to_animation_transform_model_space.m[3][3] = 1.0F;

					out_ragdoll_to_animation_direct_mapping.push_back(
						brx_asset_import_ragdoll_direct_mapping{
							ragdoll_skeleton_joint_index,
							animation_skeleton_joint_index,
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
				}
			}
			else
			{
				assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == mmd_rigid_body.m_bone_index);
				assert(0 != mmd_rigid_body.m_rigid_body_type);
				assert(1 == mmd_rigid_body.m_rigid_body_type || 2 == mmd_rigid_body.m_rigid_body_type);
			}
		}

		assert(out_ragdoll_skeleton_rigid_bodies.size() == rigid_body_count);
		assert((out_animation_to_ragdoll_direct_mapping.size() + out_ragdoll_to_animation_direct_mapping.size()) <= rigid_body_count);
	}

	{
		assert(out_ragdoll_skeleton_constraints.empty());
		out_ragdoll_skeleton_constraints = {};

		out_ragdoll_skeleton_constraints.reserve(in_mmd_constraints.size());

		for (mmd_pmx_constraint_t const &mmd_constraint : in_mmd_constraints)
		{
			if ((mmd_constraint.m_rigid_body_a_index < in_mmd_rigid_bodies.size()) && (mmd_constraint.m_rigid_body_b_index < in_mmd_rigid_bodies.size()) && (mmd_constraint.m_rigid_body_a_index != mmd_constraint.m_rigid_body_b_index))
			{
				if (0 != in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_a_index].m_rigid_body_type || 0 != in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_b_index].m_rigid_body_type)
				{

					mmd_pmx_vec3_t const mmd_constraint_translation_limit_min = internal_transform_translation_limit(mmd_constraint.m_translation_limit_min);
					mmd_pmx_vec3_t const mmd_constraint_translation_limit_max = internal_transform_translation_limit(mmd_constraint.m_translation_limit_max);
					mmd_pmx_vec3_t const mmd_constraint_rotation_limit_min = internal_transform_rotation_limit(mmd_constraint.m_rotation_limit_min);
					mmd_pmx_vec3_t const mmd_constraint_rotation_limit_max = internal_transform_rotation_limit(mmd_constraint.m_rotation_limit_max);

					float mmd_translation_limit_min_x = std::min(mmd_constraint_translation_limit_min.m_x, mmd_constraint_translation_limit_max.m_x);
					float mmd_translation_limit_max_x = std::max(mmd_constraint_translation_limit_min.m_x, mmd_constraint_translation_limit_max.m_x);
					float mmd_translation_limit_min_y = std::min(mmd_constraint_translation_limit_min.m_y, mmd_constraint_translation_limit_max.m_y);
					float mmd_translation_limit_max_y = std::max(mmd_constraint_translation_limit_min.m_y, mmd_constraint_translation_limit_max.m_y);
					float mmd_translation_limit_min_z = std::min(mmd_constraint_translation_limit_min.m_z, mmd_constraint_translation_limit_max.m_z);
					float mmd_translation_limit_max_z = std::max(mmd_constraint_translation_limit_min.m_z, mmd_constraint_translation_limit_max.m_z);

					float mmd_rotation_limit_min_x = std::min(mmd_constraint_rotation_limit_min.m_x, mmd_constraint_rotation_limit_max.m_x);
					float mmd_rotation_limit_max_x = std::max(mmd_constraint_rotation_limit_min.m_x, mmd_constraint_rotation_limit_max.m_x);
					float mmd_rotation_limit_min_y = std::min(mmd_constraint_rotation_limit_min.m_y, mmd_constraint_rotation_limit_max.m_y);
					float mmd_rotation_limit_max_y = std::max(mmd_constraint_rotation_limit_min.m_y, mmd_constraint_rotation_limit_max.m_y);
					float mmd_rotation_limit_min_z = std::min(mmd_constraint_rotation_limit_min.m_z, mmd_constraint_rotation_limit_max.m_z);
					float mmd_rotation_limit_max_z = std::max(mmd_constraint_rotation_limit_min.m_z, mmd_constraint_rotation_limit_max.m_z);

					DirectX::XMFLOAT3 mmd_constraint_local_origin;
					{
						mmd_pmx_vec3_t const mmd_constraint_translation = internal_transform_translation(mmd_constraint.m_translation);

						mmd_constraint_local_origin = DirectX::XMFLOAT3(mmd_constraint_translation.m_x, mmd_constraint_translation.m_y, mmd_constraint_translation.m_z);
					}

					DirectX::XMFLOAT3 mmd_constraint_local_axis_x;
					DirectX::XMFLOAT3 mmd_constraint_local_axis_y;
					DirectX::XMFLOAT3 mmd_constraint_local_axis_z;
					{
						mmd_pmx_vec4_t const mmd_constraint_rotation = internal_transform_rotation(mmd_constraint.m_rotation);

						DirectX::XMFLOAT4 const constraint_rotation_model_space(mmd_constraint_rotation.m_x, mmd_constraint_rotation.m_y, mmd_constraint_rotation.m_z, mmd_constraint_rotation.m_w);

						DirectX::XMMATRIX constraint_rotation_matrix_model_space = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&constraint_rotation_model_space));

						DirectX::XMFLOAT3 local_axis_x(1.0F, 0.0F, 0.0F);
						DirectX::XMStoreFloat3(&mmd_constraint_local_axis_x, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&local_axis_x), constraint_rotation_matrix_model_space));

						DirectX::XMFLOAT3 local_axis_y(0.0F, 1.0F, 0.0F);
						DirectX::XMStoreFloat3(&mmd_constraint_local_axis_y, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&local_axis_y), constraint_rotation_matrix_model_space));

						DirectX::XMFLOAT3 local_axis_z(0.0F, 0.0F, 1.0F);
						DirectX::XMStoreFloat3(&mmd_constraint_local_axis_z, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&local_axis_z), constraint_rotation_matrix_model_space));
					}

					DirectX::XMFLOAT3 const rigid_body_a_translation(
						in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_a_index].m_translation.m_x,
						in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_a_index].m_translation.m_y,
						in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_a_index].m_translation.m_z);

					DirectX::XMFLOAT3 const rigid_body_b_translation(
						in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_b_index].m_translation.m_x,
						in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_b_index].m_translation.m_y,
						in_mmd_rigid_bodies[mmd_constraint.m_rigid_body_b_index].m_translation.m_z);

					BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_TYPE brx_constraint_type;
					DirectX::XMFLOAT3 brx_pivot;
					DirectX::XMFLOAT3 brx_twist_axis;
					DirectX::XMFLOAT3 brx_plane_axis;
					DirectX::XMFLOAT3 brx_normal_axis;
					float brx_twist_limit[2];
					float brx_plane_limit[2];
					float brx_normal_limit[2];
					{
						brx_pivot = mmd_constraint_local_origin;

						constexpr float const INTERNAL_EPSILON = 1E-6F;
						// constexpr float const INTERNAL_PI = DirectX::XM_PI;
						constexpr float const INTERNAL_NEAR_PI_DIV_2 = DirectX::XM_PIDIV2 - INTERNAL_EPSILON;

						float mmd_translation_limit_abs_x = std::max(std::abs(mmd_translation_limit_min_x), std::abs(mmd_translation_limit_max_x));
						float mmd_translation_limit_abs_y = std::max(std::abs(mmd_translation_limit_min_y), std::abs(mmd_translation_limit_max_y));
						float mmd_translation_limit_abs_z = std::max(std::abs(mmd_translation_limit_min_z), std::abs(mmd_translation_limit_max_z));

						float mmd_rotation_limit_abs_x = std::max(std::abs(mmd_rotation_limit_min_x), std::abs(mmd_rotation_limit_max_x));
						float mmd_rotation_limit_abs_y = std::max(std::abs(mmd_rotation_limit_min_y), std::abs(mmd_rotation_limit_max_y));
						float mmd_rotation_limit_abs_z = std::max(std::abs(mmd_rotation_limit_min_z), std::abs(mmd_rotation_limit_max_z));

						if (mmd_rotation_limit_abs_x <= INTERNAL_EPSILON && mmd_rotation_limit_abs_y <= INTERNAL_EPSILON && mmd_rotation_limit_abs_z <= INTERNAL_EPSILON && mmd_translation_limit_abs_x <= INTERNAL_EPSILON && mmd_translation_limit_abs_y <= INTERNAL_EPSILON && mmd_translation_limit_abs_z <= INTERNAL_EPSILON)
						{
							brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_FIXED;

							brx_twist_axis = mmd_constraint_local_axis_x;

							brx_plane_axis = mmd_constraint_local_axis_y;

							brx_normal_axis = mmd_constraint_local_axis_z;

							brx_twist_limit[0] = 0.0F;
							brx_twist_limit[1] = 0.0F;

							brx_plane_limit[0] = 0.0F;
							brx_plane_limit[1] = 0.0F;

							brx_normal_limit[0] = 0.0F;
							brx_normal_limit[1] = 0.0F;
						}
						else if (mmd_rotation_limit_abs_x <= INTERNAL_EPSILON && mmd_rotation_limit_abs_y <= INTERNAL_EPSILON && mmd_rotation_limit_abs_z <= INTERNAL_EPSILON && mmd_translation_limit_abs_x <= INTERNAL_EPSILON && mmd_translation_limit_abs_y <= INTERNAL_EPSILON)
						{
							brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_PRISMATIC;

							brx_twist_axis = mmd_constraint_local_axis_x;

							brx_plane_axis = mmd_constraint_local_axis_y;

							brx_normal_axis = mmd_constraint_local_axis_z;

							brx_twist_limit[0] = 0.0F;
							brx_twist_limit[1] = 0.0F;

							brx_plane_limit[0] = 0.0F;
							brx_plane_limit[1] = 0.0F;

							brx_normal_limit[0] = mmd_translation_limit_min_z;
							brx_normal_limit[1] = mmd_translation_limit_max_z;
						}
						else if (mmd_rotation_limit_abs_x <= INTERNAL_EPSILON && mmd_rotation_limit_abs_y <= INTERNAL_EPSILON && mmd_rotation_limit_abs_z <= INTERNAL_EPSILON && mmd_translation_limit_abs_y <= INTERNAL_EPSILON && mmd_translation_limit_abs_z <= INTERNAL_EPSILON)
						{
							brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_PRISMATIC;

							brx_twist_axis = mmd_constraint_local_axis_y;

							brx_plane_axis = mmd_constraint_local_axis_z;

							brx_normal_axis = mmd_constraint_local_axis_x;

							brx_twist_limit[0] = 0.0F;
							brx_twist_limit[1] = 0.0F;

							brx_plane_limit[0] = 0.0F;
							brx_plane_limit[1] = 0.0F;

							brx_normal_limit[0] = mmd_translation_limit_min_x;
							brx_normal_limit[1] = mmd_translation_limit_max_x;
						}
						else if (mmd_rotation_limit_abs_x <= INTERNAL_EPSILON && mmd_rotation_limit_abs_y <= INTERNAL_EPSILON && mmd_rotation_limit_abs_z <= INTERNAL_EPSILON && mmd_translation_limit_abs_z <= INTERNAL_EPSILON && mmd_translation_limit_abs_x <= INTERNAL_EPSILON)
						{
							brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_PRISMATIC;

							brx_twist_axis = mmd_constraint_local_axis_z;

							brx_plane_axis = mmd_constraint_local_axis_x;

							brx_normal_axis = mmd_constraint_local_axis_y;

							brx_twist_limit[0] = 0.0F;
							brx_twist_limit[1] = 0.0F;

							brx_plane_limit[0] = 0.0F;
							brx_plane_limit[1] = 0.0F;

							brx_normal_limit[0] = mmd_translation_limit_min_y;
							brx_normal_limit[1] = mmd_translation_limit_max_y;
						}
						else
						{
							// convert from translation to rotation
							if (mmd_translation_limit_abs_x >= INTERNAL_EPSILON || mmd_translation_limit_abs_y >= INTERNAL_EPSILON || mmd_translation_limit_abs_z >= INTERNAL_EPSILON)
							{
								assert((mmd_rotation_limit_abs_x >= INTERNAL_EPSILON || mmd_rotation_limit_abs_y >= INTERNAL_EPSILON || mmd_rotation_limit_abs_z >= INTERNAL_EPSILON) || (mmd_translation_limit_abs_x >= INTERNAL_EPSILON && mmd_translation_limit_abs_y >= INTERNAL_EPSILON && mmd_translation_limit_abs_z >= INTERNAL_EPSILON));

								float rigid_body_b_body_space_translation_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&rigid_body_b_translation), DirectX::XMLoadFloat3(&mmd_constraint_local_origin))));

								if (std::abs(mmd_rotation_limit_abs_x - mmd_rotation_limit_abs_y) <= INTERNAL_EPSILON && std::abs(mmd_rotation_limit_abs_y - mmd_rotation_limit_abs_z) <= INTERNAL_EPSILON && std::abs(mmd_rotation_limit_abs_z - mmd_rotation_limit_abs_x) <= INTERNAL_EPSILON)
								{
									if (mmd_translation_limit_abs_x <= mmd_translation_limit_abs_y && mmd_translation_limit_abs_x <= mmd_translation_limit_abs_z)
									{
										mmd_rotation_limit_min_y = std::min(mmd_rotation_limit_min_y, internal_asin(mmd_translation_limit_min_z, rigid_body_b_body_space_translation_length));
										mmd_rotation_limit_min_z = std::min(mmd_rotation_limit_min_z, internal_asin(mmd_translation_limit_min_y, rigid_body_b_body_space_translation_length));

										mmd_rotation_limit_max_y = std::max(mmd_rotation_limit_max_y, internal_asin(mmd_translation_limit_max_z, rigid_body_b_body_space_translation_length));
										mmd_rotation_limit_max_z = std::max(mmd_rotation_limit_max_z, internal_asin(mmd_translation_limit_max_y, rigid_body_b_body_space_translation_length));
									}
									else if (mmd_translation_limit_abs_y <= mmd_translation_limit_abs_z && mmd_translation_limit_abs_y <= mmd_translation_limit_abs_x)
									{
										mmd_rotation_limit_min_x = std::min(mmd_rotation_limit_min_x, internal_asin(mmd_translation_limit_min_z, rigid_body_b_body_space_translation_length));
										mmd_rotation_limit_min_z = std::min(mmd_rotation_limit_min_z, internal_asin(mmd_translation_limit_min_x, rigid_body_b_body_space_translation_length));

										mmd_rotation_limit_max_x = std::max(mmd_rotation_limit_max_x, internal_asin(mmd_translation_limit_max_z, rigid_body_b_body_space_translation_length));
										mmd_rotation_limit_max_z = std::max(mmd_rotation_limit_max_z, internal_asin(mmd_translation_limit_max_x, rigid_body_b_body_space_translation_length));
									}
									else
									{
										assert(mmd_translation_limit_abs_z <= mmd_translation_limit_abs_x && mmd_translation_limit_abs_z <= mmd_translation_limit_abs_y);

										mmd_rotation_limit_min_x = std::min(mmd_rotation_limit_min_x, internal_asin(mmd_translation_limit_min_y, rigid_body_b_body_space_translation_length));
										mmd_rotation_limit_min_y = std::min(mmd_rotation_limit_min_y, internal_asin(mmd_translation_limit_min_x, rigid_body_b_body_space_translation_length));

										mmd_rotation_limit_max_x = std::max(mmd_rotation_limit_max_x, internal_asin(mmd_translation_limit_max_y, rigid_body_b_body_space_translation_length));
										mmd_rotation_limit_max_y = std::max(mmd_rotation_limit_max_y, internal_asin(mmd_translation_limit_max_x, rigid_body_b_body_space_translation_length));
									}
								}
								else if (mmd_rotation_limit_abs_x <= mmd_rotation_limit_abs_y && mmd_rotation_limit_abs_x <= mmd_rotation_limit_abs_z)
								{
									mmd_rotation_limit_min_y = std::min(mmd_rotation_limit_min_y, internal_asin(mmd_translation_limit_min_z, rigid_body_b_body_space_translation_length));
									mmd_rotation_limit_min_z = std::min(mmd_rotation_limit_min_z, internal_asin(mmd_translation_limit_min_y, rigid_body_b_body_space_translation_length));

									mmd_rotation_limit_max_y = std::max(mmd_rotation_limit_max_y, internal_asin(mmd_translation_limit_max_z, rigid_body_b_body_space_translation_length));
									mmd_rotation_limit_max_z = std::max(mmd_rotation_limit_max_z, internal_asin(mmd_translation_limit_max_y, rigid_body_b_body_space_translation_length));
								}
								else if (mmd_rotation_limit_abs_y <= mmd_rotation_limit_abs_z && mmd_rotation_limit_abs_y <= mmd_rotation_limit_abs_x)
								{
									mmd_rotation_limit_min_x = std::min(mmd_rotation_limit_min_x, internal_asin(mmd_translation_limit_min_z, rigid_body_b_body_space_translation_length));
									mmd_rotation_limit_min_z = std::min(mmd_rotation_limit_min_z, internal_asin(mmd_translation_limit_min_x, rigid_body_b_body_space_translation_length));

									mmd_rotation_limit_max_x = std::max(mmd_rotation_limit_max_x, internal_asin(mmd_translation_limit_max_z, rigid_body_b_body_space_translation_length));
									mmd_rotation_limit_max_z = std::max(mmd_rotation_limit_max_z, internal_asin(mmd_translation_limit_max_x, rigid_body_b_body_space_translation_length));
								}
								else
								{
									assert(mmd_rotation_limit_abs_z <= mmd_rotation_limit_abs_x && mmd_rotation_limit_abs_z <= mmd_rotation_limit_abs_y);

									mmd_rotation_limit_min_x = std::min(mmd_rotation_limit_min_x, internal_asin(mmd_translation_limit_min_y, rigid_body_b_body_space_translation_length));
									mmd_rotation_limit_min_y = std::min(mmd_rotation_limit_min_y, internal_asin(mmd_translation_limit_min_x, rigid_body_b_body_space_translation_length));

									mmd_rotation_limit_max_x = std::max(mmd_rotation_limit_max_x, internal_asin(mmd_translation_limit_max_y, rigid_body_b_body_space_translation_length));
									mmd_rotation_limit_max_y = std::max(mmd_rotation_limit_max_y, internal_asin(mmd_translation_limit_max_x, rigid_body_b_body_space_translation_length));
								}

								mmd_rotation_limit_abs_x = std::max(std::abs(mmd_rotation_limit_min_x), std::abs(mmd_rotation_limit_max_x));
								mmd_rotation_limit_abs_y = std::max(std::abs(mmd_rotation_limit_min_y), std::abs(mmd_rotation_limit_max_y));
								mmd_rotation_limit_abs_z = std::max(std::abs(mmd_rotation_limit_min_z), std::abs(mmd_rotation_limit_max_z));

								mmd_translation_limit_min_x = 0.0F;
								mmd_translation_limit_max_x = 0.0F;
								mmd_translation_limit_min_y = 0.0F;
								mmd_translation_limit_max_y = 0.0F;
								mmd_translation_limit_min_z = 0.0F;
								mmd_translation_limit_max_z = 0.0F;

								mmd_translation_limit_abs_x = std::max(std::abs(mmd_translation_limit_min_x), std::abs(mmd_translation_limit_max_x));
								mmd_translation_limit_abs_y = std::max(std::abs(mmd_translation_limit_min_y), std::abs(mmd_translation_limit_max_y));
								mmd_translation_limit_abs_z = std::max(std::abs(mmd_translation_limit_min_z), std::abs(mmd_translation_limit_max_z));
							}

							assert(mmd_rotation_limit_abs_x >= INTERNAL_EPSILON || mmd_rotation_limit_abs_y >= INTERNAL_EPSILON || mmd_rotation_limit_abs_z >= INTERNAL_EPSILON);

							if (mmd_rotation_limit_abs_x <= INTERNAL_EPSILON && mmd_rotation_limit_abs_y <= INTERNAL_EPSILON)
							{
								brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_HINGE;

								brx_twist_axis = mmd_constraint_local_axis_x;

								brx_plane_axis = mmd_constraint_local_axis_y;

								brx_normal_axis = mmd_constraint_local_axis_z;

								brx_twist_limit[0] = 0.0F;
								brx_twist_limit[1] = 0.0F;

								brx_plane_limit[0] = 0.0F;
								brx_plane_limit[1] = 0.0F;

								brx_normal_limit[0] = mmd_rotation_limit_min_z;
								brx_normal_limit[1] = mmd_rotation_limit_max_z;
							}
							else if (mmd_rotation_limit_abs_y <= INTERNAL_EPSILON && mmd_rotation_limit_abs_z <= INTERNAL_EPSILON)
							{
								brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_HINGE;

								brx_twist_axis = mmd_constraint_local_axis_y;

								brx_plane_axis = mmd_constraint_local_axis_z;

								brx_normal_axis = mmd_constraint_local_axis_x;

								brx_twist_limit[0] = 0.0F;
								brx_twist_limit[1] = 0.0F;

								brx_plane_limit[0] = 0.0F;
								brx_plane_limit[1] = 0.0F;

								brx_normal_limit[0] = mmd_rotation_limit_min_x;
								brx_normal_limit[1] = mmd_rotation_limit_max_x;
							}
							else if (mmd_rotation_limit_abs_z <= INTERNAL_EPSILON && mmd_rotation_limit_abs_x <= INTERNAL_EPSILON)
							{
								brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_HINGE;

								brx_twist_axis = mmd_constraint_local_axis_z;

								brx_plane_axis = mmd_constraint_local_axis_x;

								brx_normal_axis = mmd_constraint_local_axis_y;

								brx_twist_limit[0] = 0.0F;
								brx_twist_limit[1] = 0.0F;

								brx_plane_limit[0] = 0.0F;
								brx_plane_limit[1] = 0.0F;

								brx_normal_limit[0] = mmd_rotation_limit_min_y;
								brx_normal_limit[1] = mmd_rotation_limit_max_y;
							}
							else if ((std::abs(mmd_rotation_limit_abs_x) >= INTERNAL_NEAR_PI_DIV_2 && std::abs(mmd_rotation_limit_abs_y) >= INTERNAL_NEAR_PI_DIV_2) || (std::abs(mmd_rotation_limit_abs_y) >= INTERNAL_NEAR_PI_DIV_2 && std::abs(mmd_rotation_limit_abs_z) >= INTERNAL_NEAR_PI_DIV_2) || (std::abs(mmd_rotation_limit_abs_z) >= INTERNAL_NEAR_PI_DIV_2 && std::abs(mmd_rotation_limit_abs_x) >= INTERNAL_NEAR_PI_DIV_2))
							{
								brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_BALL_AND_SOCKET;

								brx_twist_axis = mmd_constraint_local_axis_x;

								brx_plane_axis = mmd_constraint_local_axis_y;

								brx_normal_axis = mmd_constraint_local_axis_z;

								brx_twist_limit[0] = 0.0F;
								brx_twist_limit[1] = 0.0F;

								brx_plane_limit[0] = 0.0F;
								brx_plane_limit[1] = 0.0F;

								brx_normal_limit[0] = 0.0F;
								brx_normal_limit[1] = 0.0F;
							}
							else
							{
								brx_constraint_type = BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_RAGDOLL;

								if (mmd_rotation_limit_abs_x <= mmd_rotation_limit_abs_y && mmd_rotation_limit_abs_x <= mmd_rotation_limit_abs_z)
								{
									brx_twist_axis = mmd_constraint_local_axis_x;

									brx_plane_axis = mmd_constraint_local_axis_y;

									brx_normal_axis = mmd_constraint_local_axis_z;

									brx_twist_limit[0] = mmd_rotation_limit_min_x;
									brx_twist_limit[1] = mmd_rotation_limit_max_x;

									brx_plane_limit[0] = mmd_rotation_limit_min_y;
									brx_plane_limit[1] = mmd_rotation_limit_max_y;

									brx_normal_limit[0] = mmd_rotation_limit_min_z;
									brx_normal_limit[1] = mmd_rotation_limit_max_z;
								}
								else if (mmd_rotation_limit_abs_y <= mmd_rotation_limit_abs_z && mmd_rotation_limit_abs_y <= mmd_rotation_limit_abs_x)
								{

									brx_twist_axis = mmd_constraint_local_axis_y;

									brx_plane_axis = mmd_constraint_local_axis_z;

									brx_normal_axis = mmd_constraint_local_axis_x;

									brx_twist_limit[0] = mmd_rotation_limit_min_y;
									brx_twist_limit[1] = mmd_rotation_limit_max_y;

									brx_plane_limit[0] = mmd_rotation_limit_min_z;
									brx_plane_limit[1] = mmd_rotation_limit_max_z;

									brx_normal_limit[0] = mmd_rotation_limit_min_x;
									brx_normal_limit[1] = mmd_rotation_limit_max_x;
								}
								else
								{
									assert(mmd_rotation_limit_abs_z <= mmd_rotation_limit_abs_x && mmd_rotation_limit_abs_z <= mmd_rotation_limit_abs_y);

									brx_twist_axis = mmd_constraint_local_axis_z;

									brx_plane_axis = mmd_constraint_local_axis_x;

									brx_normal_axis = mmd_constraint_local_axis_y;

									brx_twist_limit[0] = mmd_rotation_limit_min_z;
									brx_twist_limit[1] = mmd_rotation_limit_max_z;

									brx_plane_limit[0] = mmd_rotation_limit_min_x;
									brx_plane_limit[1] = mmd_rotation_limit_max_x;

									brx_normal_limit[0] = mmd_rotation_limit_min_y;
									brx_normal_limit[1] = mmd_rotation_limit_max_y;
								}

								if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&brx_twist_axis), DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&rigid_body_b_translation), DirectX::XMLoadFloat3(&rigid_body_a_translation)))) < (-INTERNAL_EPSILON))
								{
									brx_twist_axis.x = 0.0F - brx_twist_axis.x;
									brx_twist_axis.y = 0.0F - brx_twist_axis.y;
									brx_twist_axis.z = 0.0F - brx_twist_axis.z;

									DirectX::XMFLOAT3 temp_plane_axis = brx_plane_axis;

									brx_plane_axis.x = 0.0F - brx_normal_axis.x;
									brx_plane_axis.y = 0.0F - brx_normal_axis.y;
									brx_plane_axis.z = 0.0F - brx_normal_axis.z;

									brx_normal_axis.x = 0.0F - temp_plane_axis.x;
									brx_normal_axis.y = 0.0F - temp_plane_axis.y;
									brx_normal_axis.z = 0.0F - temp_plane_axis.z;

									float temp_plane_limit[2] = {brx_plane_limit[0], brx_plane_limit[1]};

									brx_plane_limit[0] = brx_normal_limit[0];
									brx_plane_limit[1] = brx_normal_limit[1];

									brx_normal_limit[0] = temp_plane_limit[0];
									brx_normal_limit[1] = temp_plane_limit[1];
								}

								// Bullet Cone-Twist Constraint
								// https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CDB3638D-23AF-49EF-8EF6-53081EE4D39D
								// twist_span
								// swing_span2 (plane)
								// swing_span1 (normal)

								// float const twist_span = std::min(std::max(std::abs(brx_twist_limit[0]), std::abs(brx_twist_limit[1])), (INTERNAL_PI * 1.0F));
								// float const swing_span2 = std::min(std::max(std::abs(brx_plane_limit[0]), std::abs(brx_plane_limit[1])), (INTERNAL_PI * 0.5F));
								// float const swing_span1 = std::min(std::max(std::abs(brx_normal_limit[0]), std::abs(brx_normal_limit[1])), (INTERNAL_PI * 0.5F));

								// brx_twist_limit[0] = 0.0F - twist_span;
								// brx_twist_limit[1] = twist_span;

								// brx_plane_limit[0] = 0.0F - swing_span2;
								// brx_plane_limit[1] = swing_span2;

								// brx_normal_limit[0] = 0.0F - swing_span1;
								// brx_normal_limit[1] = swing_span1;
							}
						}
					}

					assert(mmd_constraint.m_rigid_body_a_index < rigid_body_to_ragdoll_skeleton_joint_map.size());

					assert(mmd_constraint.m_rigid_body_b_index < rigid_body_to_ragdoll_skeleton_joint_map.size());

					uint32_t const skeleton_joint_index_a = rigid_body_to_ragdoll_skeleton_joint_map[mmd_constraint.m_rigid_body_a_index];
					assert(ragdoll_skeleton_joint_to_rigid_body_map[skeleton_joint_index_a] == mmd_constraint.m_rigid_body_a_index);

					uint32_t const skeleton_joint_index_b = rigid_body_to_ragdoll_skeleton_joint_map[mmd_constraint.m_rigid_body_b_index];
					assert(ragdoll_skeleton_joint_to_rigid_body_map[skeleton_joint_index_b] == mmd_constraint.m_rigid_body_b_index);

					out_ragdoll_skeleton_constraints.push_back(
						brx_asset_import_physics_constraint{
							skeleton_joint_index_a,
							skeleton_joint_index_b,
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
					// ignore the constraints of two kinematics rigid bodies
					// assert(false);
				}
			}
			else
			{
				// TODO: allow self constraint???
				assert((BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == mmd_constraint.m_rigid_body_a_index) || (BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == mmd_constraint.m_rigid_body_b_index) || (mmd_constraint.m_rigid_body_a_index == mmd_constraint.m_rigid_body_b_index));
			}
		}

		assert(out_ragdoll_skeleton_constraints.size() <= in_mmd_constraints.size());
	}
}

static inline void internal_import_mesh_sections(mcrt_vector<mmd_pmx_vertex_t> const &in_vertices, mcrt_vector<mmd_pmx_face_t> const &in_faces, mcrt_vector<mmd_pmx_texture_t> const &in_textures, mcrt_vector<mmd_pmx_material_t> const &in_materials, mcrt_vector<mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t>> const &in_morph_targets_vertices, mcrt_vector<uint32_t> const &in_model_node_to_animation_skeleton_joint_map, mcrt_vector<internal_mmd_mesh_section_t> &out_mesh_sections)
{
	uint32_t const vertex_count = in_vertices.size();
	uint32_t const face_count = in_faces.size();
	uint32_t const material_count = in_materials.size();
	uint32_t const morph_target_count = in_morph_targets_vertices.size();

	mcrt_vector<bool> mesh_sections_morph_switches;
	mcrt_vector<uint32_t> mesh_sections_material_indices;
	mcrt_vector<mcrt_vector<uint32_t>> mesh_sections_face_indices;
	{
		mcrt_set<uint32_t> morph_face_indices;
		mcrt_set<uint32_t> non_morph_face_indices;
		{
			mcrt_set<uint32_t> morph_lower_bound_vertex_indices;
			{
				for (uint32_t morph_target_index = 0U; morph_target_index < morph_target_count; ++morph_target_index)
				{
					mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t> const &morph_target_vertices = in_morph_targets_vertices[morph_target_index];
					assert(!morph_target_vertices.empty());

					for (auto const &vertex_index_and_morph_target_vertex : morph_target_vertices)
					{
						uint32_t const vertex_index = vertex_index_and_morph_target_vertex.first;
						morph_lower_bound_vertex_indices.emplace(vertex_index);
					}
				}
			}

			mcrt_map<uint32_t, mcrt_vector<uint32_t>> vertex_index_to_face_indices_map;
			{
				for (uint32_t face_index = 0U; face_index < face_count; ++face_index)
				{
					vertex_index_to_face_indices_map[in_faces[face_index].m_vertex_indices[0]].push_back(face_index);
					vertex_index_to_face_indices_map[in_faces[face_index].m_vertex_indices[1]].push_back(face_index);
					vertex_index_to_face_indices_map[in_faces[face_index].m_vertex_indices[2]].push_back(face_index);
				}
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
		}

		assert((morph_face_indices.size() + non_morph_face_indices.size()) == face_count);

		// TODO: we may not merge the surface since the alpha blending depends on the vertex order?
		auto const material_compare = [&in_materials](uint32_t const &lhs, uint32_t const &rhs) -> bool
		{
			return lhs < rhs;
		};

		mcrt_map<uint32_t, mcrt_set<uint32_t>, decltype(material_compare)> materials_face_indices(material_compare);
		{
			uint32_t face_index_begin = 0U;

			for (uint32_t material_index = 0U; material_index < material_count; ++material_index)
			{
				mcrt_set<uint32_t> &material_face_indices = materials_face_indices[material_index];

				uint32_t const material_face_count = in_materials[material_index].m_face_count;

				// we preserve the vertex order even if we use the "set" instead of the "vector"
				for (uint32_t material_face_index = 0U; material_face_index < material_face_count; ++material_face_index)
				{
					material_face_indices.emplace(face_index_begin + material_face_index);
				}

				face_index_begin = (face_index_begin + material_face_count);
			}
		}

#ifndef NDEBUG
		{
			uint32_t material_face_count = 0U;
			for (auto const &material_face_indices : materials_face_indices)
			{
				material_face_count = (material_face_count + material_face_indices.second.size());
			}
			assert(face_count == material_face_count);
		}
#endif

		for (auto const &material_index_and_face_indices : materials_face_indices)
		{
			uint32_t const material_index = material_index_and_face_indices.first;
			mcrt_set<uint32_t> const &material_face_indices = material_index_and_face_indices.second;

			mcrt_vector<uint32_t> morph_material_face_indices;
			mcrt_vector<uint32_t> non_morph_material_face_indices;

			for (uint32_t const material_face_index : material_face_indices)
			{
				if (morph_face_indices.end() != morph_face_indices.find(material_face_index))
				{
					morph_material_face_indices.push_back(material_face_index);
				}
				else if (non_morph_face_indices.end() != non_morph_face_indices.find(material_face_index))
				{
					non_morph_material_face_indices.push_back(material_face_index);
				}
				else
				{
					assert(false);
				}
			}

			if (!morph_material_face_indices.empty())
			{
				mesh_sections_morph_switches.emplace_back(true);
				mesh_sections_material_indices.emplace_back(material_index);
				mesh_sections_face_indices.emplace_back(std::move(morph_material_face_indices));
			}

			if (!non_morph_material_face_indices.empty())
			{
				mesh_sections_morph_switches.emplace_back(false);
				mesh_sections_material_indices.emplace_back(material_index);
				mesh_sections_face_indices.emplace_back(std::move(non_morph_material_face_indices));
			}
		}

#ifndef NDEBUG
		{
			mcrt_set<uint32_t> face_indices;

			for (auto const &mesh_section_face_indices : mesh_sections_face_indices)
			{
				for (uint32_t const mesh_section_face_index : mesh_section_face_indices)
				{
					assert(face_indices.end() == face_indices.find(mesh_section_face_index));
					face_indices.emplace_hint(face_indices.end(), mesh_section_face_index);
				}
			}

			assert(face_count == face_indices.size());
		}
#endif
	}

	assert(out_mesh_sections.empty());
	out_mesh_sections = {};

	uint32_t const mesh_section_count = mesh_sections_face_indices.size();
	assert(mesh_sections_morph_switches.size() == mesh_section_count);
	assert(mesh_sections_material_indices.size() == mesh_section_count);

	out_mesh_sections.resize(mesh_section_count);

	for (uint32_t mesh_section_index = 0U; mesh_section_index < mesh_section_count; ++mesh_section_index)
	{
		bool const mesh_section_morph_switch = mesh_sections_morph_switches[mesh_section_index];
		uint32_t const mesh_section_material_index = mesh_sections_material_indices[mesh_section_index];
		mcrt_vector<uint32_t> const &mesh_section_face_indices = mesh_sections_face_indices[mesh_section_index];

		mcrt_vector<internal_mmd_vertex_t> new_vertices;
		mcrt_map<uint32_t, uint32_t> original_vertex_index_to_new_vertex_index;
		{
			mcrt_map<uint32_t, internal_mmd_vertex_t> original_vertex_index_to_vertex_map;
			for (uint32_t const mesh_section_face_index : mesh_section_face_indices)
			{
				for (uint32_t face_vertex_index = 0U; face_vertex_index < 3U; ++face_vertex_index)
				{
					uint32_t const original_vertex_index = in_faces[mesh_section_face_index].m_vertex_indices[face_vertex_index];

					auto const found_vertex = original_vertex_index_to_vertex_map.find(original_vertex_index);

					if (original_vertex_index_to_vertex_map.end() == found_vertex)
					{
						DirectX::XMFLOAT3 new_position;
						{
							mmd_pmx_vec3_t const mmd_vertex_position = internal_transform_translation(in_vertices[original_vertex_index].m_position);

							new_position = DirectX::XMFLOAT3(mmd_vertex_position.m_x, mmd_vertex_position.m_y, mmd_vertex_position.m_z);
						}

						DirectX::XMFLOAT2 const new_uv(in_vertices[original_vertex_index].m_uv.m_x, in_vertices[original_vertex_index].m_uv.m_y);

						DirectX::XMFLOAT3 new_normal;
						{
							mmd_pmx_vec3_t const mmd_vertex_normal = internal_transform_normal(in_vertices[original_vertex_index].m_normal);

							DirectX::XMFLOAT3 original_normal(mmd_vertex_normal.m_x, mmd_vertex_normal.m_y, mmd_vertex_normal.m_z);

							DirectX::XMVECTOR simd_original_normal = DirectX::XMLoadFloat3(&original_normal);

							float const normal_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(simd_original_normal));

							if (normal_length > 1E-6F)
							{
								assert(std::abs(normal_length - 1.0F) < 5E-2F);

								DirectX::XMStoreFloat3(&new_normal, DirectX::XMVectorScale(simd_original_normal, (1.0F / normal_length)));
							}
							else
							{
								// assert(false);

								// Y-Up
								new_normal.x = 0.0F;
								new_normal.y = 1.0F;
								new_normal.z = 0.0F;
							}
						}

						uint32_t const new_indices[4] = {
							in_model_node_to_animation_skeleton_joint_map[in_vertices[original_vertex_index].m_bone_indices[0]],
							in_model_node_to_animation_skeleton_joint_map[in_vertices[original_vertex_index].m_bone_indices[1]],
							in_model_node_to_animation_skeleton_joint_map[in_vertices[original_vertex_index].m_bone_indices[2]],
							in_model_node_to_animation_skeleton_joint_map[in_vertices[original_vertex_index].m_bone_indices[3]]};

						DirectX::XMFLOAT4 new_weights;
						{
							DirectX::XMFLOAT4 original_weights(in_vertices[original_vertex_index].m_bone_weights[0], in_vertices[original_vertex_index].m_bone_weights[1], in_vertices[original_vertex_index].m_bone_weights[2], in_vertices[original_vertex_index].m_bone_weights[3]);

							DirectX::XMVECTOR simd_original_weights = DirectX::XMLoadFloat4(&original_weights);

							float const weight_summation = DirectX::XMVectorGetX(DirectX::XMVector4Dot(simd_original_weights, DirectX::XMVectorSplatOne()));

							if (weight_summation > 1E-6F)
							{
								assert(std::abs(weight_summation - 1.0F) < 5E-2F);

								DirectX::XMStoreFloat4(&new_weights, DirectX::XMVectorScale(simd_original_weights, (1.0F / weight_summation)));
							}
							else
							{
								assert(false);

								new_weights.x = 1.0F;
								new_weights.y = 0.0F;
								new_weights.z = 0.0F;
								new_weights.w = 0.0F;
							}
						}

						original_vertex_index_to_vertex_map.emplace_hint(found_vertex, original_vertex_index, internal_mmd_vertex_t{{new_position.x, new_position.y, new_position.z}, {new_normal.x, new_normal.y, new_normal.z}, {new_uv.x, new_uv.y}, {new_indices[0], new_indices[1], new_indices[2], new_indices[3]}, {new_weights.x, new_weights.y, new_weights.z, new_weights.w}});
					}
				}
			}

			for (auto const &original_vertex_index_and_vertex : original_vertex_index_to_vertex_map)
			{
				uint32_t const original_vertex_index = original_vertex_index_and_vertex.first;
				internal_mmd_vertex_t const &original_vertex = original_vertex_index_and_vertex.second;

				uint32_t const new_vertex_index = new_vertices.size();
				new_vertices.push_back(original_vertex);

				original_vertex_index_to_new_vertex_index[original_vertex_index] = new_vertex_index;
			}
		}

		mcrt_vector<mcrt_vector<internal_mmd_morph_target_vertex_t>> morph_targets_new_vertices;
		if (mesh_section_morph_switch)
		{
			morph_targets_new_vertices.resize(static_cast<size_t>(morph_target_count));

			for (uint32_t morph_target_index = 0U; morph_target_index < morph_target_count; ++morph_target_index)
			{
				mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t> const &original_morph_target_vertices = in_morph_targets_vertices[morph_target_index];
				assert(!original_morph_target_vertices.empty());

				// Initilize "0.0F" for NOT reference
				morph_targets_new_vertices[morph_target_index].resize(new_vertices.size(), internal_mmd_morph_target_vertex_t{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F}});

				for (auto const &original_vertex_index_and_morph_target_vertex : original_morph_target_vertices)
				{
					uint32_t const original_vertex_index = original_vertex_index_and_morph_target_vertex.first;
					internal_mmd_morph_target_vertex_t const &original_morph_target_vertex = original_vertex_index_and_morph_target_vertex.second;

					auto const found_new_vertex_index = original_vertex_index_to_new_vertex_index.find(original_vertex_index);
					if (original_vertex_index_to_new_vertex_index.end() != found_new_vertex_index)
					{
						assert(original_vertex_index == found_new_vertex_index->first);
						uint32_t const new_vertex_index = found_new_vertex_index->second;

						morph_targets_new_vertices[morph_target_index][new_vertex_index] = original_morph_target_vertex;
					}
					else
					{
						// TODO: should be found in other morph mesh sections
					}
				}
			}
		}
		else
		{
#ifndef NDEBUG
			for (uint32_t morph_target_index = 0U; morph_target_index < morph_target_count; ++morph_target_index)
			{
				mcrt_map<uint32_t, internal_mmd_morph_target_vertex_t> const &original_morph_target_vertices = in_morph_targets_vertices[morph_target_index];
				assert(!original_morph_target_vertices.empty());

				for (auto const &original_vertex_index_and_morph_target_vertex : original_morph_target_vertices)
				{
					uint32_t const original_vertex_index = original_vertex_index_and_morph_target_vertex.first;

					auto const found_new_vertex_index = original_vertex_index_to_new_vertex_index.find(original_vertex_index);
					assert(original_vertex_index_to_new_vertex_index.end() == found_new_vertex_index);
				}
			}
#endif
		}

		mcrt_vector<uint32_t> new_indices;
		for (uint32_t const mesh_section_face_index : mesh_section_face_indices)
		{
			// cw to ccw
			for (uint32_t face_vertex_index_plus_one = 3U; face_vertex_index_plus_one > 0U; --face_vertex_index_plus_one)
			{
				assert(face_vertex_index_plus_one >= 1U);
				uint32_t const face_vertex_index = face_vertex_index_plus_one - 1U;

				uint32_t const original_vertex_index = in_faces[mesh_section_face_index].m_vertex_indices[face_vertex_index];

				auto const found_new_vertex_index = original_vertex_index_to_new_vertex_index.find(original_vertex_index);
				assert(original_vertex_index_to_new_vertex_index.end() != found_new_vertex_index);

				assert(original_vertex_index == found_new_vertex_index->first);
				uint32_t const new_vertex_index = found_new_vertex_index->second;

				new_indices.push_back(new_vertex_index);
			}
		}

		mcrt_string texture_path;
		if (in_materials[mesh_section_material_index].m_texture_index < in_textures.size())
		{
			texture_path = in_textures[in_materials[mesh_section_material_index].m_texture_index].m_path;
			std::replace(texture_path.begin(), texture_path.end(), '\\', '/');
		}
		else
		{
			assert(BRX_ASSET_IMPORT_UINT32_INDEX_INVALID == in_materials[mesh_section_material_index].m_texture_index);
			assert(texture_path.empty());
		}

		out_mesh_sections[mesh_section_index] = internal_mmd_mesh_section_t{std::move(new_vertices), std::move(morph_targets_new_vertices), std::move(new_indices), in_materials[mesh_section_material_index].m_is_double_sided, {in_materials[mesh_section_material_index].m_diffuse.m_x, in_materials[mesh_section_material_index].m_diffuse.m_y, in_materials[mesh_section_material_index].m_diffuse.m_z, in_materials[mesh_section_material_index].m_diffuse.m_w}, std::move(texture_path)};
	}

#ifndef NDEBUG
	{
		uint32_t mesh_sections_vertex_count = 0U;
		uint32_t mesh_sections_index_count = 0U;

		for (uint32_t mesh_section_index = 0U; mesh_section_index < mesh_section_count; ++mesh_section_index)
		{
			mesh_sections_vertex_count = (mesh_sections_vertex_count + out_mesh_sections[mesh_section_index].m_vertices.size());
			mesh_sections_index_count = (mesh_sections_index_count + out_mesh_sections[mesh_section_index].m_indices.size());
		}

		assert((mesh_sections_vertex_count >= vertex_count) && (mesh_sections_vertex_count <= (vertex_count + (vertex_count / 3U))));
		assert((3U * face_count) == mesh_sections_index_count);
	}
#endif
}

// [ImportPmx.scale](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/operators/fileio.py#L94)
constexpr float const INTERNAL_IMPORT_PMX_SCALE = 0.1F;

// MMD
// LH
// Up +Y
// Forward -Z
// Right -X

// glTF
// RH
// Up +Y
// Forward Z
// Right -X

static inline mmd_pmx_vec3_t internal_transform_translation(mmd_pmx_vec3_t const &v)
{
	return mmd_pmx_vec3_t{v.m_x * INTERNAL_IMPORT_PMX_SCALE, v.m_y * INTERNAL_IMPORT_PMX_SCALE, -v.m_z * INTERNAL_IMPORT_PMX_SCALE};
}

static inline mmd_pmx_vec3_t internal_transform_normal(mmd_pmx_vec3_t const &v)
{
	return mmd_pmx_vec3_t{v.m_x, v.m_y, -v.m_z};
}

static inline mmd_pmx_vec4_t internal_transform_rotation(mmd_pmx_vec3_t const &v)
{
	DirectX::XMFLOAT4 q;
	{
		// YXZ
		// [FnRigidBody.new_rigid_body_object](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/rigid_body.py#L104)
		// [FnRigidBody.new_joint_object](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/rigid_body.py#L202)
		DirectX::XMMATRIX r = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationZ(v.m_z), DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(v.m_x), DirectX::XMMatrixRotationY(v.m_y)));

		DirectX::XMMATRIX z = DirectX::XMMatrixScaling(1.0F, 1.0F, -1.0F);

		DirectX::XMStoreFloat4(&q, DirectX::XMQuaternionNormalize(DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(z, r), z))));
	}

	return mmd_pmx_vec4_t{q.x, q.y, q.z, q.w};
}

static inline mmd_pmx_vec3_t internal_transform_translation_limit(mmd_pmx_vec3_t const &v)
{
	return mmd_pmx_vec3_t{v.m_x * INTERNAL_IMPORT_PMX_SCALE, v.m_y * INTERNAL_IMPORT_PMX_SCALE, -v.m_z * INTERNAL_IMPORT_PMX_SCALE};
}

static inline mmd_pmx_vec3_t internal_transform_rotation_limit(mmd_pmx_vec3_t const &v)
{
	// TODO: we do NOT need to change anything ???
	return v;
}

static inline mmd_pmx_vec3_t internal_transform_shape_size(mmd_pmx_vec3_t const &v)
{
	return mmd_pmx_vec3_t{v.m_x * INTERNAL_IMPORT_PMX_SCALE, v.m_y * INTERNAL_IMPORT_PMX_SCALE, -v.m_z * INTERNAL_IMPORT_PMX_SCALE};
}

static inline float internal_asin(float y, float z)
{
	constexpr float const INTERNAL_EPSILON = 1E-6F;
	constexpr float const INTERNAL_PI = 3.14159265358979323846264338327950288F;

	if (z > INTERNAL_EPSILON)
	{
		if (std::abs(y) > INTERNAL_EPSILON)
		{
			float const sin = (y / z);
			return DirectX::XMScalarASin(sin);
		}
		else
		{
			return 0.0F;
		}
	}
	else
	{
		if (y > INTERNAL_EPSILON)
		{
			return (INTERNAL_PI * 0.5F);
		}
		else if (y < INTERNAL_EPSILON)
		{
			return (0.0F - (INTERNAL_PI * 0.5F));
		}
		else
		{
			return 0.0F;
		}
	}
}
