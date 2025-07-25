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

#include <cstddef>
#include <cstdint>

// [Khronos ANARI (Analytic Rendering Interface): glTF To ANARI](https://github.com/KhronosGroup/ANARI-SDK/blob/next_release/src/anari_test_scenes/scenes/file/gltf2anari.h)

class brx_asset_import_scene;
class brx_asset_import_surface_group;
class brx_asset_import_surface;
class brx_asset_import_animation;

static constexpr uint32_t const BRX_ASSET_IMPORT_UINT32_INDEX_INVALID = static_cast<uint32_t>(~static_cast<uint32_t>(0U));

// [Miku Hatsune Append (Tda)](https://mikumikudance.fandom.com/wiki/Miku_Hatsune_Append_%28Tda%29)
// [Miku Hatsune (Sourxuan)](https://mikumikudance.fandom.com/wiki/Miku_Hatsune_%28Sourxuan%29)
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

// [Blender MMD Tools: Internal Dictionary](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/translations.py)
// [VRM 0.0: Defined Bones](https://github.com/vrm-c/vrm-specification/blob/master/specification/0.0/README.md#defined-bones)
// [VRM 1.0: List of Humanoid Bones](https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/humanoid.md#list-of-humanoid-bones)
// [VRM Addon for Blender: Human Bone Mapper MMD Mapping](https://github.com/saturday06/VRM-Addon-for-Blender/blob/main/src/io_scene_vrm/common/human_bone_mapper/mmd_mapping.py)

