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

#include "internal_import_mmd_motion.h"
#include "internal_mmd_vmd.h"
#include "internal_mmd_name.h"
#include "../../McRT-Malloc/include/mcrt_map.h"
#include "../../McRT-Malloc/include/mcrt_unordered_map.h"
#if defined(__GNUC__)
// GCC or CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <DirectXMath.h>
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
// MSVC or CLANG-CL
#include <DirectXMath.h>
#else
#error Unknown Compiler
#endif
#include <algorithm>
#include <cmath>
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

struct internal_rigid_transform_key_frame_t
{
    brx_asset_import_rigid_transform m_rigid_transform;
    uint8_t m_translation_x_cubic_bezier[4];
    uint8_t m_translation_y_cubic_bezier[4];
    uint8_t m_translation_z_cubic_bezier[4];
    uint8_t m_rotation_cubic_bezier[4];
};

static inline float internal_cubic_bezier(uint8_t const in_packed_k_1_x, uint8_t const in_packed_k_1_y, uint8_t const in_packed_k_2_x, uint8_t const in_packed_k_2_y, float const in_x);

static inline mmd_vmd_vec3_t internal_transform_translation(mmd_vmd_vec3_t const &v);

static inline mmd_vmd_vec4_t internal_transform_rotation(mmd_vmd_vec4_t const &q);

