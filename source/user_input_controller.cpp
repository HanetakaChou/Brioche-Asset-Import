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

#include "user_input_controller.h"
#include "../thirdparty/Brioche-ImGui/imgui.h"
#include "../thirdparty/Brioche-Asset-Import/include/brx_asset_import_input_stream.h"

static_assert(BRX_ANARI_UINT32_INDEX_INVALID == BRX_ASSET_IMPORT_UINT32_INDEX_INVALID, "");

static inline BRX_ANARI_MORPH_TARGET_NAME const wrap(BRX_ASSET_IMPORT_MORPH_TARGET_NAME const morph_target_name)
{
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_BROW_HAPPY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_HAPPY), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_BROW_ANGRY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_BROW_SAD) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SAD), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_BROW_SURPRISED) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SURPRISED), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_BLINK) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_BLINK_L) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_BLINK_R) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_HAPPY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_HAPPY), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_ANGRY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_ANGRY), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_SAD) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SAD), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_EYE_SURPRISED) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SURPRISED), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_A) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_I) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_I), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_U) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_U), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_E) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_E), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_O) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_O), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_SAD) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SAD), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED), "");
    static_assert(static_cast<uint32_t>(BRX_ANARI_MORPH_TARGET_NAME_MMD_COUNT) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT), "");
    return static_cast<BRX_ANARI_MORPH_TARGET_NAME const>(morph_target_name);
}

static inline brx_anari_surface_vertex_position const *wrap(brx_asset_import_surface_vertex_position const *surface_vertex_position)
{
    static_assert(sizeof(brx_anari_surface_vertex_position) == sizeof(brx_asset_import_surface_vertex_position), "");
    static_assert(offsetof(brx_anari_surface_vertex_position, m_position) == offsetof(brx_asset_import_surface_vertex_position, m_position), "");
    return reinterpret_cast<brx_anari_surface_vertex_position const *>(surface_vertex_position);
}

static inline brx_anari_surface_vertex_varying const *wrap(brx_asset_import_surface_vertex_varying const *surface_vertex_varying)
{
    static_assert(sizeof(brx_anari_surface_vertex_varying) == sizeof(brx_asset_import_surface_vertex_varying), "");
    static_assert(offsetof(brx_anari_surface_vertex_varying, m_normal) == offsetof(brx_asset_import_surface_vertex_varying, m_normal), "");
    static_assert(offsetof(brx_anari_surface_vertex_varying, m_tangent) == offsetof(brx_asset_import_surface_vertex_varying, m_tangent), "");
    static_assert(offsetof(brx_anari_surface_vertex_varying, m_texcoord) == offsetof(brx_asset_import_surface_vertex_varying, m_texcoord), "");
    return reinterpret_cast<brx_anari_surface_vertex_varying const *>(surface_vertex_varying);
}

static inline brx_anari_surface_vertex_blending const *wrap(brx_asset_import_surface_vertex_blending const *surface_vertex_blending)
{
    static_assert(sizeof(brx_anari_surface_vertex_blending) == sizeof(brx_asset_import_surface_vertex_blending), "");
    static_assert(offsetof(brx_anari_surface_vertex_blending, m_indices) == offsetof(brx_asset_import_surface_vertex_blending, m_indices), "");
    static_assert(offsetof(brx_anari_surface_vertex_blending, m_weights) == offsetof(brx_asset_import_surface_vertex_blending, m_weights), "");
    return reinterpret_cast<brx_anari_surface_vertex_blending const *>(surface_vertex_blending);
}

static inline brx_anari_vec3 const wrap(brx_asset_import_vec3 const vec3)
{
    static_assert(sizeof(brx_anari_vec3) == sizeof(brx_asset_import_vec3), "");
    static_assert(offsetof(brx_anari_vec3, m_x) == offsetof(brx_asset_import_vec3, m_x), "");
    static_assert(offsetof(brx_anari_vec3, m_y) == offsetof(brx_asset_import_vec3, m_y), "");
    static_assert(offsetof(brx_anari_vec3, m_z) == offsetof(brx_asset_import_vec3, m_z), "");
    return (*reinterpret_cast<brx_anari_vec3 const *>(&vec3));
}

static inline brx_anari_vec4 const wrap(brx_asset_import_vec4 const vec4)
{
    static_assert(sizeof(brx_anari_vec4) == sizeof(brx_asset_import_vec4), "");
    static_assert(offsetof(brx_anari_vec4, m_x) == offsetof(brx_asset_import_vec4, m_x), "");
    static_assert(offsetof(brx_anari_vec4, m_y) == offsetof(brx_asset_import_vec4, m_y), "");
    static_assert(offsetof(brx_anari_vec4, m_z) == offsetof(brx_asset_import_vec4, m_z), "");
    static_assert(offsetof(brx_anari_vec4, m_w) == offsetof(brx_asset_import_vec4, m_w), "");
    return (*reinterpret_cast<brx_anari_vec4 const *>(&vec4));
}

static_assert(BRX_MOTION_UINT32_INDEX_INVALID == BRX_ASSET_IMPORT_UINT32_INDEX_INVALID, "");

static inline BRX_MOTION_MORPH_TARGET_NAME const *wrap(BRX_ASSET_IMPORT_MORPH_TARGET_NAME const *morph_target_name)
{
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_BROW_HAPPY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_HAPPY), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_BROW_ANGRY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_BROW_SAD) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SAD), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_BROW_SURPRISED) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SURPRISED), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_BLINK) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_BLINK_L) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_BLINK_R) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_HAPPY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_HAPPY), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_ANGRY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_ANGRY), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_SAD) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SAD), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_EYE_SURPRISED) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SURPRISED), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_A) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_I) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_I), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_U) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_U), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_E) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_E), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_O) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_O), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_SAD) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SAD), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_MORPH_TARGET_NAME_MMD_COUNT) == static_cast<uint32_t>(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT), "");
    return reinterpret_cast<BRX_MOTION_MORPH_TARGET_NAME const *>(morph_target_name);
}

static inline BRX_MOTION_SKELETON_JOINT_NAME const *wrap(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *skeleton_joint_name)
{
    static_assert(static_cast<uint32_t>(BRX_MOTION_SKELETON_JOINT_NAME_MMD_CONTROL_NODE) == static_cast<uint32_t>(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CONTROL_NODE), "");
    return reinterpret_cast<BRX_MOTION_SKELETON_JOINT_NAME const *>(skeleton_joint_name);
}

static inline BRX_MOTION_SKELETON_JOINT_CONSTRAINT_NAME const *wrap(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *skeleton_joint_constraint_name)
{
    static_assert(static_cast<uint32_t>(BRX_MOTION_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_ANKLE) == static_cast<uint32_t>(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_ANKLE), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_TOE_TIP) == static_cast<uint32_t>(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_TOE_TIP), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_ANKLE) == static_cast<uint32_t>(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_ANKLE), "");
    static_assert(static_cast<uint32_t>(BRX_MOTION_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_TOE_TIP) == static_cast<uint32_t>(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_TOE_TIP), "");
    return reinterpret_cast<BRX_MOTION_SKELETON_JOINT_CONSTRAINT_NAME const *>(skeleton_joint_constraint_name);
}

static inline brx_motion_rigid_transform const *wrap(brx_asset_import_rigid_transform const *rigid_transform)
{
    static_assert(sizeof(brx_motion_rigid_transform) == sizeof(brx_asset_import_rigid_transform), "");
    static_assert(offsetof(brx_motion_rigid_transform, m_rotation) == offsetof(brx_asset_import_rigid_transform, m_rotation), "");
    static_assert(offsetof(brx_motion_rigid_transform, m_translation) == offsetof(brx_asset_import_rigid_transform, m_translation), "");
    return reinterpret_cast<brx_motion_rigid_transform const *>(rigid_transform);
}

static inline brx_motion_skeleton_joint_constraint const *wrap(brx_asset_import_skeleton_joint_constraint const *skeleton_joint_constraint)
{
    static_assert(sizeof(brx_motion_skeleton_joint_constraint) == sizeof(brx_asset_import_skeleton_joint_constraint), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_constraint_type) == offsetof(brx_asset_import_skeleton_joint_constraint, m_constraint_type), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform.m_source_joint_index) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform.m_source_joint_index), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform.m_source_weight_count) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform.m_source_weight_count), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform.m_source_weights) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform.m_source_weights), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform.m_destination_joint_index) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform.m_destination_joint_index), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform.m_copy_rotation) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform.m_copy_rotation), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_copy_transform.m_copy_translation) == offsetof(brx_asset_import_skeleton_joint_constraint, m_copy_transform.m_copy_translation), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_ik_end_effector_index) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_ik_end_effector_index), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_ik_joint_count) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_ik_joint_count), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_ik_joint_indices) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_ik_joint_indices), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_target_joint_index) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_target_joint_index), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_ik_two_joints_hinge_joint_axis_local_space) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_ik_two_joints_hinge_joint_axis_local_space), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_cosine_max_ik_two_joints_hinge_joint_angle) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_cosine_max_ik_two_joints_hinge_joint_angle), "");
    static_assert(offsetof(brx_motion_skeleton_joint_constraint, m_inverse_kinematics.m_cosine_min_ik_two_joints_hinge_joint_angle) == offsetof(brx_asset_import_skeleton_joint_constraint, m_inverse_kinematics.m_cosine_min_ik_two_joints_hinge_joint_angle), "");
    return reinterpret_cast<brx_motion_skeleton_joint_constraint const *>(skeleton_joint_constraint);
}

static inline brx_motion_physics_rigid_body const *wrap(brx_asset_import_physics_rigid_body const *physics_rigid_body)
{
    static_assert(sizeof(brx_motion_physics_rigid_body) == sizeof(brx_asset_import_physics_rigid_body), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_model_space_transform) == offsetof(brx_asset_import_physics_rigid_body, m_model_space_transform), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_shape_type) == offsetof(brx_asset_import_physics_rigid_body, m_shape_type), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_shape_size) == offsetof(brx_asset_import_physics_rigid_body, m_shape_size), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_motion_type) == offsetof(brx_asset_import_physics_rigid_body, m_motion_type), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_collision_filter_group) == offsetof(brx_asset_import_physics_rigid_body, m_collision_filter_group), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_collision_filter_mask) == offsetof(brx_asset_import_physics_rigid_body, m_collision_filter_mask), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_mass) == offsetof(brx_asset_import_physics_rigid_body, m_mass), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_linear_damping) == offsetof(brx_asset_import_physics_rigid_body, m_linear_damping), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_angular_damping) == offsetof(brx_asset_import_physics_rigid_body, m_angular_damping), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_friction) == offsetof(brx_asset_import_physics_rigid_body, m_friction), "");
    static_assert(offsetof(brx_motion_physics_rigid_body, m_restitution) == offsetof(brx_asset_import_physics_rigid_body, m_restitution), "");
    return reinterpret_cast<brx_motion_physics_rigid_body const *>(physics_rigid_body);
}