enum BRX_ASSET_IMPORT_SKELETON_JOINT_NAME : uint32_t
{
    // 操作中心
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CONTROL_NODE = 0,
    // 全ての親
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_PARENT_NODE = 1,
    // センター
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CENTER = 2,
    // グルーブ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_GROOVE = 3,
    // 腰
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_WAIST = 4,
    // 上半身
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY = 5,
    // 上半身2
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY_2 = 6,
    // 右胸１
    // 右胸
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_BREAST_1 = 7,
    // 右胸２
    // 右胸先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_BREAST_2 = 8,
    // 左胸１
    // 左胸
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_BREAST_1 = 9,
    // 左胸２
    // 左胸先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_BREAST_2 = 10,
    // 首
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_NECK = 11,
    // 頭
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_HEAD = 12,
    // 右目
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE = 13,
    // 右目戻
    // 右目先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE_RETURN = 14,
    // 左目
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE = 15,
    // 左目戻
    // 左目先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE_RETURN = 16,
    // 両目
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_EYES = 17,
    // 舌１
    // 舌1
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_1 = 18,
    // 舌２
    // 舌2
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_2 = 19,
    // 舌３
    // 舌3
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_3 = 20,
    // 右肩P
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER_PARENT = 21,
    // 右肩
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER = 22,
    // 右肩C
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER_CHILD = 23,
    // 右腕
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM = 24,
    // 右腕捩
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST = 25,
    // 右腕捩1
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST_1 = 26,
    // 右腕捩2
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST_2 = 27,
    // 右腕捩3
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST_3 = 28,
    // 右ひじ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ELBOW = 29,
    // 右ひじ補助
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ELBOW_AUX = 30,
    // +右ひじ補助
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_PLUS_ELBOW_AUX = 31,
    // 右手捩
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST = 32,
    // 右手捩1
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST_1 = 33,
    // 右手捩2
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST_2 = 34,
    // 右手捩3
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST_3 = 35,
    // 右手首
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_WRIST = 36,
    // 右ダミー
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_DUMMY = 37,
    // 右手先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TIP = 38,
    // 右親指０
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_0 = 39,
    // 右親指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_1 = 40,
    // 右親指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_2 = 41,
    // 右親指先
    // 右親指２先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_TIP = 42,
    // 右人指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_1 = 43,
    // 右人指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_2 = 44,
    // 右人指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_3 = 45,
    // 右人指先
    // 右人指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_TIP = 46,
    // 右中指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_1 = 47,
    // 右中指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_2 = 48,
    // 右中指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_3 = 49,
    // 右中指先
    // 右中指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_TIP = 50,
    // 右薬指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_1 = 51,
    // 右薬指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_2 = 52,
    // 右薬指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_3 = 53,
    // 右薬指先
    // 右薬指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_TIP = 54,
    // 右小指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_1 = 55,
    // 右小指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_2 = 56,
    // 右小指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_3 = 57,
    // 右小指先
    // 右小指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_TIP = 58,
    // 左肩P
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER_PARENT = 59,
    // 左肩
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER = 60,
    // 左肩C
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER_CHILD = 61,
    // 左腕
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM = 62,
    // 左腕捩
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST = 63,
    // 左腕捩1
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST_1 = 64,
    // 左腕捩2
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST_2 = 65,
    // 左腕捩3
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST_3 = 66,
    // 左ひじ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ELBOW = 67,
    // 左ひじ補助
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ELBOW_AUX = 68,
    // +左ひじ補助
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_PLUS_ELBOW_AUX = 69,
    // 左手捩
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST = 70,
    // 左手捩1
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST_1 = 71,
    // 左手捩2
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST_2 = 72,
    // 左手捩3
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST_3 = 73,
    // 左手首
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_WRIST = 74,
    // 左ダミー
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_DUMMY = 75,
    // 左手先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TIP = 76,
    // 左親指０
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_0 = 77,
    // 左親指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_1 = 78,
    // 左親指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_2 = 79,
    // 左親指先
    // 左親指２先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_TIP = 80,
    // 左人指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_1 = 81,
    // 左人指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_2 = 82,
    // 左人指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_3 = 83,
    // 左人指先
    // 左人指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_TIP = 84,
    // 左中指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_1 = 85,
    // 左中指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_2 = 86,
    // 左中指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_3 = 87,
    // 左中指先
    // 左中指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_TIP = 88,
    // 左薬指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_1 = 89,
    // 左薬指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_2 = 90,
    // 左薬指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_3 = 91,
    // 左薬指先
    // 左薬指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_TIP = 92,
    // 左小指１
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_1 = 93,
    // 左小指２
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_2 = 94,
    // 左小指３
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_3 = 95,
    // 左小指先
    // 左小指３先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_TIP = 96,
    // 下半身
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LOWER_BODY = 97,
    // 腰キャンセル右
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_WAIST_CANCEL = 98,
    // 右足
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LEG = 99,
    // 右ひざ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_KNEE = 100,
    // 右足首
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE = 101,
    // 右つま先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP = 102,
    // 右足D
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LEG_DIRECTION = 103,
    // 右ひざD
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_KNEE_DIRECTION = 104,
    // 右足首D
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE_DIRECTION = 105,
    // 右足先EX
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP_EX = 106,
    // 腰キャンセル左
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_WAIST_CANCEL = 107,
    // 左足
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LEG = 108,
    // 左ひざ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_KNEE = 109,
    // 左足首
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE = 110,
    // 左つま先
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP = 111,
    // 左足D
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LEG_DIRECTION = 112,
    // 左ひざD
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_KNEE_DIRECTION = 113,
    // 左足首D
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE_DIRECTION = 114,
    // 左足先EX
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP_EX = 115,
    // 右足IK親
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE_IK_PARENT = 116,
    // 右足ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE_IK = 117,
    // 右つま先ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP_IK = 118,
    // 左足IK親
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE_IK_PARENT = 119,
    // 左足ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE_IK = 120,
    // 左つま先ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP_IK = 121,
    //
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT = 122,
    //
    BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_INVALID = 0X7FFFFFFF
};

enum BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME : uint32_t
{
    // 右足ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_ANKLE = 0,
    // 右つま先ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_TOE_TIP = 1,
    // 左足ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_ANKLE = 2,
    // 左つま先ＩＫ
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_TOE_TIP = 3,
    //
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_COUNT = 4,
    //
    BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_INVALID = 0X7FFFFFFF

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

struct brx_asset_import_surface_vertex_position
{
    float m_position[3];
};

struct brx_asset_import_surface_vertex_varying
{
    float m_normal[3];
    float m_tangent[4];
    float m_texcoord[2];
};

struct brx_asset_import_surface_vertex_blending
{
    uint32_t m_indices[4];
    float m_weights[4];
};

struct brx_asset_import_vec3
{
    float m_x;
    float m_y;
    float m_z;
};

struct brx_asset_import_vec4
{
    float m_x;
    float m_y;
    float m_z;
    float m_w;
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
    virtual brx_asset_import_animation const *get_animation(uint32_t animation_index) const = 0;
};

class brx_asset_import_surface_group
{
public:
    virtual uint32_t get_surface_count() const = 0;
    virtual brx_asset_import_surface const *get_surface(uint32_t surface_index) const = 0;

    // 0: no skeleton
    // greater than 0: skin
    virtual uint32_t get_animation_skeleton_joint_count() const = 0;
    virtual BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *get_animation_skeleton_joint_names() const = 0;
    virtual uint32_t const *get_animation_skeleton_joint_parent_indices() const = 0;
    virtual brx_asset_import_rigid_transform const *get_animation_skeleton_joint_transforms_bind_pose_local_space() const = 0;