extern bool internal_import_mmd_motion(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_mesh_animation> &out_animations)
{
    mcrt_vector<BRX_ASSET_IMPORT_MORPH_TARGET_NAME> weight_channel_names;
    mcrt_vector<float> weights;
    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME> rigid_transform_channel_names;
    mcrt_vector<brx_asset_import_rigid_transform> rigid_transforms;
    mcrt_vector<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME> switch_channel_names;
    mcrt_vector<uint8_t> switches;
    {
        uint32_t max_frame_number = 0U;
        mcrt_unordered_map<mcrt_string, mcrt_map<uint32_t, float>> weight_channels;
        mcrt_unordered_map<mcrt_string, mcrt_map<uint32_t, internal_rigid_transform_key_frame_t>> rigid_transform_channels;
        mcrt_unordered_map<mcrt_string, mcrt_map<uint32_t, bool>> switch_channels;
        {
            mmd_vmd_t mmd_vmd;
            if (internal_unlikely(!internal_data_read_mmd_vmd(data_base, data_size, &mmd_vmd)))
            {
                return false;
            }

            for (mmd_vmd_morph_t const &mmd_vmd_morph : mmd_vmd.m_morphs)
            {
                max_frame_number = std::max(mmd_vmd_morph.m_frame_number, max_frame_number);

                assert(weight_channels[mmd_vmd_morph.m_name].end() == weight_channels[mmd_vmd_morph.m_name].find(mmd_vmd_morph.m_frame_number));
                weight_channels[mmd_vmd_morph.m_name][mmd_vmd_morph.m_frame_number] = mmd_vmd_morph.m_weight;
            }

            for (mmd_vmd_motion_t const &mmd_vmd_motion : mmd_vmd.m_motions)
            {
                max_frame_number = std::max(mmd_vmd_motion.m_frame_number, max_frame_number);

                mmd_vmd_vec4_t const mmd_vmd_motion_rotation = internal_transform_rotation(mmd_vmd_motion.m_rotation);
                mmd_vmd_vec3_t const mmd_vmd_motion_translation = internal_transform_translation(mmd_vmd_motion.m_translation);

                assert(rigid_transform_channels[mmd_vmd_motion.m_name].end() == rigid_transform_channels[mmd_vmd_motion.m_name].find(mmd_vmd_motion.m_frame_number));
                rigid_transform_channels[mmd_vmd_motion.m_name][mmd_vmd_motion.m_frame_number] = internal_rigid_transform_key_frame_t{
                    {
                        {mmd_vmd_motion_rotation.m_x, mmd_vmd_motion_rotation.m_y, mmd_vmd_motion_rotation.m_z, mmd_vmd_motion_rotation.m_w},
                        {mmd_vmd_motion_translation.m_x, mmd_vmd_motion_translation.m_y, mmd_vmd_motion_translation.m_z},
                    },
                    {mmd_vmd_motion.m_translation_x_cubic_bezier[0], mmd_vmd_motion.m_translation_x_cubic_bezier[1], mmd_vmd_motion.m_translation_x_cubic_bezier[2], mmd_vmd_motion.m_translation_x_cubic_bezier[3]},
                    {mmd_vmd_motion.m_translation_y_cubic_bezier[0], mmd_vmd_motion.m_translation_y_cubic_bezier[1], mmd_vmd_motion.m_translation_y_cubic_bezier[2], mmd_vmd_motion.m_translation_y_cubic_bezier[3]},
                    {mmd_vmd_motion.m_translation_z_cubic_bezier[0], mmd_vmd_motion.m_translation_z_cubic_bezier[1], mmd_vmd_motion.m_translation_z_cubic_bezier[2], mmd_vmd_motion.m_translation_z_cubic_bezier[3]},
                    {mmd_vmd_motion.m_rotation_cubic_bezier[0], mmd_vmd_motion.m_rotation_cubic_bezier[1], mmd_vmd_motion.m_rotation_cubic_bezier[2], mmd_vmd_motion.m_rotation_cubic_bezier[3]}};
            }

            for (mmd_vmd_ik_t const &mmd_vmd_iks : mmd_vmd.m_iks)
            {
                max_frame_number = std::max(mmd_vmd_iks.m_frame_number, max_frame_number);

                assert(switch_channels[mmd_vmd_iks.m_name].end() == switch_channels[mmd_vmd_iks.m_name].find(mmd_vmd_iks.m_frame_number));
                switch_channels[mmd_vmd_iks.m_name][mmd_vmd_iks.m_frame_number] = mmd_vmd_iks.m_enable;
            }
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
        uint32_t const frame_count = max_frame_number;

        constexpr uint32_t mmd_morph_target_name_count = BRX_ASSET_IMPORT_MORPH_TARGET_NAME_MMD_COUNT;
        mcrt_vector<mcrt_string> weight_channel_name_strings;
        constexpr uint32_t mmd_skeleton_joint_name_count = BRX_ASSET_IMPORT_SKELETON_JOINT_NAME_MMD_COUNT;
        mcrt_vector<mcrt_string> rigid_transform_channel_name_strings;
        constexpr uint32_t mmd_skeleton_joint_constraint_name_count = BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME_MMD_COUNT;
        mcrt_vector<mcrt_string> switch_channel_name_strings;
        {
            mcrt_vector<mcrt_vector<mcrt_string>> mmd_morph_target_name_strings(static_cast<size_t>(mmd_morph_target_name_count));
            internal_fill_mmd_morph_target_name_strings(mmd_morph_target_name_strings);

            for (uint32_t mmd_morph_target_name = 0U; mmd_morph_target_name < mmd_morph_target_name_count; ++mmd_morph_target_name)
            {
                for (mcrt_string const &mmd_morph_target_name_string : mmd_morph_target_name_strings[mmd_morph_target_name])
                {
                    auto const found_weight_channel = weight_channels.find(mmd_morph_target_name_string);
                    if (weight_channels.end() != found_weight_channel)
                    {
                        weight_channel_names.push_back(static_cast<BRX_ASSET_IMPORT_MORPH_TARGET_NAME>(mmd_morph_target_name));
                        weight_channel_name_strings.push_back(mmd_morph_target_name_string);
                        break;
                    }
                }
            }

            mcrt_vector<mcrt_vector<mcrt_string>> mmd_skeleton_joint_name_strings(static_cast<size_t>(mmd_skeleton_joint_name_count));
            internal_fill_mmd_skeleton_joint_name_strings(mmd_skeleton_joint_name_strings);

            for (uint32_t mmd_skeleton_joint_name = 0U; mmd_skeleton_joint_name < mmd_skeleton_joint_name_count; ++mmd_skeleton_joint_name)
            {
                for (mcrt_string const &mmd_skeleton_joint_name_string : mmd_skeleton_joint_name_strings[mmd_skeleton_joint_name])
                {
                    auto const found_rigid_transform_channel = rigid_transform_channels.find(mmd_skeleton_joint_name_string);
                    if (rigid_transform_channels.end() != found_rigid_transform_channel)
                    {
                        rigid_transform_channel_names.push_back(static_cast<BRX_ASSET_IMPORT_SKELETON_JOINT_NAME>(mmd_skeleton_joint_name));
                        rigid_transform_channel_name_strings.push_back(mmd_skeleton_joint_name_string);
                        break;
                    }
                }
            }

            mcrt_vector<mcrt_vector<mcrt_string>> mmd_skeleton_joint_constraint_name_strings(static_cast<size_t>(mmd_skeleton_joint_constraint_name_count));
            internal_fill_mmd_skeleton_joint_constraint_name_strings(mmd_skeleton_joint_constraint_name_strings);

            for (uint32_t mmd_skeleton_joint_constraint_name = 0U; mmd_skeleton_joint_constraint_name < mmd_skeleton_joint_constraint_name_count; ++mmd_skeleton_joint_constraint_name)
            {
                for (mcrt_string const &mmd_skeleton_joint_constraint_name_string : mmd_skeleton_joint_constraint_name_strings[mmd_skeleton_joint_constraint_name])
                {
                    auto const found_switch_channel = switch_channels.find(mmd_skeleton_joint_constraint_name_string);
                    if (switch_channels.end() != found_switch_channel)
                    {
                        switch_channel_names.push_back(static_cast<BRX_ASSET_IMPORT_SKELETON_JOINT_CONSTRAINT_NAME>(mmd_skeleton_joint_constraint_name));
                        switch_channel_name_strings.push_back(mmd_skeleton_joint_constraint_name_string);
                        break;
                    }
                }
            }
        }

        uint32_t const weight_channel_count = weight_channel_names.size();
        assert(weight_channel_name_strings.size() == weight_channel_count);
        assert(weight_channel_count <= mmd_morph_target_name_count);
        // Initilize "0.0F" for NOT found
        weights.resize(static_cast<size_t>(weight_channel_count * frame_count), 0.0F);

        uint32_t const rigid_transform_channel_count = rigid_transform_channel_names.size();
        assert(rigid_transform_channel_name_strings.size() == rigid_transform_channel_count);
        assert(rigid_transform_channel_count <= mmd_skeleton_joint_name_count);
        // Initialize "Identity" for NOT found
        rigid_transforms.resize(static_cast<size_t>(rigid_transform_channel_count * frame_count), brx_asset_import_rigid_transform{{0.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 0.0F}});

        uint32_t const switch_channel_count = switch_channel_names.size();
        assert(switch_channel_name_strings.size() == switch_channel_count);
        assert(switch_channel_count <= mmd_morph_target_name_count);
        // Initilize "true" for NOT found
        switches.resize(static_cast<size_t>(switch_channel_count * frame_count), true);

        for (uint32_t frame_index = 0; frame_index < frame_count; ++frame_index)
        {
            for (uint32_t weight_channel_index = 0U; weight_channel_index < weight_channel_count; ++weight_channel_index)
            {
                auto const found_weight_channel = weight_channels.find(weight_channel_name_strings[weight_channel_index]);

                if (weight_channels.end() != found_weight_channel)
                {
                    mcrt_map<uint32_t, float> const &weight_channel = found_weight_channel->second;

                    auto const &key_upper_bound = weight_channel.upper_bound(frame_index);

                    if (weight_channel.end() != key_upper_bound)
                    {
                        if (weight_channel.begin() != key_upper_bound)
                        {
                            auto const &key_next = key_upper_bound;
                            auto const &key_previous = std::prev(key_upper_bound);

                            float const sample_time = static_cast<float>(frame_index) + 0.5F;
                            assert((0U == max_frame_number) || (sample_time < max_frame_number));

                            assert(static_cast<float>(key_next->first) > sample_time);
                            assert(sample_time > static_cast<float>(key_previous->first));

                            float const normalized_time = (sample_time - static_cast<float>(key_previous->first)) / (static_cast<float>(key_next->first) - static_cast<float>(key_previous->first));
                            assert((normalized_time >= 0.0F) && (normalized_time <= 1.0F));

                            assert(0.0F == weights[weight_channel_count * frame_index + weight_channel_index]);
                            weights[weight_channel_count * frame_index + weight_channel_index] = (key_next->second - key_previous->second) * normalized_time + key_previous->second;
                        }
                        else
                        {
                            auto const &key_next = key_upper_bound;

                            float const sample_time = static_cast<float>(frame_index) + 0.5F;
                            assert((0U == max_frame_number) || (sample_time < max_frame_number));

                            assert(static_cast<float>(key_next->first) > sample_time);

                            // weight of the bind pose is zero
                            constexpr uint32_t const key_previous_frame_number = 0U;
                            constexpr float const key_previous_weight = 0.0F;

                            float const normalized_time = (sample_time - static_cast<float>(key_previous_frame_number)) / (static_cast<float>(key_next->first) - static_cast<float>(key_previous_frame_number));
                            assert((normalized_time >= 0.0F) && (normalized_time <= 1.0F));

                            assert(0.0F == weights[weight_channel_count * frame_index + weight_channel_index]);
                            weights[weight_channel_count * frame_index + weight_channel_index] = (key_next->second - key_previous_weight) * normalized_time + key_previous_weight;
                        }
                    }
                    else
                    {
                        auto const &key_previous = std::prev(key_upper_bound);
                        assert(&(*weight_channel.rbegin()) == &(*key_previous));

                        float const sample_time = static_cast<float>(frame_index) + 0.5F;
                        assert((0U == max_frame_number) || (sample_time < max_frame_number));
                        assert(sample_time > static_cast<float>(key_previous->first));

                        assert(0.0F == weights[weight_channel_count * frame_index + weight_channel_index]);
                        weights[weight_channel_count * frame_index + weight_channel_index] = key_previous->second;
                    }
                }
                else
                {
                    assert(false);
                }
            }

            for (uint32_t rigid_transform_channel_index = 0U; rigid_transform_channel_index < rigid_transform_channel_count; ++rigid_transform_channel_index)
            {
                auto const found_rigid_transform_channel = rigid_transform_channels.find(rigid_transform_channel_name_strings[rigid_transform_channel_index]);

                if (rigid_transform_channels.end() != found_rigid_transform_channel)
                {
                    mcrt_map<uint32_t, internal_rigid_transform_key_frame_t> const &rigid_transform_channel = found_rigid_transform_channel->second;

                    auto const &key_upper_bound = rigid_transform_channel.upper_bound(frame_index);

                    if (rigid_transform_channel.end() != key_upper_bound)
                    {
                        if (rigid_transform_channel.begin() != key_upper_bound)
                        {
                            auto const &key_next = key_upper_bound;
                            auto const &key_previous = std::prev(key_upper_bound);

                            float const sample_time = static_cast<float>(frame_index) + 0.5F;
                            assert((0U == max_frame_number) || (sample_time < max_frame_number));

                            assert(static_cast<float>(key_next->first) > sample_time);
                            assert(sample_time > static_cast<float>(key_previous->first));

                            float const normalized_time = (sample_time - static_cast<float>(key_previous->first)) / (static_cast<float>(key_next->first) - static_cast<float>(key_previous->first));
                            assert((normalized_time >= 0.0F) && (normalized_time <= 1.0F));

                            float const rotation_lerp_factor = internal_cubic_bezier(key_previous->second.m_rotation_cubic_bezier[0], key_previous->second.m_rotation_cubic_bezier[1], key_previous->second.m_rotation_cubic_bezier[2], key_previous->second.m_rotation_cubic_bezier[3], normalized_time);
                            float const translation_x_lerp_factor = internal_cubic_bezier(key_previous->second.m_translation_x_cubic_bezier[0], key_previous->second.m_translation_x_cubic_bezier[1], key_previous->second.m_translation_x_cubic_bezier[2], key_previous->second.m_translation_x_cubic_bezier[3], normalized_time);
                            float const translation_y_lerp_factor = internal_cubic_bezier(key_previous->second.m_translation_y_cubic_bezier[0], key_previous->second.m_translation_y_cubic_bezier[1], key_previous->second.m_translation_y_cubic_bezier[2], key_previous->second.m_translation_y_cubic_bezier[3], normalized_time);
                            float const translation_z_lerp_factor = internal_cubic_bezier(key_previous->second.m_translation_z_cubic_bezier[0], key_previous->second.m_translation_z_cubic_bezier[1], key_previous->second.m_translation_z_cubic_bezier[2], key_previous->second.m_translation_z_cubic_bezier[3], normalized_time);

                            DirectX::XMFLOAT4 const rotation_previous(key_previous->second.m_rigid_transform.m_rotation[0], key_previous->second.m_rigid_transform.m_rotation[1], key_previous->second.m_rigid_transform.m_rotation[2], key_previous->second.m_rigid_transform.m_rotation[3]);
                            DirectX::XMFLOAT4 const rotation_next(key_next->second.m_rigid_transform.m_rotation[0], key_next->second.m_rigid_transform.m_rotation[1], key_next->second.m_rigid_transform.m_rotation[2], key_next->second.m_rigid_transform.m_rotation[3]);

                            DirectX::XMFLOAT4 sample_rotation;
                            DirectX::XMStoreFloat4(&sample_rotation, DirectX::XMQuaternionNormalize(DirectX::XMQuaternionSlerp(DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&rotation_previous)), DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&rotation_next)), rotation_lerp_factor)));

                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[0] = sample_rotation.x;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[1] = sample_rotation.y;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[2] = sample_rotation.z;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[3] = sample_rotation.w;

                            DirectX::XMFLOAT3 const translation_previous(key_previous->second.m_rigid_transform.m_translation[0], key_previous->second.m_rigid_transform.m_translation[1], key_previous->second.m_rigid_transform.m_translation[2]);
                            DirectX::XMFLOAT3 const translation_next(key_next->second.m_rigid_transform.m_translation[0], key_next->second.m_rigid_transform.m_translation[1], key_next->second.m_rigid_transform.m_translation[2]);
                            DirectX::XMFLOAT3 const lerp_factor(translation_x_lerp_factor, translation_y_lerp_factor, translation_z_lerp_factor);

                            DirectX::XMFLOAT3 sample_translation;
                            DirectX::XMStoreFloat3(&sample_translation, DirectX::XMVectorLerpV(DirectX::XMLoadFloat3(&translation_previous), DirectX::XMLoadFloat3(&translation_next), DirectX::XMLoadFloat3(&lerp_factor)));

                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[0] = sample_translation.x;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[1] = sample_translation.y;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[2] = sample_translation.z;
                        }
                        else
                        {
                            auto const &key_next = key_upper_bound;

                            float const sample_time = static_cast<float>(frame_index) + 0.5F;
                            assert((0U == max_frame_number) || (sample_time < max_frame_number));

                            assert(static_cast<float>(key_next->first) > sample_time);

                            DirectX::XMFLOAT4 const rotation_next(key_next->second.m_rigid_transform.m_rotation[0], key_next->second.m_rigid_transform.m_rotation[1], key_next->second.m_rigid_transform.m_rotation[2], key_next->second.m_rigid_transform.m_rotation[3]);

                            DirectX::XMFLOAT4 rotation_next_normalized;
                            DirectX::XMStoreFloat4(&rotation_next_normalized, DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&rotation_next)));

                            // TODO: lerp from the bind pose
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[0] = rotation_next_normalized.x;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[1] = rotation_next_normalized.y;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[2] = rotation_next_normalized.z;
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[3] = rotation_next_normalized.w;

                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[0] = key_next->second.m_rigid_transform.m_translation[0];
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[1] = key_next->second.m_rigid_transform.m_translation[1];
                            rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[2] = key_next->second.m_rigid_transform.m_translation[2];
                        }
                    }
                    else
                    {
                        auto const &key_previous = std::prev(key_upper_bound);
                        assert(&(*rigid_transform_channel.rbegin()) == &(*key_previous));

                        float const sample_time = static_cast<float>(frame_index) + 0.5F;
                        assert((0U == max_frame_number) || (sample_time < max_frame_number));
                        assert(sample_time > static_cast<float>(key_previous->first));

                        DirectX::XMFLOAT4 const rotation_previous(key_previous->second.m_rigid_transform.m_rotation[0], key_previous->second.m_rigid_transform.m_rotation[1], key_previous->second.m_rigid_transform.m_rotation[2], key_previous->second.m_rigid_transform.m_rotation[3]);

                        DirectX::XMFLOAT4 rotation_previous_normalized;
                        DirectX::XMStoreFloat4(&rotation_previous_normalized, DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&rotation_previous)));

                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[0] = rotation_previous_normalized.x;
                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[1] = rotation_previous_normalized.y;
                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[2] = rotation_previous_normalized.z;
                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_rotation[3] = rotation_previous_normalized.w;

                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[0] = key_previous->second.m_rigid_transform.m_translation[0];
                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[1] = key_previous->second.m_rigid_transform.m_translation[1];
                        rigid_transforms[rigid_transform_channel_count * frame_index + rigid_transform_channel_index].m_translation[2] = key_previous->second.m_rigid_transform.m_translation[2];
                    }
                }
                else
                {
                    assert(false);
                }
            }

            for (uint32_t switch_channel_index = 0U; switch_channel_index < switch_channel_count; ++switch_channel_index)
            {
                auto const found_switch_channel = switch_channels.find(switch_channel_name_strings[switch_channel_index]);

                if (switch_channels.end() != found_switch_channel)
                {
                    mcrt_map<uint32_t, bool> const &switch_channel = found_switch_channel->second;

                    auto const &key_upper_bound = switch_channel.upper_bound(frame_index);

                    if (switch_channel.end() != key_upper_bound)
                    {
                        if (switch_channel.begin() != key_upper_bound)
                        {
                            auto const &key_next = key_upper_bound;
                            auto const &key_previous = std::prev(key_upper_bound);

                            float const sample_time = static_cast<float>(frame_index) + 0.5F;
                            assert((0U == max_frame_number) || (sample_time < max_frame_number));

                            assert(static_cast<float>(key_next->first) > sample_time);
                            assert(sample_time > static_cast<float>(key_previous->first));

                            switches[switch_channel_count * frame_index + switch_channel_index] = key_previous->second;
                        }
                        else
                        {
                            auto const &key_next = key_upper_bound;

                            float const sample_time = static_cast<float>(frame_index) + 0.5F;
                            assert((0U == max_frame_number) || (sample_time < max_frame_number));

                            assert(static_cast<float>(key_next->first) > sample_time);

                            switches[switch_channel_count * frame_index + switch_channel_index] = key_next->second;
                        }
                    }
                    else
                    {
                        auto const &key_previous = std::prev(key_upper_bound);
                        assert(&(*switch_channel.rbegin()) == &(*key_previous));

                        float const sample_time = static_cast<float>(frame_index) + 0.5F;
                        assert((0U == max_frame_number) || (sample_time < max_frame_number));
                        assert(sample_time > static_cast<float>(key_previous->first));

                        switches[switch_channel_count * frame_index + switch_channel_index] = key_previous->second;
                    }
                }
                else
                {
                    assert(false);
                }
            }
        }
    }

    assert(out_animations.empty());
    out_animations = {};

    out_animations.reserve(1U);

    out_animations.emplace_back(std::move(weight_channel_names), std::move(weights), std::move(rigid_transform_channel_names), std::move(rigid_transforms), std::move(switch_channel_names), std::move(switches));

    return true;
}

