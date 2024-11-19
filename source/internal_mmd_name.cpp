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

#include "internal_mmd_name.h"
#include <cassert>

extern void internal_fill_mmd_morph_target_name_strings(mcrt_vector<mcrt_vector<mcrt_string>> &out_mmd_morph_target_name_strings)
{
    assert(BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT == out_mmd_morph_target_name_strings.size());
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_HAPPY].emplace_back(reinterpret_cast<char const *>(u8"にこり"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY].emplace_back(reinterpret_cast<char const *>(u8"怒り"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_ANGRY].emplace_back(reinterpret_cast<char const *>(u8"真面目"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SAD].emplace_back(reinterpret_cast<char const *>(u8"困る"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_BROW_SURPRISED].emplace_back(reinterpret_cast<char const *>(u8"上"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK].emplace_back(reinterpret_cast<char const *>(u8"まばたき"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L].emplace_back(reinterpret_cast<char const *>(u8"ウィンク２"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_L].emplace_back(reinterpret_cast<char const *>(u8"ウィンク"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R].emplace_back(reinterpret_cast<char const *>(u8"ｳｨﾝｸ２右"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_BLINK_R].emplace_back(reinterpret_cast<char const *>(u8"ウィンク右"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_HAPPY].emplace_back(reinterpret_cast<char const *>(u8"笑い"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_ANGRY].emplace_back(reinterpret_cast<char const *>(u8"ｷﾘｯ"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SAD].emplace_back(reinterpret_cast<char const *>(u8"じと目"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_EYE_SURPRISED].emplace_back(reinterpret_cast<char const *>(u8"びっくり"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A].emplace_back(reinterpret_cast<char const *>(u8"あ"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_A].emplace_back(reinterpret_cast<char const *>(u8"あ２"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_I].emplace_back(reinterpret_cast<char const *>(u8"い"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_U].emplace_back(reinterpret_cast<char const *>(u8"う"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_E].emplace_back(reinterpret_cast<char const *>(u8"え"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_O].emplace_back(reinterpret_cast<char const *>(u8"お"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY].emplace_back(reinterpret_cast<char const *>(u8"にっこり"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY].emplace_back(reinterpret_cast<char const *>(u8"にやり"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_HAPPY].emplace_back(reinterpret_cast<char const *>(u8"にやり２"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_ANGRY].emplace_back(reinterpret_cast<char const *>(u8"∧"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SAD].emplace_back(reinterpret_cast<char const *>(u8"口角下げ"));
    out_mmd_morph_target_name_strings[BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_MOUTH_SURPRISED].emplace_back(reinterpret_cast<char const *>(u8"▲"));
}

extern void internal_fill_mmd_skeleton_joint_name_strings(mcrt_vector<mcrt_vector<mcrt_string>> &out_mmd_skeleton_joint_name_strings)
{
    assert(BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT == out_mmd_skeleton_joint_name_strings.size());
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CONTROL_NODE].emplace_back(reinterpret_cast<char const *>(u8"操作中心"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_PARENT_NODE].emplace_back(reinterpret_cast<char const *>(u8"全ての親"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_CENTER].emplace_back(reinterpret_cast<char const *>(u8"センター"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_GROOVE].emplace_back(reinterpret_cast<char const *>(u8"グルーブ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_WAIST].emplace_back(reinterpret_cast<char const *>(u8"腰"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY].emplace_back(reinterpret_cast<char const *>(u8"上半身"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_UPPER_BODY_2].emplace_back(reinterpret_cast<char const *>(u8"上半身2"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_BREAST_1].emplace_back(reinterpret_cast<char const *>(u8"右胸１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_BREAST_1].emplace_back(reinterpret_cast<char const *>(u8"右胸"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_BREAST_2].emplace_back(reinterpret_cast<char const *>(u8"右胸２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_BREAST_2].emplace_back(reinterpret_cast<char const *>(u8"右胸先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_BREAST_1].emplace_back(reinterpret_cast<char const *>(u8"左胸１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_BREAST_1].emplace_back(reinterpret_cast<char const *>(u8"左胸"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_BREAST_2].emplace_back(reinterpret_cast<char const *>(u8"左胸２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_BREAST_2].emplace_back(reinterpret_cast<char const *>(u8"左胸先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_NECK].emplace_back(reinterpret_cast<char const *>(u8"首"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_HEAD].emplace_back(reinterpret_cast<char const *>(u8"頭"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE].emplace_back(reinterpret_cast<char const *>(u8"右目"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE_RETURN].emplace_back(reinterpret_cast<char const *>(u8"右目戻"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_EYE_RETURN].emplace_back(reinterpret_cast<char const *>(u8"右目先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE].emplace_back(reinterpret_cast<char const *>(u8"左目"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE_RETURN].emplace_back(reinterpret_cast<char const *>(u8"左目戻"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_EYE_RETURN].emplace_back(reinterpret_cast<char const *>(u8"左目先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_EYES].emplace_back(reinterpret_cast<char const *>(u8"両目"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_1].emplace_back(reinterpret_cast<char const *>(u8"舌１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_1].emplace_back(reinterpret_cast<char const *>(u8"舌1"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_2].emplace_back(reinterpret_cast<char const *>(u8"舌２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_2].emplace_back(reinterpret_cast<char const *>(u8"舌2"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_3].emplace_back(reinterpret_cast<char const *>(u8"舌３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_TONGUE_3].emplace_back(reinterpret_cast<char const *>(u8"舌3"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER_PARENT].emplace_back(reinterpret_cast<char const *>(u8"右肩P"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER].emplace_back(reinterpret_cast<char const *>(u8"右肩"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_SHOULDER_CHILD].emplace_back(reinterpret_cast<char const *>(u8"右肩C"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM].emplace_back(reinterpret_cast<char const *>(u8"右腕"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST].emplace_back(reinterpret_cast<char const *>(u8"右腕捩"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST_1].emplace_back(reinterpret_cast<char const *>(u8"右腕捩1"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST_2].emplace_back(reinterpret_cast<char const *>(u8"右腕捩2"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ARM_TWIST_3].emplace_back(reinterpret_cast<char const *>(u8"右腕捩3"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ELBOW].emplace_back(reinterpret_cast<char const *>(u8"右ひじ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ELBOW_AUX].emplace_back(reinterpret_cast<char const *>(u8"右ひじ補助"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_PLUS_ELBOW_AUX].emplace_back(reinterpret_cast<char const *>(u8"+右ひじ補助"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST].emplace_back(reinterpret_cast<char const *>(u8"右手捩"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST_1].emplace_back(reinterpret_cast<char const *>(u8"右手捩1"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST_2].emplace_back(reinterpret_cast<char const *>(u8"右手捩2"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TWIST_3].emplace_back(reinterpret_cast<char const *>(u8"右手捩3"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_WRIST].emplace_back(reinterpret_cast<char const *>(u8"右手首"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_DUMMY].emplace_back(reinterpret_cast<char const *>(u8"右ダミー"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_HAND_TIP].emplace_back(reinterpret_cast<char const *>(u8"右手先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_0].emplace_back(reinterpret_cast<char const *>(u8"右親指０"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_1].emplace_back(reinterpret_cast<char const *>(u8"右親指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_2].emplace_back(reinterpret_cast<char const *>(u8"右親指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_TIP].emplace_back(reinterpret_cast<char const *>(u8"右親指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_THUMB_TIP].emplace_back(reinterpret_cast<char const *>(u8"右親指２先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"右人指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"右人指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"右人指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右人指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_INDEX_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右人指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"右中指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"右中指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"右中指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右中指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_MIDDLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右中指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"右薬指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"右薬指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"右薬指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右薬指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_RING_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右薬指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"右小指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"右小指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"右小指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右小指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LITTLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"右小指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER_PARENT].emplace_back(reinterpret_cast<char const *>(u8"左肩P"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER].emplace_back(reinterpret_cast<char const *>(u8"左肩"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_SHOULDER_CHILD].emplace_back(reinterpret_cast<char const *>(u8"左肩C"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM].emplace_back(reinterpret_cast<char const *>(u8"左腕"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST].emplace_back(reinterpret_cast<char const *>(u8"左腕捩"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST_1].emplace_back(reinterpret_cast<char const *>(u8"左腕捩1"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST_2].emplace_back(reinterpret_cast<char const *>(u8"左腕捩2"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ARM_TWIST_3].emplace_back(reinterpret_cast<char const *>(u8"左腕捩3"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ELBOW].emplace_back(reinterpret_cast<char const *>(u8"左ひじ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ELBOW_AUX].emplace_back(reinterpret_cast<char const *>(u8"左ひじ補助"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_PLUS_ELBOW_AUX].emplace_back(reinterpret_cast<char const *>(u8"+左ひじ補助"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST].emplace_back(reinterpret_cast<char const *>(u8"左手捩"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST_1].emplace_back(reinterpret_cast<char const *>(u8"左手捩1"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST_2].emplace_back(reinterpret_cast<char const *>(u8"左手捩2"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TWIST_3].emplace_back(reinterpret_cast<char const *>(u8"左手捩3"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_WRIST].emplace_back(reinterpret_cast<char const *>(u8"左手首"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_DUMMY].emplace_back(reinterpret_cast<char const *>(u8"左ダミー"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_HAND_TIP].emplace_back(reinterpret_cast<char const *>(u8"左手先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_0].emplace_back(reinterpret_cast<char const *>(u8"左親指０"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_1].emplace_back(reinterpret_cast<char const *>(u8"左親指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_2].emplace_back(reinterpret_cast<char const *>(u8"左親指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_TIP].emplace_back(reinterpret_cast<char const *>(u8"左親指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_THUMB_TIP].emplace_back(reinterpret_cast<char const *>(u8"左親指２先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"左人指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"左人指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"左人指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左人指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_INDEX_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左人指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"左中指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"左中指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"左中指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左中指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_MIDDLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左中指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"左薬指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"左薬指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"左薬指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左薬指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_RING_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左薬指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_1].emplace_back(reinterpret_cast<char const *>(u8"左小指１"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_2].emplace_back(reinterpret_cast<char const *>(u8"左小指２"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_3].emplace_back(reinterpret_cast<char const *>(u8"左小指３"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左小指先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LITTLE_FINGER_TIP].emplace_back(reinterpret_cast<char const *>(u8"左小指３先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LOWER_BODY].emplace_back(reinterpret_cast<char const *>(u8"下半身"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_WAIST_CANCEL].emplace_back(reinterpret_cast<char const *>(u8"腰キャンセル右"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LEG].emplace_back(reinterpret_cast<char const *>(u8"右足"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_KNEE].emplace_back(reinterpret_cast<char const *>(u8"右ひざ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE].emplace_back(reinterpret_cast<char const *>(u8"右足首"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP].emplace_back(reinterpret_cast<char const *>(u8"右つま先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_LEG_DIRECTION].emplace_back(reinterpret_cast<char const *>(u8"右足D"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_KNEE_DIRECTION].emplace_back(reinterpret_cast<char const *>(u8"右ひざD"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE_DIRECTION].emplace_back(reinterpret_cast<char const *>(u8"右足首D"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP_EX].emplace_back(reinterpret_cast<char const *>(u8"右足先EX"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_WAIST_CANCEL].emplace_back(reinterpret_cast<char const *>(u8"腰キャンセル左"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LEG].emplace_back(reinterpret_cast<char const *>(u8"左足"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_KNEE].emplace_back(reinterpret_cast<char const *>(u8"左ひざ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE].emplace_back(reinterpret_cast<char const *>(u8"左足首"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP].emplace_back(reinterpret_cast<char const *>(u8"左つま先"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_LEG_DIRECTION].emplace_back(reinterpret_cast<char const *>(u8"左足D"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_KNEE_DIRECTION].emplace_back(reinterpret_cast<char const *>(u8"左ひざD"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE_DIRECTION].emplace_back(reinterpret_cast<char const *>(u8"左足首D"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP_EX].emplace_back(reinterpret_cast<char const *>(u8"左足先EX"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE_IK_PARENT].emplace_back(reinterpret_cast<char const *>(u8"右足IK親"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_ANKLE_IK].emplace_back(reinterpret_cast<char const *>(u8"右足ＩＫ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_RIGHT_TOE_TIP_IK].emplace_back(reinterpret_cast<char const *>(u8"右つま先ＩＫ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE_IK_PARENT].emplace_back(reinterpret_cast<char const *>(u8"左足IK親"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_ANKLE_IK].emplace_back(reinterpret_cast<char const *>(u8"左足ＩＫ"));
    out_mmd_skeleton_joint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_LEFT_TOE_TIP_IK].emplace_back(reinterpret_cast<char const *>(u8"左つま先ＩＫ"));
}

extern void internal_fill_mmd_skeleton_joint_constraint_name_strings(mcrt_vector<mcrt_vector<mcrt_string>> &out_mmd_skeleton_joint_constraint_name_strings)
{
    assert(BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_COUNT == out_mmd_skeleton_joint_constraint_name_strings.size());
    out_mmd_skeleton_joint_constraint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_ANKLE].emplace_back(reinterpret_cast<char const *>(u8"右足ＩＫ"));
    out_mmd_skeleton_joint_constraint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_RIGHT_TOE_TIP].emplace_back(reinterpret_cast<char const *>(u8"右つま先ＩＫ"));
    out_mmd_skeleton_joint_constraint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_ANKLE].emplace_back(reinterpret_cast<char const *>(u8"左足ＩＫ"));
    out_mmd_skeleton_joint_constraint_name_strings[BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_IK_LEFT_TOE_TIP].emplace_back(reinterpret_cast<char const *>(u8"左つま先ＩＫ"));
}
