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
#include "../../Brioche-Analytic-Rendering-Interface/include/brx_anari_format.h"
#include "../../Brioche-Motion/include/brx_motion_format.h"
#include <cstddef>
#include <cstdint>

// [Khronos ANARI (Analytic Rendering Interface): glTF To ANARI](https://github.com/KhronosGroup/ANARI-SDK/blob/next_release/src/anari_test_scenes/scenes/file/gltf2anari.h)

class brx_asset_import_scene;
class brx_asset_import_surface_group;
class brx_asset_import_surface;
class brx_asset_import_morph_animation;
class brx_asset_import_skeleton_animation;

struct brx_asset_import_geometry_vertex_position
{
    // R32G32B32_FLOAT
    float m_position[3];
};

struct brx_asset_import_geometry_vertex_varying
{
    // R16G16_SNORM (octahedron map)
    uint32_t m_normal;
    // R15G15B2_SNORM (octahedron map + tangent w)
    uint32_t m_tangent;
    // R16G16_UNORM
    uint32_t m_texcoord;
};

struct brx_asset_import_geometry_vertex_joint
{
    // R16G16B16A16_UINT (xy)
    uint32_t m_indices_xy;
    // R16G16B16A16_UINT (wz)
    uint32_t m_indices_wz;
    // R8G8B8A8_UNORM
    uint32_t m_weights;
};

class brx_asset_import_scene
{
public:
    virtual uint32_t get_surface_group_count() const = 0;
    virtual brx_asset_import_surface_group const *get_surface_group(uint32_t group_index) const = 0;
    virtual uint32_t get_skeleton_animation_count() const = 0;
    virtual brx_asset_import_skeleton_animation *get_skeleton_animation(uint32_t skeleton_animation_index) const = 0;
};

class brx_asset_import_surface_group
{
public:
    virtual uint32_t get_surface_count() const = 0;
    virtual brx_asset_import_surface const *get_surface(uint32_t surface_index) const = 0;

    // NULL: no skeleton
    // not NULL: skin
    virtual uint32_t const get_skeleton_joint_count() const = 0;
    virtual char const *get_skeleton_joint_name(uint32_t skeleton_joint_index) const = 0;
    virtual uint32_t get_skeleton_joint_parent_index(uint32_t skeleton_joint_index) const = 0;
    virtual brx_motion_rigid_transform const *get_skeleton_joint_bind_pose_transform(uint32_t skeleton_joint_index) const = 0;
    virtual uint32_t get_vrm_skeleton_joint_index(BRX_MOTION_VRM_SKELETON_JOINT_NAME vrm_skeleton_joint_name) const = 0;
};

class brx_asset_import_surface
{
    // geometry
    // less than 1: invalid
    // 1: no morph animation
    // greater than 1: morph animation
    virtual uint32_t get_morph_target_count() const = 0;
    virtual brx_asset_import_geometry_vertex_position const *get_morph_target_vertex_positions(uint32_t morph_target_index) const = 0;
    virtual brx_asset_import_geometry_vertex_varying const *get_morph_target_vertex_varyings(uint32_t morph_target_index) const = 0;
    virtual uint32_t get_vrm_morph_target_name_index(BRX_MOTION_VRM_MORPH_TARGET_NAME vrm_morph_target_name) const = 0;

    // NULL: no skin (even if there is one skeleton bound to the group)
    // not NULL: skin (there must be one skeleton bound to the group)
    virtual brx_asset_import_geometry_vertex_joint const *get_vertex_joints() const = 0;

    // material
    virtual uint32_t get_texture_count() const = 0;
    // start with file:// : external file
    // start with data:// : internal data
    virtual char const *get_texture_url(uint32_t texture_index) const = 0;
    virtual uint32_t get_pbr_texture_index(BRX_ANARI_PBR_TEXTURE_NAME pbr_texture_name) const = 0;
};

class brx_asset_import_morph_animation
{

};

class brx_asset_import_skeleton_animation
{
    virtual uint32_t const get_skeleton_joint_count() const = 0;
    virtual char const *get_skeleton_joint_name(uint32_t skeleton_joint_index) const = 0;
    virtual uint32_t const get_frame_count() const = 0;
    virtual brx_motion_rigid_transform const *get_animation_pose_skeleton_joint_transforms(uint32_t frame_index) const = 0;
};

extern "C" brx_asset_import_scene *brx_asset_import_create_scene(brx_asset_import_input_stream_factory *input_stream_factory, char const *file_name);

#endif