static inline float internal_cubic_bezier(uint8_t const in_packed_k_1_x, uint8_t const in_packed_k_1_y, uint8_t const in_packed_k_2_x, uint8_t const in_packed_k_2_y, float const in_x)
{
    // https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function/cubic-bezier

    assert(in_packed_k_1_x <= static_cast<uint8_t>(INT8_MAX));
    assert(in_packed_k_1_y <= static_cast<uint8_t>(INT8_MAX));
    assert(in_packed_k_2_x <= static_cast<uint8_t>(INT8_MAX));
    assert(in_packed_k_2_y <= static_cast<uint8_t>(INT8_MAX));
    assert((in_x >= 0.0F) && (in_x <= 1.0F));

    // [_FnBezier.__find_roots](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/vmd/importer.py#L198)

    float out_y;
    if ((in_packed_k_1_x == in_packed_k_1_y) && (in_packed_k_2_x == in_packed_k_2_y))
    {
        out_y = in_x;
    }
    else
    {
        float const k_1_x = static_cast<float>(static_cast<double>(in_packed_k_1_x) / static_cast<double>(INT8_MAX));
        float const k_1_y = static_cast<float>(static_cast<double>(in_packed_k_1_y) / static_cast<double>(INT8_MAX));

        float const k_2_x = static_cast<float>(static_cast<double>(in_packed_k_2_x) / static_cast<double>(INT8_MAX));
        float const k_2_y = static_cast<float>(static_cast<double>(in_packed_k_2_y) / static_cast<double>(INT8_MAX));

        // https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function/cubic-bezier
        // [_FnBezier.__find_roots](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/vmd/importer.py#L198)

        float t;
        {
            // 0 * (1 - t)^3 + 3 * k_1 * (1 - t)^2 * t + 3 * k_2 * (1 - t) * t^2 + 1 * t^3 - x = 0
            // (3 * k_1 - 3 * k_2 + 1) * t^3 + (3 * k_2 - 6 * k_1) * t^2 + (3 * k_1) * t - x = 0

            float const a = k_1_x * 3.0F - k_2_x * 3.0F + 1.0F;
            float const b = k_2_x * 3.0F - k_1_x * 6.0F;
            float const c = k_1_x * 3.0F;
            float const d = 0.0F - in_x;

            // solve cubic equation: a*t^3 + b*t^2 + c*t + d = 0

            constexpr float const internal_epsilon = 1E-6F;
            constexpr float const internal_verification_epsilon = 2E-3F;

            if (std::abs(a) > internal_epsilon)
            {
                // Cubic Equation

                float const p = (c / a) - ((b / a) * (b / a) * (1.0F / 3.0F));
                float const q = ((b / a) * (b / a) * (b / a) * (2.0F / 27.0F)) - ((b / a) * (c / a) * (1.0F / 3.0F)) + (d / a);

                float const delta = q * q * (1.0F / 4.0F) + (p * p * p) * (1.0F / 27.0F);

                if (delta > internal_epsilon)
                {
                    float const sqrt_delta = std::sqrt(delta);
                    float const u = std::cbrt(((0.0F - q) * (1.0F / 2.0F)) + sqrt_delta);
                    float const v = std::cbrt(((0.0F - q) * (1.0F / 2.0F)) - sqrt_delta);
                    float const t1 = (u + v) - ((b / a) * (1.0F / 3.0F));
                    assert(std::abs(a * t1 * t1 * t1 + b * t1 * t1 + c * t1 + d) < internal_verification_epsilon);

                    if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t1), 1.0F);
                    }
                    else
                    {
                        assert(false);
                        t = in_x;
                    }
                }
                else if (delta > -internal_epsilon)
                {
                    float const u = std::cbrt((0.0F - q) * (1.0F / 2.0F));
                    float const t1 = (u * 2.0F) - ((b / a) * (1.0F / 3.0F));
                    float const t2 = (0.0F - u) - ((b / a) * (1.0F / 3.0F));
                    assert(std::abs(a * t1 * t1 * t1 + b * t1 * t1 + c * t1 + d) < internal_verification_epsilon);
                    assert(std::abs(a * t2 * t2 * t2 + b * t2 * t2 + c * t2 + d) < internal_verification_epsilon);

                    if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)) && (t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)))
                    {
                        if (std::abs(t1 - in_x) < std::abs(t2 - in_x))
                        {
                            t = std::min(std::max(0.0F, t1), 1.0F);
                        }
                        else
                        {
                            t = std::min(std::max(0.0F, t2), 1.0F);
                        }
                    }
                    else if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t1), 1.0F);
                    }
                    else if ((t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t1), 1.0F);
                    }
                    else
                    {
                        assert(false);
                        t = in_x;
                    }
                }
                else
                {

                    assert(delta <= -internal_epsilon);
                    assert(p <= -internal_epsilon);

                    constexpr double const internal_pi = 3.14159265358979323846264338327950288;

                    double const r = 2.0 * std::sqrt((0.0 - static_cast<double>(p)) * (1.0 / 3.0));
                    double const phi = std::acos(((0.0 - static_cast<double>(q)) * 3.0) / (r * (0.0 - static_cast<double>(p))));
                    float const t1 = static_cast<float>((r * std::cos(phi * (1.0 / 3.0))) - ((static_cast<double>(b) / static_cast<double>(a)) * (1.0 / 3.0)));
                    float const t2 = static_cast<float>((r * std::cos((phi + 2.0 * internal_pi) * (1.0 / 3.0))) - ((static_cast<double>(b) / static_cast<double>(a)) * (1.0 / 3.0)));
                    float const t3 = static_cast<float>((r * std::cos((phi + 2.0 * internal_pi * 2.0) * (1.0 / 3.0))) - ((static_cast<double>(b) / static_cast<double>(a)) * (1.0 / 3.0)));
                    assert(std::abs(a * t1 * t1 * t1 + b * t1 * t1 + c * t1 + d) < internal_verification_epsilon);
                    assert(std::abs(a * t2 * t2 * t2 + b * t2 * t2 + c * t2 + d) < internal_verification_epsilon);
                    assert(std::abs(a * t3 * t3 * t3 + b * t3 * t3 + c * t3 + d) < internal_verification_epsilon);

                    if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)) && (t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)) && (t3 >= (0.0F - internal_epsilon)) && (t3 <= (1.0F + internal_epsilon)))
                    {
                        if (std::abs(t1 - in_x) < std::abs(t2 - in_x) && std::abs(t1 - in_x) < std::abs(t3 - in_x))
                        {
                            t = std::min(std::max(0.0F, t1), 1.0F);
                        }
                        else if (std::abs(t2 - in_x) < std::abs(t1 - in_x) && std::abs(t2 - in_x) < std::abs(t3 - in_x))
                        {
                            t = std::min(std::max(0.0F, t2), 1.0F);
                        }
                        else
                        {
                            assert(std::abs(t3 - in_x) <= std::abs(t1 - in_x) && std::abs(t3 - in_x) <= std::abs(t2 - in_x));
                            t = std::min(std::max(0.0F, t3), 1.0F);
                        }
                    }
                    else if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)) && (t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)))
                    {
                        if (std::abs(t1 - in_x) < std::abs(t2 - in_x))
                        {
                            t = std::min(std::max(0.0F, t1), 1.0F);
                        }
                        else
                        {
                            assert(std::abs(t2 - in_x) <= std::abs(t1 - in_x));
                            t = std::min(std::max(0.0F, t2), 1.0F);
                        }
                    }
                    else if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)) && (t3 >= (0.0F - internal_epsilon)) && (t3 <= (1.0F + internal_epsilon)))
                    {
                        if (std::abs(t1 - in_x) < std::abs(t3 - in_x))
                        {
                            t = std::min(std::max(0.0F, t1), 1.0F);
                        }
                        else
                        {
                            assert(std::abs(t3 - in_x) <= std::abs(t1 - in_x));
                            t = std::min(std::max(0.0F, t3), 1.0F);
                        }
                    }
                    else if ((t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)) && (t3 >= (0.0F - internal_epsilon)) && (t3 <= (1.0F + internal_epsilon)))
                    {
                        if (std::abs(t2 - in_x) < std::abs(t3 - in_x))
                        {
                            t = std::min(std::max(0.0F, t2), 1.0F);
                        }
                        else
                        {
                            assert(std::abs(t3 - in_x) <= std::abs(t2 - in_x));
                            t = std::min(std::max(0.0F, t3), 1.0F);
                        }
                    }
                    else if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t1), 1.0F);
                    }
                    else if ((t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t2), 1.0F);
                    }
                    else if ((t3 >= (0.0F - internal_epsilon)) && (t3 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t3), 1.0F);
                    }
                    else
                    {
                        assert(false);
                        t = in_x;
                    }
                }
            }
            else if (std::abs(b) > internal_epsilon)
            {
                // Quadratic Equation

                float const delta = c * c - b * d * 4.0F;
                if (delta > internal_epsilon)
                {
                    float const sqrt_delta = std::sqrt(delta);
                    float const t1 = ((0.0F - c) + sqrt_delta) / (b * 2.0F);
                    float const t2 = ((0.0F - c) - sqrt_delta) / (b * 2.0F);
                    assert(std::abs(a * t1 * t1 * t1 + b * t1 * t1 + c * t1 + d) < internal_verification_epsilon);
                    assert(std::abs(a * t2 * t2 * t2 + b * t2 * t2 + c * t2 + d) < internal_verification_epsilon);

                    if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)) && (t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)))
                    {
                        if (std::abs(t1 - in_x) < std::abs(t2 - in_x))
                        {
                            t = std::min(std::max(0.0F, t1), 1.0F);
                        }
                        else
                        {
                            assert(std::abs(t2 - in_x) <= std::abs(t1 - in_x));
                            t = std::min(std::max(0.0F, t2), 1.0F);
                        }
                    }
                    else if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t1), 1.0F);
                    }
                    else if ((t2 >= (0.0F - internal_epsilon)) && (t2 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t2), 1.0F);
                    }
                    else
                    {
                        assert(false);
                        t = in_x;
                    }
                }
                else
                {
                    assert(false);
                    t = in_x;
                }
            }
            else
            {
                if (std::abs(c) > internal_epsilon)
                {
                    // Linear Equation

                    float const t1 = (0.0F - d) / c;
                    assert(std::abs(a * t1 * t1 * t1 + b * t1 * t1 + c * t1 + d) < internal_verification_epsilon);

                    if ((t1 >= (0.0F - internal_epsilon)) && (t1 <= (1.0F + internal_epsilon)))
                    {
                        t = std::min(std::max(0.0F, t1), 1.0F);
                    }
                    else
                    {
                        assert(false);
                        t = in_x;
                    }
                }
                else
                {
                    assert(false);
                    t = in_x;
                }
            }
        }

        out_y = (k_1_y * 3.0F - k_2_y * 3.0F + 1.0F) * t * t * t + (k_2_y * 3.0F - k_1_y * 6.0F) * t * t + (k_1_y * 3.0F) * t;
    }

    return out_y;
}