    // 0: no skeleton
    // greater than 0: skin
    virtual uint32_t get_animation_skeleton_joint_constraint_count() const = 0;
    virtual BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *get_animation_skeleton_joint_constraint_names() const = 0;
    virtual brx_asset_import_skeleton_joint_constraint const *get_animation_skeleton_joint_constraints() const = 0;

    // 0: no skeleton
    // greater than 0: skin
    virtual uint32_t get_ragdoll_skeleton_rigid_body_count() const = 0;
    virtual brx_asset_import_physics_rigid_body const *get_ragdoll_skeleton_rigid_bodies() const = 0;

    // 0: no skeleton
    // greater than 0: skin
    virtual uint32_t get_ragdoll_skeleton_constraint_count() const = 0;
    virtual brx_asset_import_physics_constraint const *get_ragdoll_skeleton_constraints() const = 0;

    // 0: no skeleton
    // greater than 0: skin
    virtual uint32_t get_animation_to_ragdoll_direct_mapping_count() const = 0;
    virtual brx_asset_import_ragdoll_direct_mapping const *get_animation_to_ragdoll_direct_mappings() const = 0;

    // 0: no skeleton
    // greater than 0: skin
    virtual uint32_t get_ragdoll_to_animation_direct_mapping_count() const = 0;
    virtual brx_asset_import_ragdoll_direct_mapping const *get_ragdoll_to_animation_direct_mappings() const = 0;
};

class brx_asset_import_surface
{
public:
    virtual uint32_t get_vertex_count() const = 0;
    virtual brx_asset_import_surface_vertex_position const *get_vertex_positions() const = 0;
    virtual brx_asset_import_surface_vertex_varying const *get_vertex_varyings() const = 0;
    // NULL: no skin (even if there is one skeleton bound to the group)
    // not NULL: skin (there must be one skeleton bound to the group)
    virtual brx_asset_import_surface_vertex_blending const *get_vertex_blendings() const = 0;

    // geometry
    // 0: no morph animation
    // greater than 0: morph animation
    virtual uint32_t get_morph_target_count() const = 0;
    virtual BRX_ASSET_IMPORT_MORPH_TARGET_NAME get_morph_target_name(uint32_t morph_target_index) const = 0;
    virtual brx_asset_import_surface_vertex_position const *get_morph_target_vertex_positions(uint32_t morph_target_index) const = 0;
    virtual brx_asset_import_surface_vertex_varying const *get_morph_target_vertex_varyings(uint32_t morph_target_index) const = 0;

    virtual uint32_t get_index_count() const = 0;
    virtual uint32_t const *get_indices() const = 0;

    // material
    // start with file:// : external file
    // start with data:// : internal data
    virtual uint8_t const *get_emissive_image_url() const = 0;
    virtual brx_asset_import_vec3 get_emissive_factor() const = 0;
    virtual uint8_t const *get_normal_image_url() const = 0;
    virtual float get_normal_scale() const = 0;
    virtual uint8_t const *get_base_color_image_url() const = 0;
    virtual brx_asset_import_vec4 get_base_color_factor() const = 0;
    virtual uint8_t const *get_metallic_roughness_image_url() const = 0;
    virtual float get_metallic_factor() const = 0;
    virtual float get_roughness_factor() const = 0;
};

class brx_asset_import_animation
{
public:
    virtual uint32_t const get_frame_count() const = 0;

    virtual uint32_t const get_weight_channel_count() const = 0;
    virtual BRX_ASSET_IMPORT_MORPH_TARGET_NAME const *get_weight_channel_names() const = 0;
    virtual float const *get_weights() const = 0;

    virtual uint32_t const get_rigid_transform_channel_count() const = 0;
    virtual BRX_ASSET_IMPORT_SKELETON_JOINT_NAME const *get_rigid_transform_channel_names() const = 0;
    virtual brx_asset_import_rigid_transform const *get_rigid_transforms() const = 0;

    virtual uint32_t const get_switch_channel_count() const = 0;
    virtual BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME const *get_switch_channel_names() const = 0;
    virtual uint8_t const *get_switches() const = 0;
};

extern "C" brx_asset_import_scene *brx_asset_import_create_scene_from_memory(void const *data_base, size_t data_size);
extern "C" void brx_asset_import_destroy_scene(brx_asset_import_scene *scene);

#endif
