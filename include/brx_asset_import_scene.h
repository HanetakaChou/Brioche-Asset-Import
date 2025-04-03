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

#ifndef _BRX_ASSET_IMPORT_SCENE_H_
#define _BRX_ASSET_IMPORT_SCENE_H_ 1

#include "brx_asset_import_input_stream.h"
#include <cstddef>
#include <cstdint>

// [Khronos ANARI (Analytic Rendering Interface): glTF To ANARI](https://github.com/KhronosGroup/ANARI-SDK/blob/next_release/src/anari_test_scenes/scenes/file/gltf2anari.h)

class brx_asset_import_scene;
class brx_asset_import_surface_group;
class brx_asset_import_surface;
class brx_asset_import_animation;

static constexpr uint32_t const BRX_ASSET_IMPORT_UINT32_INDEX_INVALID = static_cast<uint32_t>(~static_cast<uint32_t>(0U));

// [PMX TDA Miku Append](https://www.deviantart.com/xoriu/art/MMD-Facial-Expressions-Chart-341504917)
// [PMX Mirai Akari](https://www.deviantart.com/inochi-pm/art/MMD-Facial-Expressions-Chart-V2-802048879)
// [ARKit](https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation)
// [FACS AU](https://www.cs.cmu.edu/~face/facs.htm)

enum BRX_ASSET_IMPORT_MORPH_TARGET_NAME : uint32_t
{
    // にこり
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_HAPPY = 0,
    // 怒り
    // 真面目
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY = 1,
    // 困る
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SAD = 2,
    // 上
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SURPRISED = 3,
    // まばたき
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK = 4,
    // ウィンク２
    // ウィンク
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L = 5,
    // ｳｨﾝｸ２右
    // ウィンク右
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R = 6,
    // 笑い
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_HAPPY = 7,
    // ｷﾘｯ
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_ANGRY = 8,
    // じと目
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SAD = 9,
    // びっくり
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SURPRISED = 10,
    // あ
    // あ２
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A = 11,
    // い
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_I = 12,
    // う
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_U = 13,
    // え
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_E = 14,
    // お
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_O = 15,
    // にっこり
    // にやり
    // にやり２
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY = 16,
    // ∧
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY = 17,
    // 口角下げ
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SAD = 18,
    // ▲
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED = 19,
    //
    BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT = 20
};

enum BRX_ASSET_IMPORT_SKELETON_JOINT_NAME : uint32_t
{
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_VRM_HIPS = 0,
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_COUNT = 55
};

enum BRX_ASSET_IMPORT_PBR_TEXTURE_NAME : uint32_t
{
    BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_BASE_COLOR = 0,
    BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_ROUGHNESS_METALLIC = 1,
    BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_NORMAL = 2,
    BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_EMISSIVE = 3,
    BRX_ASSET_IMPORT_TEXTURE_NAME_PBR_COUNT = 4
};

enum BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_TYPE : uint32_t
{
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_COPY_TRANSFORM = 0,
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_INVERSE_KINEMATICS = 1
};

enum BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_TYPE : uint32_t
{
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_SPHERE = 0,
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_BOX = 1,
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_CAPSULE = 2
};

enum BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_TYPE : uint32_t
{
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_FIXED = 0,
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_KEYFRAME = 1,
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_DYNAMIC = 2
};

enum BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_TYPE : uint32_t
{
    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_FIXED = 0,
    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_BALL_AND_SOCKET = 1,
    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_HINGE = 2,
    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_PRISMATIC = 3,
    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_RAGDOLL = 4
};

struct brx_asset_import_vertex_position
{
    // R32G32B32_FLOAT
    float m_position[3];
};

struct brx_asset_import_vertex_varying
{
    // R16G16_SNORM (octahedron map)
    uint32_t m_normal;
    // R15G15B2_SNORM (octahedron map + tangent w)
    uint32_t m_tangent;
    // R16G16_UNORM
    uint32_t m_texcoord;
};

struct brx_asset_import_vertex_blending
{
    // R16G16B16A16_UINT (xy)
    uint32_t m_indices_xy;
    // R16G16B16A16_UINT (wz)
    uint32_t m_indices_wz;
    // R8G8B8A8_UNORM
    uint32_t m_weights;
};

struct brx_asset_import_rigid_transform
{
    float m_rotation[4];
    float m_translation[3];
};

struct brx_asset_import_skeleton_joint_constraint
{
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_TYPE m_constraint_type;

    union
    {
        struct
        {
            uint32_t m_source_joint_index;
            uint32_t m_source_weight_count;
            float *m_source_weights;
            uint32_t m_destination_joint_index;
            bool m_copy_rotation;
            bool m_copy_translation;
        } m_copy_transform;

        struct
        {
            uint32_t m_ik_end_effector_index;
            uint32_t m_ik_joint_count;
            uint32_t *m_ik_joint_indices;
            uint32_t m_target_joint_index;
            float m_ik_two_joints_hinge_joint_axis_local_space[3];
            float m_cosine_max_ik_two_joints_hinge_joint_angle;
            float m_cosine_min_ik_two_joints_hinge_joint_angle;
        } m_inverse_kinematics;
    };
};