static inline brx_motion_physics_constraint const *wrap(brx_asset_import_physics_constraint const *physics_constrain)
{
    static_assert(sizeof(brx_motion_physics_constraint) == sizeof(brx_asset_import_physics_constraint), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_rigid_body_a_index) == offsetof(brx_asset_import_physics_constraint, m_rigid_body_a_index), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_rigid_body_b_index) == offsetof(brx_asset_import_physics_constraint, m_rigid_body_b_index), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_constraint_type) == offsetof(brx_asset_import_physics_constraint, m_constraint_type), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_pivot) == offsetof(brx_asset_import_physics_constraint, m_pivot), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_twist_axis) == offsetof(brx_asset_import_physics_constraint, m_twist_axis), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_plane_axis) == offsetof(brx_asset_import_physics_constraint, m_plane_axis), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_normal_axis) == offsetof(brx_asset_import_physics_constraint, m_normal_axis), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_twist_limit) == offsetof(brx_asset_import_physics_constraint, m_twist_limit), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_plane_limit) == offsetof(brx_asset_import_physics_constraint, m_plane_limit), "");
    static_assert(offsetof(brx_motion_physics_constraint, m_normal_limit) == offsetof(brx_asset_import_physics_constraint, m_normal_limit), "");
    return reinterpret_cast<brx_motion_physics_constraint const *>(physics_constrain);
}

static inline brx_motion_ragdoll_direct_mapping const *wrap(brx_asset_import_ragdoll_direct_mapping const *ragdoll_direct_mapping)
{
    static_assert(sizeof(brx_motion_ragdoll_direct_mapping) == sizeof(brx_asset_import_ragdoll_direct_mapping), "");
    static_assert(offsetof(brx_motion_ragdoll_direct_mapping, m_joint_index_a) == offsetof(brx_asset_import_ragdoll_direct_mapping, m_joint_index_a), "");
    static_assert(offsetof(brx_motion_ragdoll_direct_mapping, m_joint_index_b) == offsetof(brx_asset_import_ragdoll_direct_mapping, m_joint_index_b), "");
    static_assert(offsetof(brx_motion_ragdoll_direct_mapping, m_a_to_b_transform_model_space) == offsetof(brx_asset_import_ragdoll_direct_mapping, m_a_to_b_transform_model_space), "");
    return reinterpret_cast<brx_motion_ragdoll_direct_mapping const *>(ragdoll_direct_mapping);
}

extern bool _internal_platform_get_open_file_name(void *platform_context, size_t filter_count, char const *const *filter_names, char const *const *filter_specs, int &inout_file_type_index, mcrt_string *out_file_name, uint64_t *out_file_timestamp, mcrt_vector<uint8_t> *out_file_data);

extern bool _internal_platform_get_file_timestamp_and_data(char const *file_name, uint64_t *out_file_timestamp, mcrt_vector<uint8_t> *out_file_data);

extern uint64_t _internal_tick_count_now();

static inline brx_anari_image *_internal_load_asset_image(uint8_t const *asset_image_url, char const *asset_model_directory_name, bool force_srgb, brx_anari_device *device, ui_model_t *ui_model);

static inline brx_anari_image *_internal_load_asset_image_file(char const *asset_image_file_name, uint64_t asset_image_file_timestamp, void const *asset_image_file_data_base, size_t asset_image_file_data_size, bool force_srgb, brx_anari_device *device, ui_model_t *ui_model);

extern void ui_controller_init(ui_controller_t *ui_controller)
{
    constexpr size_t const MAX_INPUT_TEXT_SIZE = 4096U;

    ui_controller->m_language_index = 0;

    ui_controller->m_import_asset_image_force_srgb = true;
    ui_controller->m_import_asset_image_get_open_file_name_file_type_index = 1;
    ui_controller->m_selected_asset_image.clear();

    ui_controller->m_import_asset_model_get_open_file_name_file_type_index = 1;
    ui_controller->m_selected_asset_model.clear();

    ui_controller->m_import_asset_motion_get_open_file_name_file_type_index = 1;
    ui_controller->m_selected_asset_motion.clear();

    ui_controller->m_new_instance_motion_selected_asset_motion.clear();
    ui_controller->m_new_instance_motion_selected_animation_index = -1;
    ui_controller->m_new_instance_motion_name.resize(MAX_INPUT_TEXT_SIZE);

    ui_controller->m_selected_instance_motion = -1;
}