// [ImportVmd.scale](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/operators/fileio.py#L231)
constexpr float const INTERNAL_IMPORT_VMD_SCALE = 0.1F;

// MMD
// LH
// Up +Y
// Forward -Z
// Right -X

// glTF
// RH
// Up +Y
// Forward +Z
// Right -X

static inline mmd_vmd_vec3_t internal_transform_translation(mmd_vmd_vec3_t const &v)
{
    return mmd_vmd_vec3_t{v.m_x * INTERNAL_IMPORT_VMD_SCALE, v.m_y * INTERNAL_IMPORT_VMD_SCALE, -v.m_z * INTERNAL_IMPORT_VMD_SCALE};
}

static inline mmd_vmd_vec4_t internal_transform_rotation(mmd_vmd_vec4_t const &raw_q)
{
    DirectX::XMFLOAT4 out_q;
    {
        DirectX::XMFLOAT4 const in_q(raw_q.m_x, raw_q.m_y, raw_q.m_z, raw_q.m_w);

        DirectX::XMMATRIX r = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&in_q));

        DirectX::XMMATRIX z = DirectX::XMMatrixScaling(1.0F, 1.0F, -1.0F);

        DirectX::XMStoreFloat4(&out_q, DirectX::XMQuaternionNormalize(DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(z, r), z))));
    }

    return mmd_vmd_vec4_t{out_q.x, out_q.y, out_q.z, out_q.w};
}