struct brx_asset_import_physics_rigid_body
{
    brx_asset_import_rigid_transform m_model_space_transform;
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_SHAPE_TYPE m_shape_type;
    float m_shape_size[3];
    BRX_ASSET_IMPORT_PHYSICS_RIGID_BODY_MOTION_TYPE m_motion_type;
    uint32_t m_collision_filter_group;
    uint32_t m_collision_filter_mask;
    float m_mass;
    float m_linear_damping;
    float m_angular_damping;
    float m_friction;
    float m_restitution;
};

struct brx_asset_import_physics_constraint
{
    uint32_t m_rigid_body_a_index;
    uint32_t m_rigid_body_b_index;
    BRX_ASSET_IMPORT_PHYSICS_CONSTRAINT_TYPE m_constraint_type;
    float m_pivot[3];
    float m_twist_axis[3];
    float m_plane_axis[3];
    float m_normal_axis[3];
    float m_twist_limit[2];
    float m_plane_limit[2];
    float m_normal_limit[2];
};

struct brx_asset_import_ragdoll_direct_mapping
{
    uint32_t m_joint_index_a;
    uint32_t m_joint_index_b;
    float m_a_to_b_transform_model_space[4][4];
};

class brx_asset_import_scene
{
public:
    virtual uint32_t get_surface_group_count() const = 0;
    virtual brx_asset_import_surface_group const *get_surface_group(uint32_t group_index) const = 0;
    virtual uint32_t get_animation_count() const = 0;
    virtual brx_asset_import_animation *get_animation(uint32_t animation_index) const = 0;
};

class brx_asset_import_surface_group
{
public:
    virtual uint32_t get_surface_count() const = 0;
    virtual brx_asset_import_surface const *get_surface(uint32_t surface_index) const = 0;

    // NULL: no skeleton
    // not NULL: skin
    virtual uint32_t const get_skeleton_joint_count() const = 0;
    virtual uint32_t get_skeleton_joint_parent_index(uint32_t skeleton_joint_index) const = 0;
    virtual brx_asset_import_rigid_transform const *get_skeleton_joint_bind_pose_transform_local_space(uint32_t skeleton_joint_index) const = 0;
    virtual uint32_t get_skeleton_joint_index(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME skeleton_joint_name) const = 0;
};

class brx_asset_import_surface
{
    virtual uint32_t get_vertex_count() const = 0;
    virtual brx_asset_import_vertex_position const *get_vertex_position() const = 0;
    virtual brx_asset_import_vertex_varying const *get_vertex_varying() const = 0;
    // NULL: no skin (even if there is one skeleton bound to the group)
    // not NULL: skin (there must be one skeleton bound to the group)
    virtual brx_asset_import_vertex_blending const *get_vertex_blending() const = 0;

    // geometry
    // 0: no morph animation
    // greater than 0: morph animation
    virtual uint32_t get_morph_target_count() const = 0;
    virtual brx_asset_import_vertex_position const *get_morph_target_vertex_positions(uint32_t morph_target_index) const = 0;
    virtual brx_asset_import_vertex_varying const *get_morph_target_vertex_varyings(uint32_t morph_target_index) const = 0;
    virtual uint32_t get_morph_target_name_index(BRX_ASSET_IMPORT_MORPH_TARGET_NAME morph_target_name) const = 0;

    // material
    virtual uint32_t get_texture_count() const = 0;
    // start with file:// : external file
    // start with data:// : internal data
    virtual char const *get_texture_url(uint32_t texture_index) const = 0;
    virtual uint32_t get_texture_index(BRX_ASSET_IMPORT_PBR_TEXTURE_NAME texture_name) const = 0;
};

class brx_asset_import_animation
{
public:
    virtual uint32_t const get_frame_count() const = 0;
    virtual uint32_t const get_weight_channel_count() const = 0;
    virtual char const *get_weight_channel_name(uint32_t channel_index) const = 0;
    virtual float const get_weight(uint32_t frame_index, uint32_t channel_index) const = 0;
    virtual uint32_t const get_rigid_transform_channel_count() const = 0;
    virtual char const *get_rigid_transform_channel_name(uint32_t channel_index) const = 0;
    virtual brx_asset_import_rigid_transform const *get_rigid_transform(uint32_t frame_index, uint32_t channel_index) const = 0;
    virtual uint32_t const get_ik_switch_channel_count() const = 0;
    virtual char const *get_ik_switch_channel_name(uint32_t channel_index) const = 0;
    virtual bool get_ik_switch(uint32_t frame_index, uint32_t channel_index) const = 0;
};

extern "C" brx_asset_import_scene *brx_asset_import_create_scene(brx_asset_import_input_stream_factory *input_stream_factory, char const *file_name);

#endif