extern void ui_simulate(void *platform_context, brx_anari_device *device, ui_model_t *ui_model, ui_controller_t *ui_controller)
{
    ImGuiIO const &io = ImGui::GetIO();

    constexpr int const LANGUAGE_COUNT = 4;

    constexpr float const ui_width = 320.0F;
    constexpr float const ui_height = 200.0F;

    ImGui::SetNextWindowSize(ImVec2(ui_width, ui_height), ImGuiCond_FirstUseEver);

    ImGui::Begin("Brioche Asset Import (ESC Enable/Disable UI)");

    {
        constexpr char const *const text[LANGUAGE_COUNT] = {
            "Language",
            "言語",
            "語言",
            "语言"};

        ImGui::TextUnformatted(text[ui_controller->m_language_index]);

        ImGui::SameLine();

        constexpr char const *const items[LANGUAGE_COUNT] = {
            "English",
            "日本語",
            "華語",
            "华语"};

        ImGui::Combo("##Language", &ui_controller->m_language_index, items, IM_ARRAYSIZE(items));
    }

    {
        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Rendering FPS:",
                "Rendering FPS:",
                "渲染 FPS:",
                "渲染 FPS:"};
            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
        }

        ImGui::SameLine();

        {
            char fps_text[] = {"18446744073709551615"};
            std::snprintf(fps_text, sizeof(fps_text) / sizeof(fps_text[0]), "%llu", static_cast<long long unsigned>(io.Framerate));
            fps_text[(sizeof(fps_text) / sizeof(fps_text[0])) - 1] = '\0';

            ImGui::TextUnformatted(fps_text);
        }
    }

    {
        ImGui::Separator();

        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Asset Motion Manager",
                "資源行動管理",
                "資源動作管理",
                "资源动作管理"};
            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
        }

        ImGui::SameLine();

        if (ImGui::TreeNodeEx("##Asset-Motion-Manager", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoAutoOpenOnLog))
        {
            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Import",
                    "輸入",
                    "導入",
                    "导入"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                if (ImGui::Button("O##Asset-Motion-Manager-Import"))
                {
                    bool asset_motion_file_open;
                    mcrt_string asset_motion_file_name;
                    uint64_t asset_motion_file_timestamp;
                    mcrt_vector<uint8_t> asset_motion_file_data;
                    {
                        constexpr size_t const asset_motion_filter_count = 2;

                        // TODO
                        // "glTF Binary"    "*.glb;*.vrma"
                        // "glTF Separate"  "*.gltf"

                        constexpr char const *const asset_motion_filter_names[asset_motion_filter_count] = {
                            "All Files",
                            "MMD Motion Data"};

                        constexpr char const *const asset_motion_filter_specs[asset_motion_filter_count] = {
                            "*.*",
                            "*.vmd"};

                        asset_motion_file_open = _internal_platform_get_open_file_name(platform_context, asset_motion_filter_count, asset_motion_filter_names, asset_motion_filter_specs, ui_controller->m_import_asset_motion_get_open_file_name_file_type_index, &asset_motion_file_name, &asset_motion_file_timestamp, &asset_motion_file_data);
                    }

                    if (asset_motion_file_open)
                    {
                        assert(!asset_motion_file_name.empty());
                        assert(0U != asset_motion_file_timestamp);
                        assert(!asset_motion_file_data.empty());

                        mcrt_string asset_motion_file_identity;
                        {
                            char asset_motion_file_timestamp_text[] = {"18446744073709551615"};
                            std::snprintf(asset_motion_file_timestamp_text, sizeof(asset_motion_file_timestamp_text) / sizeof(asset_motion_file_timestamp_text[0]), "%llu", static_cast<long long unsigned>(asset_motion_file_timestamp));
                            asset_motion_file_timestamp_text[(sizeof(asset_motion_file_timestamp_text) / sizeof(asset_motion_file_timestamp_text[0])) - 1] = '\0';

                            asset_motion_file_identity += asset_motion_file_timestamp_text;
                            asset_motion_file_identity += ' ';
                            asset_motion_file_identity += asset_motion_file_name;
                        }

                        auto const &found_asset_motion = ui_model->m_asset_motions.find(asset_motion_file_identity);

                        if (ui_model->m_asset_motions.end() == found_asset_motion)
                        {
                            brx_asset_import_scene *const asset_import_scene = brx_asset_import_create_scene_from_memory(asset_motion_file_data.data(), asset_motion_file_data.size());
                            if (NULL != asset_import_scene)
                            {
                                bool success = true;
                                uint32_t const animation_count = asset_import_scene->get_animation_count();
                                mcrt_vector<brx_motion_animation *> animations(static_cast<size_t>(animation_count), NULL);
                                for (uint32_t animation_index = 0U; animation_index < animation_count; ++animation_index)
                                {
                                    brx_asset_import_animation const *const asset_import_animation = asset_import_scene->get_animation(animation_index);

                                    brx_motion_animation *const motion_animation = brx_motion_create_animation(asset_import_animation->get_frame_count(), asset_import_animation->get_weight_channel_count(), wrap(asset_import_animation->get_weight_channel_names()), asset_import_animation->get_weights(), asset_import_animation->get_rigid_transform_channel_count(), wrap(asset_import_animation->get_rigid_transform_channel_names()), wrap(asset_import_animation->get_rigid_transforms()), asset_import_animation->get_switch_channel_count(), wrap(asset_import_animation->get_switch_channel_names()), asset_import_animation->get_switches());

                                    if (NULL != motion_animation)
                                    {
                                        animations[animation_index] = motion_animation;
                                    }
                                    else
                                    {
                                        success = false;
                                        break;
                                    }
                                }

                                if (success)
                                {
                                    for (brx_motion_animation *const animation : animations)
                                    {
                                        assert(NULL != animation);
                                    }

                                    std::pair<mcrt_string, ui_asset_motion_model_2_t> motion_model(asset_motion_file_identity, ui_asset_motion_model_2_t{std::move(animations)});
                                    ui_model->m_asset_motions.insert(found_asset_motion, std::move(motion_model));
                                }
                                else
                                {
                                    assert(false);

                                    for (brx_motion_animation *const animation : animations)
                                    {
                                        if (NULL != animation)
                                        {
                                            brx_motion_destroy_animation(animation);
                                        }
                                    }

                                    animations.clear();
                                }

                                brx_asset_import_destroy_scene(asset_import_scene);
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        else
                        {
                            assert(!found_asset_motion->second.m_animations.empty());
                        }
                    }
                }
            }

            ImGui::Separator();

            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Delete",
                    "削除",
                    "刪除",
                    "删除"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                if (ImGui::Button("X##Asset-Motion-Manager-Delete"))
                {
                    auto const &found_asset_motion = ui_model->m_asset_motions.find(ui_controller->m_selected_asset_motion);
                    if (ui_model->m_asset_motions.end() != found_asset_motion)
                    {
                        assert(!found_asset_motion->second.m_animations.empty());

                        for (brx_motion_animation *const animation : found_asset_motion->second.m_animations)
                        {
                            if (NULL != animation)
                            {
                                brx_motion_destroy_animation(animation);
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        found_asset_motion->second.m_animations.clear();

                        ui_model->m_asset_motions.erase(found_asset_motion);
                        ui_controller->m_selected_asset_motion.clear();
                    }
                }
            }

            ImGui::Separator();

            if (ImGui::BeginChild("##Asset-Motion-Manager-Left-Child", ImVec2(ui_width * 0.5F, 0.0F), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
            {
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                ImGuiTextFilter text_filter;
                {
                    constexpr char const *const hint[LANGUAGE_COUNT] = {
                        "Search",
                        "検索",
                        "檢索",
                        "检索"};
                    if (ImGui::InputTextWithHint("##Asset-Motion-Manager-Left-Child-Text-Filter", hint[ui_controller->m_language_index], text_filter.InputBuf, IM_ARRAYSIZE(text_filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                    {
                        text_filter.Build();
                    }
                }

                ImGui::PopItemFlag();

                if (ImGui::BeginTable("##Asset-Motion-Manager-Left-Child-Table", 1, ImGuiTableFlags_RowBg))
                {
                    for (auto const &asset_motion : ui_model->m_asset_motions)
                    {
                        if (text_filter.PassFilter(asset_motion.first.c_str()))
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();

                            ImGuiTreeNodeFlags const flags = ((ui_controller->m_selected_asset_motion != asset_motion.first)) ? ImGuiTreeNodeFlags_Leaf : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected);

                            bool const node_open = ImGui::TreeNodeEx(asset_motion.first.c_str(), flags);

                            if (ImGui::IsItemFocused())
                            {
                                ui_controller->m_selected_asset_motion = asset_motion.first;
                            }

                            if (node_open)
                            {
                                ImGui::TreePop();
                            }
                        }
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            auto const &found_asset_motion = ui_model->m_asset_motions.find(ui_controller->m_selected_asset_motion);

            if (ui_model->m_asset_motions.end() != found_asset_motion)
            {
                ImGui::SameLine();

                ImGui::BeginGroup();

                mcrt_string timestamp_text = "Timestamp: ";
                mcrt_string directory_name;
                mcrt_string file_name;
                {
                    size_t const timestamp_text_end_pos = found_asset_motion->first.find(' ');
                    size_t const directory_name_end_pos = found_asset_motion->first.find_last_of("/\\");
                    if ((mcrt_string::npos != timestamp_text_end_pos) && (mcrt_string::npos != directory_name_end_pos) && (timestamp_text_end_pos < directory_name_end_pos))
                    {
                        timestamp_text += found_asset_motion->first.substr(0U, timestamp_text_end_pos);
                        directory_name = found_asset_motion->first.substr(timestamp_text_end_pos + 1U, ((directory_name_end_pos - timestamp_text_end_pos) - 1U));
                        file_name = found_asset_motion->first.substr(directory_name_end_pos + 1U);
                    }
                    else
                    {
                        assert(false);
                        timestamp_text = "N/A";
                        directory_name = "N/A";
                        file_name = "N/A";
                    }
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(file_name.c_str());
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(directory_name.c_str());
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(timestamp_text.c_str());
                    ImGui::PopStyleColor();
                }

                ImGui::Separator();

                if (ImGui::BeginTable("##Asset-Motion-Manager-Right-Group-Table", 2, ImGuiTableFlags_BordersInnerV))
                {
                    ImGui::TableSetupColumn("##Asset-Motion-Manager-Right-Group-Table-Property", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("##Asset-Motion-Manager-Right-Group-Table-Value", ImGuiTableColumnFlags_WidthStretch, 2.0F);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    {
                        constexpr char const *const text[LANGUAGE_COUNT] = {
                            "Animation Count",
                            "Animation 数",
                            "動畫數",
                            "动画数"};
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                        ImGui::TextUnformatted(text[ui_controller->m_language_index]);
                        ImGui::PopStyleColor();
                    }
                    ImGui::TableNextColumn();
                    {
                        assert(!found_asset_motion->second.m_animations.empty());
                        uint32_t const animation_count = static_cast<uint32_t>(found_asset_motion->second.m_animations.size());

                        char animation_count_text[] = {"18446744073709551615"};
                        std::snprintf(animation_count_text, sizeof(animation_count_text) / sizeof(animation_count_text[0]), "%llu", static_cast<long long unsigned>(animation_count));
                        animation_count_text[(sizeof(animation_count_text) / sizeof(animation_count_text[0])) - 1] = '\0';

                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                        ImGui::TextUnformatted(animation_count_text);
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndTable();
                }

                ImGui::EndGroup();
            }

            ImGui::TreePop();
        }
    }

    {
        ImGui::Separator();

        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Asset Model Manager",
                "資源型式管理",
                "資源模型管理",
                "资源模型管理"};
            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
        }

        ImGui::SameLine();

        if (ImGui::TreeNodeEx("##Asset-Model-Manager", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoAutoOpenOnLog))
        {
            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Import",
                    "輸入",
                    "導入",
                    "导入"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                if (ImGui::Button("O##Asset-Model-Manager-Import"))
                {
                    bool asset_model_file_open;
                    mcrt_string asset_model_file_name;
                    uint64_t asset_model_file_timestamp;
                    mcrt_vector<uint8_t> asset_model_file_data;
                    {
                        constexpr size_t const asset_model_filter_count = 2;

                        // TODO
                        // "glTF Binary"    "*.glb;*.vrm"
                        // "glTF Separate"  "*.gltf"

                        constexpr char const *const asset_model_filter_names[asset_model_filter_count] = {
                            "All Files",
                            "MMD Model Data"};

                        constexpr char const *const asset_model_filter_specs[asset_model_filter_count] = {
                            "*.*",
                            "*.pmx"};

                        asset_model_file_open = _internal_platform_get_open_file_name(platform_context, asset_model_filter_count, asset_model_filter_names, asset_model_filter_specs, ui_controller->m_import_asset_model_get_open_file_name_file_type_index, &asset_model_file_name, &asset_model_file_timestamp, &asset_model_file_data);
                    }

                    if (asset_model_file_open)
                    {
                        assert(!asset_model_file_name.empty());
                        assert(0U != asset_model_file_timestamp);
                        assert(!asset_model_file_data.empty());

                        mcrt_string asset_model_file_identity;
                        {
                            char asset_model_file_timestamp_text[] = {"18446744073709551615"};
                            std::snprintf(asset_model_file_timestamp_text, sizeof(asset_model_file_timestamp_text) / sizeof(asset_model_file_timestamp_text[0]), "%llu", static_cast<long long unsigned>(asset_model_file_timestamp));
                            asset_model_file_timestamp_text[(sizeof(asset_model_file_timestamp_text) / sizeof(asset_model_file_timestamp_text[0])) - 1] = '\0';

                            asset_model_file_identity += asset_model_file_timestamp_text;
                            asset_model_file_identity += ' ';
                            asset_model_file_identity += asset_model_file_name;
                        }

                        auto const &found_asset_model = ui_model->m_asset_models.find(asset_model_file_identity);

                        if (ui_model->m_asset_models.end() == found_asset_model)
                        {
                            brx_asset_import_scene *const asset_import_scene = brx_asset_import_create_scene_from_memory(asset_model_file_data.data(), asset_model_file_data.size());
                            if (NULL != asset_import_scene)
                            {
                                mcrt_string asset_model_directory_name;
                                {
                                    size_t directory_name_end_pos = asset_model_file_name.find_last_of("/\\");
                                    if (mcrt_string::npos != directory_name_end_pos)
                                    {
                                        asset_model_directory_name = asset_model_file_name.substr(0U, directory_name_end_pos);
                                    }
                                    else
                                    {
                                        asset_model_directory_name = ".";
                                    }
                                }

                                bool success = true;
                                uint32_t const surface_group_count = asset_import_scene->get_surface_group_count();
                                mcrt_vector<brx_anari_surface_group *> surface_groups(static_cast<size_t>(surface_group_count), NULL);
                                mcrt_vector<brx_motion_skeleton *> skeletons(static_cast<size_t>(surface_group_count), NULL);
                                for (uint32_t surface_group_index = 0U; surface_group_index < surface_group_count; ++surface_group_index)
                                {
                                    brx_asset_import_surface_group const *const asset_import_surface_group = asset_import_scene->get_surface_group(surface_group_index);

                                    uint32_t const surface_count = asset_import_surface_group->get_surface_count();

                                    mcrt_vector<BRX_ANARI_SURFACE> surfaces(static_cast<size_t>(surface_count));

                                    for (uint32_t surface_index = 0U; surface_index < surface_count; ++surface_index)
                                    {
                                        brx_asset_import_surface const *const asset_import_surface = asset_import_surface_group->get_surface(surface_index);

                                        surfaces[surface_index].m_vertex_count = asset_import_surface->get_vertex_count();
                                        surfaces[surface_index].m_vertex_positions = wrap(asset_import_surface->get_vertex_positions());
                                        surfaces[surface_index].m_vertex_varyings = wrap(asset_import_surface->get_vertex_varyings());
                                        surfaces[surface_index].m_vertex_blendings = wrap(asset_import_surface->get_vertex_blendings());
                                        for (uint32_t morph_target_name_index = 0U; morph_target_name_index < BRX_ANARI_MORPH_TARGET_NAME_MMD_COUNT; ++morph_target_name_index)
                                        {
                                            surfaces[surface_index].m_morph_targets_vertex_positions[morph_target_name_index] = NULL;
                                            surfaces[surface_index].m_morph_targets_vertex_varyings[morph_target_name_index] = NULL;
                                        }
                                        for (uint32_t morph_target_index = 0U; morph_target_index < asset_import_surface->get_morph_target_count(); ++morph_target_index)
                                        {
                                            BRX_ANARI_MORPH_TARGET_NAME const morph_target_name = wrap(asset_import_surface->get_morph_target_name(morph_target_index));
                                            uint32_t const morph_target_name_index = morph_target_name;
                                            surfaces[surface_index].m_morph_targets_vertex_positions[morph_target_name_index] = wrap(asset_import_surface->get_morph_target_vertex_positions(morph_target_index));
                                            surfaces[surface_index].m_morph_targets_vertex_varyings[morph_target_name_index] = wrap(asset_import_surface->get_morph_target_vertex_varyings(morph_target_index));
                                        }
                                        surfaces[surface_index].m_index_count = asset_import_surface->get_index_count();
                                        surfaces[surface_index].m_indices = asset_import_surface->get_indices();
                                        surfaces[surface_index].m_emissive_image = _internal_load_asset_image(asset_import_surface->get_emissive_image_url(), asset_model_directory_name.c_str(), false, device, ui_model);
                                        surfaces[surface_index].m_emissive_factor = wrap(asset_import_surface->get_emissive_factor());
                                        surfaces[surface_index].m_normal_image = _internal_load_asset_image(asset_import_surface->get_normal_image_url(), asset_model_directory_name.c_str(), false, device, ui_model);
                                        surfaces[surface_index].m_normal_scale = asset_import_surface->get_normal_scale();
                                        surfaces[surface_index].m_base_color_image = _internal_load_asset_image(asset_import_surface->get_base_color_image_url(), asset_model_directory_name.c_str(), true, device, ui_model);
                                        surfaces[surface_index].m_base_color_factor = wrap(asset_import_surface->get_base_color_factor());
                                        surfaces[surface_index].m_metallic_roughness_image = _internal_load_asset_image(asset_import_surface->get_metallic_roughness_image_url(), asset_model_directory_name.c_str(), false, device, ui_model);
                                        surfaces[surface_index].m_metallic_factor = asset_import_surface->get_metallic_factor();
                                        surfaces[surface_index].m_roughness_factor = asset_import_surface->get_roughness_factor();
                                    }

                                    brx_anari_surface_group *const anari_surface_group = device->new_surface_group(surface_count, surfaces.data());

                                    uint32_t const animation_skeleton_joint_count = asset_import_surface_group->get_animation_skeleton_joint_count();

                                    brx_motion_skeleton *const motion_skeleton = brx_motion_create_skeleton(animation_skeleton_joint_count, wrap(asset_import_surface_group->get_animation_skeleton_joint_names()), asset_import_surface_group->get_animation_skeleton_joint_parent_indices(), wrap(asset_import_surface_group->get_animation_skeleton_joint_transforms_bind_pose_local_space()), asset_import_surface_group->get_animation_skeleton_joint_constraint_count(), wrap(asset_import_surface_group->get_animation_skeleton_joint_constraint_names()), wrap(asset_import_surface_group->get_animation_skeleton_joint_constraints()), asset_import_surface_group->get_ragdoll_skeleton_rigid_body_count(), wrap(asset_import_surface_group->get_ragdoll_skeleton_rigid_bodies()), asset_import_surface_group->get_ragdoll_skeleton_constraint_count(), wrap(asset_import_surface_group->get_ragdoll_skeleton_constraints()), asset_import_surface_group->get_animation_to_ragdoll_direct_mapping_count(), wrap(asset_import_surface_group->get_animation_to_ragdoll_direct_mappings()), asset_import_surface_group->get_ragdoll_to_animation_direct_mapping_count(), wrap(asset_import_surface_group->get_ragdoll_to_animation_direct_mappings()));

                                    if ((NULL != anari_surface_group) && (NULL != motion_skeleton))
                                    {
                                        surface_groups[surface_group_index] = anari_surface_group;
                                        skeletons[surface_group_index] = motion_skeleton;
                                    }
                                    else
                                    {
                                        success = false;
                                        break;
                                    }
                                }

                                if (success)
                                {
                                    for (brx_anari_surface_group *const surface_group : surface_groups)
                                    {
                                        assert(NULL != surface_group);
                                    }

                                    for (brx_motion_skeleton *const skeleton : skeletons)
                                    {
                                        assert(NULL != skeleton);
                                    }

                                    std::pair<mcrt_string, ui_asset_model_model_2_t> model_model(asset_model_file_identity, ui_asset_model_model_2_t{std::move(surface_groups), std::move(skeletons)});
                                    ui_model->m_asset_models.insert(found_asset_model, std::move(model_model));
                                }
                                else
                                {
                                    assert(false);

                                    for (brx_anari_surface_group *const surface_group : surface_groups)
                                    {
                                        if (NULL != surface_group)
                                        {
                                            device->release_surface_group(surface_group);
                                        }
                                    }
                                    surface_groups.clear();

                                    for (brx_motion_skeleton *const skeleton : skeletons)
                                    {
                                        if (NULL != skeleton)
                                        {
                                            brx_motion_destroy_skeleton(skeleton);
                                        }
                                    }
                                    skeletons.clear();
                                }

                                brx_asset_import_destroy_scene(asset_import_scene);
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        else
                        {
                            assert(!found_asset_model->second.m_surface_groups.empty());
                            assert(!found_asset_model->second.m_skeletons.empty());
                            assert(found_asset_model->second.m_surface_groups.size() == found_asset_model->second.m_skeletons.size());
                        }
                    }
                }
            }

            ImGui::Separator();

            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Delete",
                    "削除",
                    "刪除",
                    "删除"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                if (ImGui::Button("X##Asset-Model-Manager-Delete"))
                {
                    auto const &found_asset_model = ui_model->m_asset_models.find(ui_controller->m_selected_asset_model);
                    if (ui_model->m_asset_models.end() != found_asset_model)
                    {
                        assert(!found_asset_model->second.m_surface_groups.empty());
                        assert(!found_asset_model->second.m_skeletons.empty());
                        assert(found_asset_model->second.m_surface_groups.size() == found_asset_model->second.m_skeletons.size());

                        for (brx_anari_surface_group *const surface_group : found_asset_model->second.m_surface_groups)
                        {
                            if (NULL != surface_group)
                            {
                                device->release_surface_group(surface_group);
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        found_asset_model->second.m_surface_groups.clear();

                        for (brx_motion_skeleton *const skeleton : found_asset_model->second.m_skeletons)
                        {
                            if (NULL != skeleton)
                            {
                                brx_motion_destroy_skeleton(skeleton);
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        found_asset_model->second.m_skeletons.clear();

                        ui_model->m_asset_models.erase(found_asset_model);
                        ui_controller->m_selected_asset_model.clear();
                    }
                }
            }

            ImGui::Separator();

            if (ImGui::BeginChild("##Asset-Model-Manager-Left-Child", ImVec2(ui_width * 0.5F, 0.0F), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
            {
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                ImGuiTextFilter text_filter;
                {
                    constexpr char const *const hint[LANGUAGE_COUNT] = {
                        "Search",
                        "検索",
                        "檢索",
                        "检索"};
                    if (ImGui::InputTextWithHint("##Asset-Model-Manager-Left-Child-Text-Filter", hint[ui_controller->m_language_index], text_filter.InputBuf, IM_ARRAYSIZE(text_filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                    {
                        text_filter.Build();
                    }
                }

                ImGui::PopItemFlag();

                if (ImGui::BeginTable("##Asset-Model-Manager-Left-Child-Table", 1, ImGuiTableFlags_RowBg))
                {
                    for (auto const &asset_model : ui_model->m_asset_models)
                    {
                        if (text_filter.PassFilter(asset_model.first.c_str()))
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();

                            ImGuiTreeNodeFlags const flags = ((ui_controller->m_selected_asset_model != asset_model.first)) ? ImGuiTreeNodeFlags_Leaf : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected);

                            bool const node_open = ImGui::TreeNodeEx(asset_model.first.c_str(), flags);

                            if (ImGui::IsItemFocused())
                            {
                                ui_controller->m_selected_asset_model = asset_model.first;
                            }

                            if (node_open)
                            {
                                ImGui::TreePop();
                            }
                        }
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            auto const &found_asset_model = ui_model->m_asset_models.find(ui_controller->m_selected_asset_model);

            if (ui_model->m_asset_models.end() != found_asset_model)
            {
                ImGui::SameLine();

                ImGui::BeginGroup();

                mcrt_string timestamp_text = "Timestamp: ";
                mcrt_string directory_name;
                mcrt_string file_name;
                {
                    size_t const timestamp_text_end_pos = found_asset_model->first.find(' ');
                    size_t const directory_name_end_pos = found_asset_model->first.find_last_of("/\\");
                    if ((mcrt_string::npos != timestamp_text_end_pos) && (mcrt_string::npos != directory_name_end_pos) && (timestamp_text_end_pos < directory_name_end_pos))
                    {
                        timestamp_text += found_asset_model->first.substr(0U, timestamp_text_end_pos);
                        directory_name = found_asset_model->first.substr(timestamp_text_end_pos + 1U, ((directory_name_end_pos - timestamp_text_end_pos) - 1U));
                        file_name = found_asset_model->first.substr(directory_name_end_pos + 1U);
                    }
                    else
                    {
                        assert(false);
                        timestamp_text = "N/A";
                        directory_name = "N/A";
                        file_name = "N/A";
                    }
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(file_name.c_str());
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(directory_name.c_str());
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(timestamp_text.c_str());
                    ImGui::PopStyleColor();
                }

                ImGui::Separator();

                if (ImGui::BeginTable("##Asset-Model-Manager-Right-Group-Table", 2, ImGuiTableFlags_BordersInnerV))
                {
                    ImGui::TableSetupColumn("##Asset-Model-Manager-Right-Group-Table-Property", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("##Asset-Model-Manager-Right-Group-Table-Value", ImGuiTableColumnFlags_WidthStretch, 2.0F);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    {
                        constexpr char const *const text[LANGUAGE_COUNT] = {
                            "Mesh Count",
                            "Mesh 数",
                            "網格數",
                            "网格数"};
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                        ImGui::TextUnformatted(text[ui_controller->m_language_index]);
                        ImGui::PopStyleColor();
                    }
                    ImGui::TableNextColumn();
                    {
                        assert(!found_asset_model->second.m_surface_groups.empty());
                        assert(!found_asset_model->second.m_skeletons.empty());
                        uint32_t const mesh_count = static_cast<uint32_t>(found_asset_model->second.m_surface_groups.size());
                        assert(mesh_count == found_asset_model->second.m_skeletons.size());

                        char mesh_count_text[] = {"18446744073709551615"};
                        std::snprintf(mesh_count_text, sizeof(mesh_count_text) / sizeof(mesh_count_text[0]), "%llu", static_cast<long long unsigned>(mesh_count));
                        mesh_count_text[(sizeof(mesh_count_text) / sizeof(mesh_count_text[0])) - 1] = '\0';

                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                        ImGui::TextUnformatted(mesh_count_text);
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndTable();
                }

                ImGui::EndGroup();
            }

            ImGui::TreePop();
        }
    }

    {
        ImGui::Separator();

        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Asset Image Manager",
                "資源画像管理",
                "資源图像管理",
                "资源圖像管理"};
            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
        }

        ImGui::SameLine();

        if (ImGui::TreeNodeEx("##Asset-Image-Manager", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoAutoOpenOnLog))
        {
            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Force SRGB",
                    "強制 SRGB",
                    "強制 SRGB",
                    "强制 SRGB"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                ImGui::Checkbox("##Asset-Image-Manager-Force-SRGB", &ui_controller->m_import_asset_image_force_srgb);
            }

            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Import",
                    "輸入",
                    "導入",
                    "导入"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                if (ImGui::Button("O##Asset-Image-Manager-Import"))
                {
                    bool asset_image_file_open;
                    mcrt_string asset_image_file_name;
                    uint64_t asset_image_file_timestamp;
                    mcrt_vector<uint8_t> asset_image_file_data;
                    {
                        constexpr size_t const asset_image_filter_count = 4;

                        constexpr char const *const asset_image_filter_names[asset_image_filter_count] = {
                            "All Files",
                            "Web Picture",
                            "Portable Network Graphics",
                            "Joint Photographic Expert Group"};

                        constexpr char const *const asset_image_filter_specs[asset_image_filter_count] = {
                            "*.*",
                            "*.webp",
                            "*.png",
                            "*.jpg;*.jpeg"};

                        asset_image_file_open = _internal_platform_get_open_file_name(platform_context, asset_image_filter_count, asset_image_filter_names, asset_image_filter_specs, ui_controller->m_import_asset_image_get_open_file_name_file_type_index, &asset_image_file_name, &asset_image_file_timestamp, &asset_image_file_data);
                    }

                    if (asset_image_file_open)
                    {
                        assert(!asset_image_file_name.empty());
                        assert(0U != asset_image_file_timestamp);
                        assert(!asset_image_file_data.empty());

                        brx_anari_image *const load_anari_image = _internal_load_asset_image_file(asset_image_file_name.c_str(), asset_image_file_timestamp, asset_image_file_data.data(), asset_image_file_data.size(), ui_controller->m_import_asset_image_force_srgb, device, ui_model);
                        assert(NULL != load_anari_image);
                    }
                }
            }

            ImGui::Separator();

            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Delete",
                    "削除",
                    "刪除",
                    "删除"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                ImGui::SameLine();

                if (ImGui::Button("X##Asset-Image-Manager-Delete"))
                {
                    auto const &found_asset_image = ui_model->m_asset_images.find(ui_controller->m_selected_asset_image);
                    if (ui_model->m_asset_images.end() != found_asset_image)
                    {
                        if (NULL != found_asset_image->second.m_image)
                        {
                            device->release_image(found_asset_image->second.m_image);
                            found_asset_image->second.m_image = NULL;
                        }
                        else
                        {
                            assert(false);
                        }

                        ui_model->m_asset_images.erase(found_asset_image);
                        ui_controller->m_selected_asset_image.clear();
                    }
                }
            }

            ImGui::Separator();

            if (ImGui::BeginChild("##Asset-Image-Manager-Left-Child", ImVec2(ui_width * 0.5F, 0.0F), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
            {
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                ImGuiTextFilter text_filter;
                {
                    constexpr char const *const hint[LANGUAGE_COUNT] = {
                        "Search",
                        "検索",
                        "檢索",
                        "检索"};
                    if (ImGui::InputTextWithHint("##Asset-Image-Manager-Left-Child-Text-Filter", hint[ui_controller->m_language_index], text_filter.InputBuf, IM_ARRAYSIZE(text_filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                    {
                        text_filter.Build();
                    }
                }
                ImGui::PopItemFlag();

                if (ImGui::BeginTable("##Asset-Image-Manager-Left-Child-Table", 1, ImGuiTableFlags_RowBg))
                {
                    for (auto const &asset_image : ui_model->m_asset_images)
                    {
                        if (text_filter.PassFilter(asset_image.first.c_str()))
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();

                            ImGuiTreeNodeFlags const flags = ((ui_controller->m_selected_asset_image != asset_image.first)) ? ImGuiTreeNodeFlags_Leaf : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected);

                            bool const node_open = ImGui::TreeNodeEx(asset_image.first.c_str(), flags);

                            if (ImGui::IsItemFocused())
                            {
                                ui_controller->m_selected_asset_image = asset_image.first;
                            }

                            if (node_open)
                            {
                                ImGui::TreePop();
                            }
                        }
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            auto const &found_asset_image = ui_model->m_asset_images.find(ui_controller->m_selected_asset_image);

            if (ui_model->m_asset_images.end() != found_asset_image)
            {
                ImGui::SameLine();

                ImGui::BeginGroup();

                mcrt_string timestamp_text = "Timestamp: ";
                mcrt_string directory_name;
                mcrt_string file_name;
                {
                    size_t const timestamp_text_end_pos = found_asset_image->first.find(' ');
                    size_t const directory_name_end_pos = found_asset_image->first.find_last_of("/\\");
                    if ((mcrt_string::npos != timestamp_text_end_pos) && (mcrt_string::npos != directory_name_end_pos) && (timestamp_text_end_pos < directory_name_end_pos))
                    {
                        timestamp_text += found_asset_image->first.substr(0U, timestamp_text_end_pos);
                        directory_name = found_asset_image->first.substr(timestamp_text_end_pos + 1U, ((directory_name_end_pos - timestamp_text_end_pos) - 1U));
                        file_name = found_asset_image->first.substr(directory_name_end_pos + 1U);
                    }
                    else
                    {
                        assert(false);
                        timestamp_text = "N/A";
                        directory_name = "N/A";
                        file_name = "N/A";
                    }
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(file_name.c_str());
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(directory_name.c_str());
                    ImGui::PopStyleColor();
                }

                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    ImGui::TextUnformatted(timestamp_text.c_str());
                    ImGui::PopStyleColor();
                }

                ImGui::Separator();

                if (ImGui::BeginTable("##Asset-Image-Manager-Right-Group-Table", 2, ImGuiTableFlags_BordersInnerV))
                {
                    ImGui::TableSetupColumn("##Asset-Image-Manager-Right-Group-Table-Property", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("##Asset-Image-Manager-Right-Group-Table-Value", ImGuiTableColumnFlags_WidthStretch, 2.0F);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    {
                        constexpr char const *const text[LANGUAGE_COUNT] = {
                            "Force SRGB",
                            "強制 SRGB",
                            "強制 SRGB",
                            "强制 SRGB"};
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                        ImGui::TextUnformatted(text[ui_controller->m_language_index]);
                        ImGui::PopStyleColor();
                    }
                    ImGui::TableNextColumn();
                    {
                        if (found_asset_image->second.m_force_srgb)
                        {
                            constexpr char const *const text[LANGUAGE_COUNT] = {
                                "Enable",
                                "有効",
                                "啟用",
                                "启用"};

                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            char const *text[LANGUAGE_COUNT] = {
                                "Disable",
                                "無効",
                                "停用",
                                "停用"};

                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
                            ImGui::PopStyleColor();
                        }
                    }

                    ImGui::EndTable();
                }

                ImGui::EndGroup();
            }

            ImGui::TreePop();
        }
    }

    {
        {
            ImGui::Separator();

            {
                constexpr char const *const text[LANGUAGE_COUNT] = {
                    "Instance Motion Manager",
                    "実例行動管理",
                    "實例動作管理",
                    "实例动作管理"};
                ImGui::TextUnformatted(text[ui_controller->m_language_index]);
            }

            ImGui::SameLine();

            if (ImGui::TreeNodeEx("##Instance-Motion-Manager", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoAutoOpenOnLog))
            {
                {
                    constexpr char const *const text[LANGUAGE_COUNT] = {
                        "Asset Motion",
                        "資源行動",
                        "資源動作",
                        "资源动作"};

                    constexpr char const *const label = "##Instance-Motion-Manager-Select-Asset-Motion";

                    if (!ui_model->m_asset_motions.empty())
                    {
                        ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                        ImGui::SameLine();

                        mcrt_vector<mcrt_string> item_strings(static_cast<size_t>(ui_model->m_asset_motions.size()));
                        mcrt_vector<char const *> items(static_cast<size_t>(ui_model->m_asset_motions.size()));
                        int selected_asset_motion_index = 0;
                        {
                            size_t asset_motion_index = 0U;
                            for (auto const &asset_motion : ui_model->m_asset_motions)
                            {
                                item_strings[asset_motion_index] = asset_motion.first;
                                items[asset_motion_index] = item_strings[asset_motion_index].c_str();

                                if (0 == std::strcmp(ui_controller->m_new_instance_motion_selected_asset_motion.data(), asset_motion.first.c_str()))
                                {
                                    assert(0 == selected_asset_motion_index);
                                    assert((1U + asset_motion_index) < static_cast<size_t>(INT_MAX));
                                    selected_asset_motion_index = static_cast<int>(asset_motion_index);
                                }

                                ++asset_motion_index;
                            }
                        }

                        ImGui::Combo(label, &selected_asset_motion_index, items.data(), items.size());

                        if (selected_asset_motion_index >= 0)
                        {
                            ui_controller->m_new_instance_motion_selected_asset_motion = std::move(item_strings[selected_asset_motion_index]);
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                    else
                    {

                        ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                        ImGui::SameLine();

                        char const *const items[LANGUAGE_COUNT][1] = {
                            {"Disable"},
                            {"無効"},
                            {"停用"},
                            {"停用"}};

                        int selected_asset_motion_index = 0;

                        ImGui::Combo(label, &selected_asset_motion_index, items[ui_controller->m_language_index], IM_ARRAYSIZE(items[ui_controller->m_language_index]));

                        assert(0 == selected_asset_motion_index);

                        ui_controller->m_new_instance_motion_selected_asset_motion.clear();
                    }
                }

                {
                    auto const &found_asset_motion = ui_model->m_asset_motions.find(ui_controller->m_new_instance_motion_selected_asset_motion);

                    constexpr char const *const text[LANGUAGE_COUNT] = {
                        "Animation Index",
                        "Animation 索引",
                        "動畫索引",
                        "动画索引"};

                    constexpr char const *const label = "##Instance-Motion-Manager-Select-Animation-Index";

                    if (ui_model->m_asset_motions.end() != found_asset_motion)
                    {
                        assert(!found_asset_motion->second.m_animations.empty());
                        uint32_t const animation_count = static_cast<uint32_t>(found_asset_motion->second.m_animations.size());

                        if (animation_count > 0U)
                        {

                            ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                            ImGui::SameLine();

                            mcrt_vector<mcrt_string> item_strings(static_cast<size_t>(animation_count));
                            mcrt_vector<char const *> items(static_cast<size_t>(animation_count));
                            int selected_animation_index = 0;
                            {
                                for (uint32_t animation_index = 0U; animation_index < animation_count; ++animation_index)
                                {
                                    char animation_index_text[] = {"18446744073709551615"};
                                    std::snprintf(animation_index_text, sizeof(animation_index_text) / sizeof(animation_index_text[0]), "%llu", static_cast<long long unsigned>(animation_index));
                                    animation_index_text[(sizeof(animation_index_text) / sizeof(animation_index_text[0])) - 1] = '\0';

                                    item_strings[animation_index] = animation_index_text;
                                    items[animation_index] = item_strings[animation_index].c_str();

                                    if (ui_controller->m_new_instance_motion_selected_animation_index == animation_index)
                                    {
                                        assert(0 == selected_animation_index);
                                        assert(static_cast<size_t>(1U + animation_index) < static_cast<size_t>(INT_MAX));
                                        selected_animation_index = static_cast<int>(animation_index);
                                    }
                                }
                            }

                            ImGui::Combo(label, &selected_animation_index, items.data(), items.size());

                            if (selected_animation_index >= 0)
                            {
                                ui_controller->m_new_instance_motion_selected_animation_index = selected_animation_index;
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                        else
                        {
                            assert(false);

                            ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                            ImGui::SameLine();

                            char const *const items[LANGUAGE_COUNT][1] = {
                                {"Disable"},
                                {"無効"},
                                {"停用"},
                                {"停用"}};

                            int selected_asset_motion_index = 0;

                            ImGui::Combo(label, &selected_asset_motion_index, items[ui_controller->m_language_index], IM_ARRAYSIZE(items[ui_controller->m_language_index]));

                            assert(0 == selected_asset_motion_index);

                            ui_controller->m_new_instance_motion_selected_animation_index = -1;
                        }
                    }
                    else
                    {
                        ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                        ImGui::SameLine();

                        char const *const items[LANGUAGE_COUNT][1] = {
                            {"Disable"},
                            {"無効"},
                            {"停用"},
                            {"停用"}};

                        int selected_asset_motion_index = 0;

                        ImGui::Combo(label, &selected_asset_motion_index, items[ui_controller->m_language_index], IM_ARRAYSIZE(items[ui_controller->m_language_index]));

                        assert(0 == selected_asset_motion_index);

                        ui_controller->m_new_instance_motion_selected_animation_index = -1;
                    }
                }

                {
                    constexpr char const *const hint[LANGUAGE_COUNT] = {
                        "New Instance Motion Name",
                        "新規実例資源名前",
                        "新建實例資源名稱",
                        "新建实例资源名称"};
                    ImGui::InputTextWithHint("##Instance-Motion-Manager-New-Instance-Name", hint[ui_controller->m_language_index], ui_controller->m_new_instance_motion_name.data(), ui_controller->m_new_instance_motion_name.size());
                }

                {
                    constexpr char const *const text[LANGUAGE_COUNT] = {
                        "New",
                        "新規作成",
                        "新建",
                        "新建"};

                    ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                    ImGui::SameLine();

                    if (ImGui::Button("N##Instance-Motion-Manager-New"))
                    {
                        auto const &found_asset_motion = ui_model->m_asset_motions.find(ui_controller->m_new_instance_motion_selected_asset_motion);

                        if ((ui_model->m_asset_motions.end() != found_asset_motion) && (ui_controller->m_new_instance_motion_selected_animation_index < found_asset_motion->second.m_animations.size()))
                        {
                            brx_motion_animation_instance *const motion_animation_instance = brx_motion_create_animation_instance(found_asset_motion->second.m_animations[ui_controller->m_new_instance_motion_selected_animation_index]);
                            if (NULL != motion_animation_instance)
                            {
                                uint64_t const timestamp = _internal_tick_count_now();

                                // should always NOT alreay exist in practice
                                std::pair<uint64_t, ui_instance_motion_model_t> instance_motion_model(timestamp, ui_instance_motion_model_t{ui_controller->m_new_instance_motion_name.data(), ui_controller->m_new_instance_motion_selected_asset_motion, static_cast<uint32_t>(ui_controller->m_new_instance_motion_selected_animation_index), motion_animation_instance});
                                ui_model->m_instance_motions.insert(ui_model->m_instance_motions.end(), instance_motion_model);
                            }
                            else
                            {
                                assert(false);
                            }
                        }
                    }
                }

                ImGui::Separator();

                {
                    constexpr char const *const text[LANGUAGE_COUNT] = {
                        "Delete",
                        "削除",
                        "刪除",
                        "删除"};
                    ImGui::TextUnformatted(text[ui_controller->m_language_index]);

                    ImGui::SameLine();

                    if (ImGui::Button("X##Instance-Motion-Manager-Delete"))
                    {
                        auto const &found_instance_motion = ui_model->m_instance_motions.find(ui_controller->m_selected_instance_motion);
                        if (ui_model->m_instance_motions.end() != found_instance_motion)
                        {
                            if (NULL != found_instance_motion->second.m_animation_instance)
                            {
                                brx_motion_destroy_animation_instance(found_instance_motion->second.m_animation_instance);
                                found_instance_motion->second.m_animation_instance = NULL;
                            }
                            else
                            {
                                assert(false);
                            }

                            ui_model->m_instance_motions.erase(found_instance_motion);
                            ui_controller->m_selected_instance_motion = -1;
                        }
                    }
                }

                ImGui::Separator();

                if (ImGui::BeginChild("##Instance-Motion-Manager-Left-Child", ImVec2(ui_width * 0.5F, 0.0F), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
                {
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                    ImGuiTextFilter text_filter;
                    {
                        constexpr char const *const hint[LANGUAGE_COUNT] = {
                            "Search",
                            "検索",
                            "檢索",
                            "检索"};
                        if (ImGui::InputTextWithHint("##Instance-Motion-Manager-Left-Child-Text-Filter", hint[ui_controller->m_language_index], text_filter.InputBuf, IM_ARRAYSIZE(text_filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                        {
                            text_filter.Build();
                        }
                    }

                    ImGui::PopItemFlag();

                    if (ImGui::BeginTable("##Instance-Motion-Manager-Left-Child-Table", 1, ImGuiTableFlags_RowBg))
                    {
                        for (auto const &instance_motion : ui_model->m_instance_motions)
                        {
                            mcrt_string instance_motion_identity;
                            {
                                char instance_motion_timestamp_text[] = {"18446744073709551615"};
                                std::snprintf(instance_motion_timestamp_text, sizeof(instance_motion_timestamp_text) / sizeof(instance_motion_timestamp_text[0]), "%llu", static_cast<long long unsigned>(instance_motion.first));
                                instance_motion_timestamp_text[(sizeof(instance_motion_timestamp_text) / sizeof(instance_motion_timestamp_text[0])) - 1] = '\0';

                                instance_motion_identity += instance_motion_timestamp_text;
                                instance_motion_identity += ' ';
                                instance_motion_identity += instance_motion.second.m_name;
                            }

                            if (text_filter.PassFilter(instance_motion_identity.c_str()))
                            {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();

                                ImGuiTreeNodeFlags const flags = ((ui_controller->m_selected_instance_motion != instance_motion.first)) ? ImGuiTreeNodeFlags_Leaf : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected);

                                bool const node_open = ImGui::TreeNodeEx(instance_motion_identity.c_str(), flags);

                                if (ImGui::IsItemFocused())
                                {
                                    ui_controller->m_selected_instance_motion = instance_motion.first;
                                }

                                if (node_open)
                                {
                                    ImGui::TreePop();
                                }
                            }
                        }

                        ImGui::EndTable();
                    }
                }
                ImGui::EndChild();

                auto const &found_instance_motion = ui_model->m_instance_motions.find(ui_controller->m_selected_instance_motion);

                if (ui_model->m_instance_motions.end() != found_instance_motion)
                {
                    ImGui::SameLine();

                    ImGui::BeginGroup();

                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                        ImGui::TextUnformatted(found_instance_motion->second.m_name.c_str());
                        ImGui::PopStyleColor();
                    }

                    ImGui::Separator();
                    {
                        mcrt_string timestamp_text = "Timestamp: ";
                        mcrt_string directory_name;
                        mcrt_string file_name;
                        {
                            size_t const timestamp_text_end_pos = found_instance_motion->second.m_asset_motion.find(' ');
                            size_t const directory_name_end_pos = found_instance_motion->second.m_asset_motion.find_last_of("/\\");
                            if ((mcrt_string::npos != timestamp_text_end_pos) && (mcrt_string::npos != directory_name_end_pos) && (timestamp_text_end_pos < directory_name_end_pos))
                            {
                                timestamp_text += found_instance_motion->second.m_asset_motion.substr(0U, timestamp_text_end_pos);
                                directory_name = found_instance_motion->second.m_asset_motion.substr(timestamp_text_end_pos + 1U, ((directory_name_end_pos - timestamp_text_end_pos) - 1U));
                                file_name = found_instance_motion->second.m_asset_motion.substr(directory_name_end_pos + 1U);
                            }
                            else
                            {
                                assert(false);
                                timestamp_text = "N/A";
                                directory_name = "N/A";
                                file_name = "N/A";
                            }
                        }

                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(file_name.c_str());
                            ImGui::PopStyleColor();
                        }

                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(directory_name.c_str());
                            ImGui::PopStyleColor();
                        }

                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(timestamp_text.c_str());
                            ImGui::PopStyleColor();
                        }
                    }

                    ImGui::Separator();
                    {
                        mcrt_string animation_index_text;
                        {
                            constexpr char const *const text1[LANGUAGE_COUNT] = {
                                "Animation Index ",
                                "Animation 索引 ",
                                "動畫索引 ",
                                "动画索引 "};

                            animation_index_text += text1[ui_controller->m_language_index];

                            char text2[] = {"18446744073709551615"};
                            std::snprintf(text2, sizeof(text2) / sizeof(text2[0]), "%llu", static_cast<long long unsigned>(found_instance_motion->second.m_animation_index));
                            text2[(sizeof(text2) / sizeof(text2[0])) - 1] = '\0';

                            animation_index_text += text2;
                        }

                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(animation_index_text.c_str());
                            ImGui::PopStyleColor();
                        }
                    }

                    ImGui::Separator();

#if 0
                    if (ImGui::BeginTable("##Instance-Motion-Manager-Right-Group-Table", 2, ImGuiTableFlags_BordersInnerV))
                    {
                        ImGui::TableSetupColumn("##Instance-Motion-Manager-Right-Group-Table-Property", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("##Instance-Motion-Manager-Right-Group-Table-Value", ImGuiTableColumnFlags_WidthStretch, 2.0F);

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        {
                            constexpr char const *const text[LANGUAGE_COUNT] = {
                                "Mesh Count",
                                "Mesh 数",
                                "網格數",
                                "网格数"};
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(text[ui_controller->m_language_index]);
                            ImGui::PopStyleColor();
                        }
                        ImGui::TableNextColumn();
                        {
                            assert(!found_asset_model->second.m_surface_groups.empty());
                            assert(!found_asset_model->second.m_skeletons.empty());
                            uint32_t const mesh_count = static_cast<uint32_t>(found_asset_model->second.m_surface_groups.size());
                            assert(mesh_count == found_asset_model->second.m_skeletons.size());

                            char mesh_count_text[] = {"18446744073709551615"};
                            std::snprintf(mesh_count_text, sizeof(mesh_count_text) / sizeof(mesh_count_text[0]), "%llu", static_cast<long long unsigned>(mesh_count));
                            mesh_count_text[(sizeof(mesh_count_text) / sizeof(mesh_count_text[0])) - 1] = '\0';

                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                            ImGui::TextUnformatted(mesh_count_text);
                            ImGui::PopStyleColor();
                        }

                        ImGui::EndTable();
                    }
#endif
                    ImGui::EndGroup();
                }

                ImGui::TreePop();
            }
        }
    }

    {
        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Import Model Asset",
                "輸入型式資源",
                "導入模型資源",
                "导入模型资源"};

            ImGui::TextUnformatted(text[ui_controller->m_language_index]);

            ImGui::SameLine();

            if (ImGui::Button("O##Import-Model-Asset"))
            {
                bool model_file;
                mcrt_string model_file_name;
                uint64_t model_file_timestamp;
                mcrt_vector<uint8_t> model_file_data;
                {
                    constexpr size_t const model_filter_count = 2;

                    // TODO
                    // "glTF Binary"    "*.glb;*.vrm"
                    // "glTF Separate"  "*.gltf"

                    constexpr char const *const model_filter_names[model_filter_count] = {
                        "All Files",
                        "MMD Model Data"};

                    constexpr char const *const model_filter_specs[model_filter_count] = {
                        "*.*",
                        "*.pmx"};

                    model_file = _internal_platform_get_open_file_name(platform_context, model_filter_count, model_filter_names, model_filter_specs, ui_controller->m_import_asset_model_get_open_file_name_file_type_index, &model_file_name, &model_file_timestamp, &model_file_data);
                }

                if (model_file)
                {
                    assert(!model_file_name.empty());
                    assert(0U != model_file_timestamp);

                    if ((ui_model->m_asset_model.m_file_name != model_file_name) || (ui_model->m_asset_model.m_file_timestamp != model_file_timestamp))
                    {
                        brx_asset_import_scene *const asset_import_scene = brx_asset_import_create_scene_from_memory(model_file_data.data(), model_file_data.size());
                        if (NULL != asset_import_scene)
                        {
                            for (brx_motion_skeleton_instance *const skeleton_instance : ui_model->m_asset_model.m_skeleton_instances)
                            {
                                assert(NULL != skeleton_instance);
                                brx_motion_destroy_skeleton_instance(skeleton_instance);
                            }
                            ui_model->m_asset_model.m_skeleton_instances.clear();

                            for (brx_motion_skeleton *const skeleton : ui_model->m_asset_model.m_skeletons)
                            {
                                assert(NULL != skeleton);
                                brx_motion_destroy_skeleton(skeleton);
                            }
                            ui_model->m_asset_model.m_skeletons.clear();

                            for (brx_anari_surface_group_instance *const surface_group_instance : ui_model->m_asset_model.m_surface_group_instances)
                            {
                                assert(NULL != surface_group_instance);
                                device->world_release_surface_group_instance(surface_group_instance);
                            }
                            ui_model->m_asset_model.m_surface_group_instances.clear();

                            for (brx_anari_surface_group *const surface_group : ui_model->m_asset_model.m_surface_groups)
                            {
                                assert(NULL != surface_group);
                                device->release_surface_group(surface_group);
                            }
                            ui_model->m_asset_model.m_surface_groups.clear();

                            mcrt_string asset_model_directory_name;
                            {
                                size_t directory_name_end_pos = model_file_name.find_last_of("/\\");
                                if (mcrt_string::npos != directory_name_end_pos)
                                {
                                    asset_model_directory_name = model_file_name.substr(0U, directory_name_end_pos + 1U);
                                }
                                else
                                {
                                    asset_model_directory_name = "./";
                                }
                            }

                            uint32_t const surface_group_count = asset_import_scene->get_surface_group_count();

                            mcrt_vector<brx_anari_surface_group *> surface_groups(static_cast<size_t>(surface_group_count));
                            mcrt_vector<brx_anari_surface_group_instance *> surface_group_instances(static_cast<size_t>(surface_group_count));
                            mcrt_vector<brx_motion_skeleton *> skeletons(static_cast<size_t>(surface_group_count));
                            mcrt_vector<brx_motion_skeleton_instance *> skeleton_instances(static_cast<size_t>(surface_group_count));
                            for (uint32_t surface_group_index = 0U; surface_group_index < surface_group_count; ++surface_group_index)
                            {
                                brx_asset_import_surface_group const *const asset_import_surface_group = asset_import_scene->get_surface_group(surface_group_index);

                                uint32_t const surface_count = asset_import_surface_group->get_surface_count();

                                mcrt_vector<BRX_ANARI_SURFACE> surfaces(static_cast<size_t>(surface_count));

                                for (uint32_t surface_index = 0U; surface_index < surface_count; ++surface_index)
                                {
                                    brx_asset_import_surface const *const asset_import_surface = asset_import_surface_group->get_surface(surface_index);

                                    surfaces[surface_index].m_vertex_count = asset_import_surface->get_vertex_count();
                                    surfaces[surface_index].m_vertex_positions = wrap(asset_import_surface->get_vertex_positions());
                                    surfaces[surface_index].m_vertex_varyings = wrap(asset_import_surface->get_vertex_varyings());
                                    surfaces[surface_index].m_vertex_blendings = wrap(asset_import_surface->get_vertex_blendings());
                                    for (uint32_t morph_target_name_index = 0U; morph_target_name_index < BRX_ANARI_MORPH_TARGET_NAME_MMD_COUNT; ++morph_target_name_index)
                                    {
                                        surfaces[surface_index].m_morph_targets_vertex_positions[morph_target_name_index] = NULL;
                                        surfaces[surface_index].m_morph_targets_vertex_varyings[morph_target_name_index] = NULL;
                                    }
                                    for (uint32_t morph_target_index = 0U; morph_target_index < asset_import_surface->get_morph_target_count(); ++morph_target_index)
                                    {
                                        BRX_ANARI_MORPH_TARGET_NAME const morph_target_name = wrap(asset_import_surface->get_morph_target_name(morph_target_index));
                                        uint32_t const morph_target_name_index = morph_target_name;
                                        surfaces[surface_index].m_morph_targets_vertex_positions[morph_target_name_index] = wrap(asset_import_surface->get_morph_target_vertex_positions(morph_target_index));
                                        surfaces[surface_index].m_morph_targets_vertex_varyings[morph_target_name_index] = wrap(asset_import_surface->get_morph_target_vertex_varyings(morph_target_index));
                                    }
                                    surfaces[surface_index].m_index_count = asset_import_surface->get_index_count();
                                    surfaces[surface_index].m_indices = asset_import_surface->get_indices();
                                    surfaces[surface_index].m_emissive_image = _internal_load_asset_image(asset_import_surface->get_emissive_image_url(), asset_model_directory_name.c_str(), false, device, ui_model);
                                    surfaces[surface_index].m_emissive_factor = wrap(asset_import_surface->get_emissive_factor());
                                    surfaces[surface_index].m_normal_image = _internal_load_asset_image(asset_import_surface->get_normal_image_url(), asset_model_directory_name.c_str(), false, device, ui_model);
                                    surfaces[surface_index].m_normal_scale = asset_import_surface->get_normal_scale();
                                    surfaces[surface_index].m_base_color_image = _internal_load_asset_image(asset_import_surface->get_base_color_image_url(), asset_model_directory_name.c_str(), true, device, ui_model);
                                    surfaces[surface_index].m_base_color_factor = wrap(asset_import_surface->get_base_color_factor());
                                    surfaces[surface_index].m_metallic_roughness_image = _internal_load_asset_image(asset_import_surface->get_metallic_roughness_image_url(), asset_model_directory_name.c_str(), false, device, ui_model);
                                    surfaces[surface_index].m_metallic_factor = asset_import_surface->get_metallic_factor();
                                    surfaces[surface_index].m_roughness_factor = asset_import_surface->get_roughness_factor();
                                }

                                brx_anari_surface_group *const anari_surface_group = device->new_surface_group(surface_count, surfaces.data());

                                surface_groups[surface_group_index] = anari_surface_group;

                                brx_anari_surface_group_instance *const anari_surface_group_instance = device->world_new_surface_group_instance(anari_surface_group);

                                uint32_t const animation_skeleton_joint_count = asset_import_surface_group->get_animation_skeleton_joint_count();

                                {
                                    for (uint32_t morph_target_name_index = 0U; morph_target_name_index < BRX_ANARI_MORPH_TARGET_NAME_MMD_COUNT; ++morph_target_name_index)
                                    {
                                        BRX_ANARI_MORPH_TARGET_NAME const morph_target_name = static_cast<BRX_ANARI_MORPH_TARGET_NAME>(morph_target_name_index);

                                        anari_surface_group_instance->set_morph_target_weight(morph_target_name, 0.0F);
                                    }
                                }

                                {
                                    mcrt_vector<brx_anari_rigid_transform> const skin_transforms(static_cast<size_t>(animation_skeleton_joint_count), brx_anari_rigid_transform{{0.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 0.0F}});

                                    anari_surface_group_instance->set_skin_transforms(animation_skeleton_joint_count, skin_transforms.data());
                                }

                                anari_surface_group_instance->set_model_transform(brx_anari_rigid_transform{{0.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 0.0F}});

                                surface_group_instances[surface_group_index] = anari_surface_group_instance;

                                brx_motion_skeleton *const motion_skeleton = brx_motion_create_skeleton(animation_skeleton_joint_count, wrap(asset_import_surface_group->get_animation_skeleton_joint_names()), asset_import_surface_group->get_animation_skeleton_joint_parent_indices(), wrap(asset_import_surface_group->get_animation_skeleton_joint_transforms_bind_pose_local_space()), asset_import_surface_group->get_animation_skeleton_joint_constraint_count(), wrap(asset_import_surface_group->get_animation_skeleton_joint_constraint_names()), wrap(asset_import_surface_group->get_animation_skeleton_joint_constraints()), asset_import_surface_group->get_ragdoll_skeleton_rigid_body_count(), wrap(asset_import_surface_group->get_ragdoll_skeleton_rigid_bodies()), asset_import_surface_group->get_ragdoll_skeleton_constraint_count(), wrap(asset_import_surface_group->get_ragdoll_skeleton_constraints()), asset_import_surface_group->get_animation_to_ragdoll_direct_mapping_count(), wrap(asset_import_surface_group->get_animation_to_ragdoll_direct_mappings()), asset_import_surface_group->get_ragdoll_to_animation_direct_mapping_count(), wrap(asset_import_surface_group->get_ragdoll_to_animation_direct_mappings()));

                                skeletons[surface_group_index] = motion_skeleton;

                                brx_motion_skeleton_instance *const motion_skeleton_instace = brx_motion_create_skeleton_instance(motion_skeleton);

                                if (ui_model->m_asset_motion.m_animation_instances.size() > surface_group_index)
                                {
                                    motion_skeleton_instace->set_input(ui_model->m_asset_motion.m_animation_instances[surface_group_index]);
                                }

                                skeleton_instances[surface_group_index] = motion_skeleton_instace;
                            }

                            ui_model->m_asset_model.m_file_name = std::move(model_file_name);
                            ui_model->m_asset_model.m_file_timestamp = model_file_timestamp;
                            assert(ui_model->m_asset_model.m_surface_groups.empty());
                            ui_model->m_asset_model.m_surface_groups = std::move(surface_groups);
                            assert(ui_model->m_asset_model.m_surface_group_instances.empty());
                            ui_model->m_asset_model.m_surface_group_instances = std::move(surface_group_instances);
                            assert(ui_model->m_asset_model.m_skeletons.empty());
                            ui_model->m_asset_model.m_skeletons = std::move(skeletons);
                            assert(ui_model->m_asset_model.m_skeleton_instances.empty());
                            ui_model->m_asset_model.m_skeleton_instances = std::move(skeleton_instances);

                            brx_asset_import_destroy_scene(asset_import_scene);
                        }
                        else
                        {
                            assert(false);
                            ui_model->m_asset_model.m_file_name.clear();
                            ui_model->m_asset_model.m_file_timestamp = 0U;
                        }
                    }
                    else
                    {
                        // Do Nothing
                    }
                }
            }
        }

        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Import Motion Asset",
                "輸入行動資源",
                "導入動作資源",
                "导入动作资源"};

            ImGui::TextUnformatted(text[ui_controller->m_language_index]);

            ImGui::SameLine();

            if (ImGui::Button("O##Import-Motion-Asset"))
            {
                bool motion_file;
                mcrt_string motion_file_name;
                uint64_t motion_file_timestamp;
                mcrt_vector<uint8_t> motion_file_data;
                {
                    constexpr size_t const motion_filter_count = 2;

                    // TODO
                    // "glTF Binary"    "*.glb;*.vrma"
                    // "glTF Separate"  "*.gltf"

                    constexpr char const *const motion_filter_names[motion_filter_count] = {
                        "All Files",
                        "MMD Motion Data"};

                    constexpr char const *const motion_filter_specs[motion_filter_count] = {
                        "*.*",
                        "*.vmd"};

                    motion_file = _internal_platform_get_open_file_name(platform_context, motion_filter_count, motion_filter_names, motion_filter_specs, ui_controller->m_import_asset_motion_get_open_file_name_file_type_index, &motion_file_name, &motion_file_timestamp, &motion_file_data);
                }

                if (motion_file)
                {
                    assert(!motion_file_name.empty());
                    assert(0U != motion_file_timestamp);

                    if ((ui_model->m_asset_motion.m_file_name != motion_file_name) || (ui_model->m_asset_motion.m_file_timestamp != motion_file_timestamp))
                    {
                        brx_asset_import_scene *const asset_import_scene = brx_asset_import_create_scene_from_memory(motion_file_data.data(), motion_file_data.size());
                        if (NULL != asset_import_scene)
                        {
                            for (brx_motion_skeleton_instance *const skeleton_instance : ui_model->m_asset_model.m_skeleton_instances)
                            {
                                assert(NULL != skeleton_instance);
                                skeleton_instance->set_input(NULL);
                            }

                            for (brx_motion_animation_instance *const animation_instance : ui_model->m_asset_motion.m_animation_instances)
                            {
                                assert(NULL != animation_instance);
                                brx_motion_destroy_animation_instance(animation_instance);
                            }
                            ui_model->m_asset_motion.m_animation_instances.clear();

                            for (brx_motion_animation *const animation : ui_model->m_asset_motion.m_animations)
                            {
                                assert(NULL != animation);
                                brx_motion_destroy_animation(animation);
                            }
                            ui_model->m_asset_motion.m_animations.clear();

                            uint32_t const animation_count = asset_import_scene->get_animation_count();

                            mcrt_vector<brx_motion_animation *> animations(static_cast<size_t>(animation_count));
                            mcrt_vector<brx_motion_animation_instance *> animation_instances(static_cast<size_t>(animation_count));
                            for (uint32_t animation_index = 0U; animation_index < animation_count; ++animation_index)
                            {
                                brx_asset_import_animation const *const asset_import_animation = asset_import_scene->get_animation(animation_index);

                                brx_motion_animation *const motion_animation = brx_motion_create_animation(asset_import_animation->get_frame_count(), asset_import_animation->get_weight_channel_count(), wrap(asset_import_animation->get_weight_channel_names()), asset_import_animation->get_weights(), asset_import_animation->get_rigid_transform_channel_count(), wrap(asset_import_animation->get_rigid_transform_channel_names()), wrap(asset_import_animation->get_rigid_transforms()), asset_import_animation->get_switch_channel_count(), wrap(asset_import_animation->get_switch_channel_names()), asset_import_animation->get_switches());

                                animations[animation_index] = motion_animation;

                                brx_motion_animation_instance *const motion_animation_instance = brx_motion_create_animation_instance(motion_animation);

                                if (ui_model->m_asset_model.m_skeleton_instances.size() > animation_index)
                                {
                                    ui_model->m_asset_model.m_skeleton_instances[animation_index]->set_input(motion_animation_instance);
                                }

                                animation_instances[animation_index] = motion_animation_instance;
                            }

                            ui_model->m_asset_motion.m_file_name = std::move(motion_file_name);
                            ui_model->m_asset_motion.m_file_timestamp = motion_file_timestamp;
                            assert(ui_model->m_asset_motion.m_animations.empty());
                            ui_model->m_asset_motion.m_animations = std::move(animations);
                            assert(ui_model->m_asset_motion.m_animation_instances.empty());
                            ui_model->m_asset_motion.m_animation_instances = std::move(animation_instances);

                            brx_asset_import_destroy_scene(asset_import_scene);
                        }
                        else
                        {
                            assert(false);
                            ui_model->m_asset_motion.m_file_name.clear();
                            ui_model->m_asset_motion.m_file_timestamp = 0U;
                        }
                    }
                    else
                    {
                        // Do Nothing
                    }
                }
            }
        }

        {
            constexpr char const *const text[LANGUAGE_COUNT] = {
                "Clear Value",
                "Clear 値",
                "Clear 值",
                "Clear 值"};

            ImGui::TextUnformatted(text[ui_controller->m_language_index]);

            ImGui::SameLine();

            ImGui::ColorEdit3("##Clear Value", &ui_model->m_scene_color_clear_value[0]);
        }
    }

    ImGui::End();
}

extern void user_camera_controller_init(brx_anari_device *device, user_camera_model_t const *user_camera_model, user_camera_controller_t *user_camera_controller)
{
    brx_anari_vec3 const camera_position = device->camera_get_position();

    brx_anari_vec3 const camera_direction = device->camera_get_direction();

    brx_anari_vec3 const camera_up = device->camera_get_up();

    DirectX::XMFLOAT3 const eye_position(camera_position.m_x, camera_position.m_y, camera_position.m_z);

    DirectX::XMFLOAT3 const eye_direction(camera_direction.m_x, camera_direction.m_y, camera_direction.m_z);

    DirectX::XMFLOAT3 const up_direction(camera_up.m_x, camera_up.m_y, camera_up.m_z);

    user_camera_controller->m_first_person_camera.SetEyePt(eye_position);

    user_camera_controller->m_first_person_camera.SetEyeDir(eye_direction);

    user_camera_controller->m_first_person_camera.SetUpDir(up_direction);
}

extern void user_camera_simulate(float interval_time, brx_anari_device *device, user_camera_model_t *user_camera_model, user_camera_controller_t *user_camera_controller)
{
    user_camera_controller->m_first_person_camera.FrameMove(interval_time);

    DirectX::XMFLOAT3 eye_position;
    DirectX::XMStoreFloat3(&eye_position, user_camera_controller->m_first_person_camera.GetEyePt());

    DirectX::XMFLOAT3 eye_direction;
    DirectX::XMStoreFloat3(&eye_direction, user_camera_controller->m_first_person_camera.GetEyeDir());

    DirectX::XMFLOAT3 up_direction;
    DirectX::XMStoreFloat3(&up_direction, user_camera_controller->m_first_person_camera.GetUpDir());

    brx_anari_vec3 const camera_position{eye_position.x, eye_position.y, eye_position.z};

    brx_anari_vec3 const camera_direction{eye_direction.x, eye_direction.y, eye_direction.z};

    brx_anari_vec3 const camera_up{up_direction.x, up_direction.y, up_direction.z};

    device->camera_set_position(camera_position);
    device->camera_set_direction(camera_direction);
    device->camera_set_up(camera_up);
}

static inline brx_anari_image *_internal_load_asset_image(uint8_t const *asset_image_url, char const *asset_model_directory_name, bool force_srgb, brx_anari_device *device, ui_model_t *ui_model)
{
    brx_anari_image *load_anari_image;
    if (NULL != asset_image_url)
    {
        if (('f' == asset_image_url[0]) && ('i' == asset_image_url[1]) && ('l' == asset_image_url[2]) && ('e' == asset_image_url[3]) && (':' == asset_image_url[4]) && ('/' == asset_image_url[5]) && ('/' == asset_image_url[6]))
        {
            mcrt_string asset_image_file_name;
            {
                asset_image_file_name += asset_model_directory_name;
                asset_image_file_name += '/';
                asset_image_file_name += reinterpret_cast<char const *>(asset_image_url + 7);
            }

            bool asset_image_file;
            uint64_t asset_image_file_timestamp;
            mcrt_vector<uint8_t> asset_image_file_data;
            {
                asset_image_file = _internal_platform_get_file_timestamp_and_data(asset_image_file_name.c_str(), &asset_image_file_timestamp, &asset_image_file_data);
            }

            if (asset_image_file)
            {
                assert(0U != asset_image_file_timestamp);
                assert(!asset_image_file_data.empty());
                load_anari_image = _internal_load_asset_image_file(asset_image_file_name.c_str(), asset_image_file_timestamp, asset_image_file_data.data(), asset_image_file_data.size(), force_srgb, device, ui_model);
            }
            else
            {
                assert(false);
                load_anari_image = NULL;
            }
        }
        else
        {
            assert(false);
            load_anari_image = NULL;
        }
    }
    else
    {
        load_anari_image = NULL;
    }

    return load_anari_image;
}

static inline brx_anari_image *_internal_load_asset_image_file(char const *asset_image_file_name, uint64_t asset_image_file_timestamp, void const *asset_image_file_data_base, size_t asset_image_file_data_size, bool force_srgb, brx_anari_device *device, ui_model_t *ui_model)
{
    brx_anari_image *load_anari_image;
    {
        mcrt_string asset_image_file_identity;
        {
            char asset_image_file_timestamp_text[] = {"18446744073709551615"};
            std::snprintf(asset_image_file_timestamp_text, sizeof(asset_image_file_timestamp_text) / sizeof(asset_image_file_timestamp_text[0]), "%llu", static_cast<long long unsigned>(asset_image_file_timestamp));
            asset_image_file_timestamp_text[(sizeof(asset_image_file_timestamp_text) / sizeof(asset_image_file_timestamp_text[0])) - 1] = '\0';

            asset_image_file_identity += asset_image_file_timestamp_text;
            asset_image_file_identity += ' ';
            asset_image_file_identity += asset_image_file_name;
        }

        auto const &found_asset_image = ui_model->m_asset_images.find(asset_image_file_identity);

        if (ui_model->m_asset_images.end() == found_asset_image)
        {
            brx_asset_import_image *asset_import_image = brx_asset_import_create_image_from_memory(asset_image_file_data_base, asset_image_file_data_size);

            if (NULL != asset_import_image)
            {
                if (BRX_ASSET_IMPORT_IMAGE_FORMAT_R8G8B8A8 == asset_import_image->get_format())
                {
                    brx_anari_image *anari_image = device->new_image(force_srgb ? BRX_ANARI_IMAGE_FORMAT_R8G8B8A8_SRGB : BRX_ANARI_IMAGE_FORMAT_R8G8B8A8_UNORM, asset_import_image->get_pixel_data(), asset_import_image->get_width(), asset_import_image->get_height());

                    if (NULL != anari_image)
                    {
                        std::pair<mcrt_string, ui_asset_image_model_t> image_model(asset_image_file_identity, ui_asset_image_model_t{force_srgb, anari_image});
                        ui_model->m_asset_images.insert(found_asset_image, std::move(image_model));

                        load_anari_image = anari_image;
                    }
                    else
                    {
                        assert(false);
                        load_anari_image = NULL;
                    }
                }
                else
                {
                    assert(false);
                    load_anari_image = NULL;
                }

                brx_asset_import_destroy_image(asset_import_image);
            }
            else
            {
                assert(false);
                load_anari_image = NULL;
            }
        }
        else
        {
            assert(NULL != found_asset_image->second.m_image);
            load_anari_image = found_asset_image->second.m_image;
        }
    }

    return load_anari_image;
}