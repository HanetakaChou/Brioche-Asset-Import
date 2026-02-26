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

#include "internal_mmd_pmx.h"
#include "../include/brx_asset_import_scene.h"
#include "../../libiconv/include/iconv.h"
#include <cstring>
#include <cassert>
#include <bit>

// [PMX (Polygon Model eXtended) 2.1](https://gist.github.com/felixjones/f8a06bd48f9da9a4539f)
// [Blender MMD Tools](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py)

#if defined(__GNUC__)
// GCC or CLANG
#define internal_unlikely(x) __builtin_expect(!!(x), 0)
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __ORDER_LITTLE_ENDIAN__ == __BYTE_ORDER__
static inline uint16_t internal_le_to_h_16(uint16_t x) { return x; }
static inline uint32_t internal_le_to_h_32(uint32_t x) { return x; }
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __ORDER_BIG_ENDIAN__ == __BYTE_ORDER__
static inline uint16_t internal_le_to_h_16(uint16_t x) { return __builtin_bswap16(x); }
static inline uint32_t internal_le_to_h_32(uint32_t x) { return __builtin_bswap32(x); }
#else
#error Unknown Byte Order
#endif
#elif defined(_MSC_VER)
#if defined(__clang__)
// CLANG-CL
#define internal_unlikely(x) __builtin_expect(!!(x), 0)
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __ORDER_LITTLE_ENDIAN__ == __BYTE_ORDER__
static inline uint16_t internal_le_to_h_16(uint16_t x) { return x; }
static inline uint32_t internal_le_to_h_32(uint32_t x) { return x; }
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __ORDER_BIG_ENDIAN__ == __BYTE_ORDER__
static inline uint16_t internal_le_to_h_16(uint16_t x) { return __builtin_bswap16(x); }
static inline uint32_t internal_le_to_h_32(uint32_t x) { return __builtin_bswap32(x); }
#else
#error Unknown Byte Order
#endif
#else
// MSVC
#define internal_unlikely(x) (!!(x))
// Assume Little Endian
static inline uint16_t internal_le_to_h_16(uint16_t x) { return x; }
static inline uint32_t internal_le_to_h_32(uint32_t x) { return x; }
#endif
#else
#error Unknown Compiler
#endif

static inline bool internal_data_read_mmd_pmx_header(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t *out_text_encoding, uint8_t *out_additional_vec4_count, uint8_t *out_vertex_index_size, uint8_t *out_texture_index_size, uint8_t *out_material_index_size, uint8_t *out_bone_index_size, uint8_t *out_morph_index_size, uint8_t *out_rigid_body_index_size, mmd_pmx_header_t *out_header);

static inline bool internal_data_read_mmd_pmx_vertices(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t additional_vec4_count, uint8_t bone_index_size, mcrt_vector<mmd_pmx_vertex_t> &out_vertices);

static inline bool internal_data_read_mmd_pmx_faces(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t vertex_index_size, mcrt_vector<mmd_pmx_face_t> &out_faces);

static inline bool internal_data_read_mmd_pmx_textures(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, mcrt_vector<mmd_pmx_texture_t> &out_textures);

static inline bool internal_data_read_mmd_pmx_materials(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint8_t texture_index_size, mcrt_vector<mmd_pmx_material_t> &out_materials);

static inline bool internal_data_read_mmd_pmx_bones(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint8_t bone_index_size, mcrt_vector<mmd_pmx_bone_t> &out_bones);

static inline bool internal_data_read_mmd_pmx_morphs(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint8_t vertex_index_size, uint32_t material_index_size, uint32_t bone_index_size, uint8_t morph_index_size, uint8_t rigid_body_index_size, mcrt_vector<mmd_pmx_morph_t> &out_morphs);

static inline bool internal_data_read_mmd_pmx_display_frames(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint32_t bone_index_size, uint8_t morph_index_size);

static inline bool internal_data_read_mmd_pmx_rigid_bodies(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint32_t bone_index_size, mcrt_vector<mmd_pmx_rigid_body_t> &out_rigid_bodies);

static inline bool internal_data_read_mmd_pmx_constraints(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint32_t rigid_body_index_size, mcrt_vector<mmd_pmx_constraint_t> &out_joints);

static inline bool internal_data_read_mmd_pmx_vec2(void const *data_base, size_t data_size, size_t &inout_data_offset, mmd_pmx_vec2_t *out_vec3);

static inline bool internal_data_read_mmd_pmx_vec3(void const *data_base, size_t data_size, size_t &inout_data_offset, mmd_pmx_vec3_t *out_vec3);

static inline bool internal_data_read_mmd_pmx_vec4(void const *data_base, size_t data_size, size_t &inout_data_offset, mmd_pmx_vec4_t *out_vec3);

static inline bool internal_data_read_mmd_pmx_text(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, mcrt_string &out_text);

static inline bool internal_data_read_mmd_pmx_signed_index(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t index_size, uint32_t *out_index);

static inline bool internal_data_read_mmd_pmx_unsigned_index(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t index_size, uint32_t *out_index);

static inline bool internal_data_read_uint8(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t *out_uint8);

static inline bool internal_data_read_uint16(void const *data_base, size_t data_size, size_t &inout_data_offset, uint16_t *out_uint16);

static inline bool internal_data_read_uint32(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t *out_uint32);

static inline bool internal_data_read_float(void const *data_base, size_t data_size, size_t &inout_data_offset, float *out_float);

static inline bool internal_data_read_bytes(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t length, void *out_bytes);

extern bool internal_data_read_mmd_pmx(void const *data_base, size_t data_size, mmd_pmx_t *out_mmd_pmx)
{
    size_t data_offset = 0U;

    uint8_t text_encoding;
    uint8_t additional_vec4_count;
    uint8_t vertex_index_size;
    uint8_t texture_index_size;
    uint8_t material_index_size;
    uint8_t bone_index_size;
    uint8_t morph_index_size;
    uint8_t rigid_body_index_size;

    if (internal_unlikely(!internal_data_read_mmd_pmx_header(data_base, data_size, data_offset, &text_encoding, &additional_vec4_count, &vertex_index_size, &texture_index_size, &material_index_size, &bone_index_size, &morph_index_size, &rigid_body_index_size, &out_mmd_pmx->m_header)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_vertices(data_base, data_size, data_offset, additional_vec4_count, bone_index_size, out_mmd_pmx->m_vertices)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_faces(data_base, data_size, data_offset, vertex_index_size, out_mmd_pmx->m_faces)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_textures(data_base, data_size, data_offset, text_encoding, out_mmd_pmx->m_textures)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_materials(data_base, data_size, data_offset, text_encoding, texture_index_size, out_mmd_pmx->m_materials)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_bones(data_base, data_size, data_offset, text_encoding, bone_index_size, out_mmd_pmx->m_bones)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_morphs(data_base, data_size, data_offset, text_encoding, vertex_index_size, material_index_size, bone_index_size, morph_index_size, rigid_body_index_size, out_mmd_pmx->m_morphs)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_display_frames(data_base, data_size, data_offset, text_encoding, bone_index_size, morph_index_size)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_rigid_bodies(data_base, data_size, data_offset, text_encoding, bone_index_size, out_mmd_pmx->m_rigid_bodies)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_constraints(data_base, data_size, data_offset, text_encoding, rigid_body_index_size, out_mmd_pmx->m_constraints)))
    {
        return false;
    }

    // TODO:
    // if (data_offset < data_size)
    // {
    //     read soft body
    // }
    // else
    // {
    //     assert(data_size == data_offset);
    // }

    assert(data_size == data_offset);

    return true;
}

static inline bool internal_data_read_mmd_pmx_header(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t *out_text_encoding, uint8_t *out_additional_vec4_count, uint8_t *out_vertex_index_size, uint8_t *out_texture_index_size, uint8_t *out_material_index_size, uint8_t *out_bone_index_size, uint8_t *out_morph_index_size, uint8_t *out_rigid_body_index_size, mmd_pmx_header_t *out_header)
{
    // [Header.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L255)

    uint8_t signature[4];
    if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, sizeof(uint8_t) * 4U, &signature[0])))
    {
        return false;
    }

    // "PMX "
    if (internal_unlikely(!((80U == signature[0]) && (77U == signature[1]) && (88U == signature[2]) && (32U == signature[3]))))
    {
        return false;
    }

    uint8_t version[4];
    if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, sizeof(uint8_t) * 4U, &version[0])))
    {
        return false;
    }

    //   0   0 0 64: float 2.0
    // 102 102 6 64: float 2.1
    if (internal_unlikely(!(((0U == version[0]) && (0U == version[1]) && (0U == version[2]) && (64U == version[3])) || ((102U == version[0]) && (102U == version[1]) && (6U == version[2]) && (64U == version[3])))))
    {
        return false;
    }

    uint8_t global_count;
    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &global_count)))
    {
        return false;
    }

    if (internal_unlikely(!(global_count >= 8U)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_text_encoding)))
    {
        return false;
    }

    if (internal_unlikely(!((0U == (*out_text_encoding)) || (1U == (*out_text_encoding)))))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_additional_vec4_count)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_vertex_index_size)))
    {
        return false;
    }

    if (internal_unlikely(!((1U == (*out_vertex_index_size)) || (2U == (*out_vertex_index_size)) || (4U == (*out_vertex_index_size)))))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_texture_index_size)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_material_index_size)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_bone_index_size)))
    {
        return false;
    }

    if (internal_unlikely(!((1U == (*out_bone_index_size)) || (2U == (*out_bone_index_size)) || (4U == (*out_bone_index_size)))))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_morph_index_size)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, out_rigid_body_index_size)))
    {
        return false;
    }

    for (uint8_t global_index = 8U; global_index < global_count; ++global_index)
    {
        // Tolerance
        uint8_t unused_global;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_global)))
        {
            return false;
        }
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, (*out_text_encoding), out_header->m_name)))
    {
        return false;
    }

    mcrt_string unused_name_e;
    if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, (*out_text_encoding), unused_name_e)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, (*out_text_encoding), out_header->m_comment)))
    {
        return false;
    }

    mcrt_string unused_comment_e;
    if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, (*out_text_encoding), unused_comment_e)))
    {
        return false;
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_vertices(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t additional_vec4_count, uint8_t bone_index_size, mcrt_vector<mmd_pmx_vertex_t> &out_vertices)
{
    // [Vertex.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L665)

    assert(out_vertices.empty());
    out_vertices = {};

    uint32_t vertex_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &vertex_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == vertex_count || (vertex_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_vertices.resize(vertex_count);

    for (uint32_t vertex_index = 0U; vertex_index < vertex_count; ++vertex_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_position)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_normal)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec2(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_uv)))
        {
            return false;
        }

        for (uint8_t additional_vec4_index = 0U; additional_vec4_index < additional_vec4_count; ++additional_vec4_index)
        {
            mmd_pmx_vec4_t unused_additional_vec4;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_additional_vec4)))
            {
                return false;
            }
        }

        uint8_t bone_weight_type;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &bone_weight_type)))
        {
            return false;
        }

        if (0U == bone_weight_type)
        {
            // BDEF1
            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[0])))
            {
                return false;
            }

            out_vertices[vertex_index].m_bone_indices[1] = out_vertices[vertex_index].m_bone_indices[0];
            out_vertices[vertex_index].m_bone_indices[2] = out_vertices[vertex_index].m_bone_indices[0];
            out_vertices[vertex_index].m_bone_indices[3] = out_vertices[vertex_index].m_bone_indices[0];
            out_vertices[vertex_index].m_bone_weights[0] = 1.0F;
            out_vertices[vertex_index].m_bone_weights[1] = 0.0F;
            out_vertices[vertex_index].m_bone_weights[2] = 0.0F;
            out_vertices[vertex_index].m_bone_weights[3] = 0.0F;
        }
        else if (1U == bone_weight_type)
        {
            // BDEF2

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[0])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[1])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_bone_weights[0])))
            {
                return false;
            }

            out_vertices[vertex_index].m_bone_indices[2] = out_vertices[vertex_index].m_bone_indices[0];
            out_vertices[vertex_index].m_bone_indices[3] = out_vertices[vertex_index].m_bone_indices[1];
            out_vertices[vertex_index].m_bone_weights[1] = 1.0F - out_vertices[vertex_index].m_bone_weights[0];
            out_vertices[vertex_index].m_bone_weights[2] = 0.0F;
            out_vertices[vertex_index].m_bone_weights[3] = 0.0F;
        }
        else if ((2U == bone_weight_type) || (4U == bone_weight_type))
        {
            // BDEF4
            // QDEF

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[0])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[1])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[2])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[3])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_bone_weights[0])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_bone_weights[1])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_bone_weights[2])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_bone_weights[3])))
            {
                return false;
            }
        }
        else if (3U == bone_weight_type)
        {
            // assert(false);

            // SDEF

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[0])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_vertices[vertex_index].m_bone_indices[1])))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vertices[vertex_index].m_bone_weights[0])))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_C;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_C)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_R0;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_R0)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_R1;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_R1)))
            {
                return false;
            }

            out_vertices[vertex_index].m_bone_indices[2] = out_vertices[vertex_index].m_bone_indices[0];
            out_vertices[vertex_index].m_bone_indices[3] = out_vertices[vertex_index].m_bone_indices[1];
            out_vertices[vertex_index].m_bone_weights[1] = 1.0F - out_vertices[vertex_index].m_bone_weights[0];
            out_vertices[vertex_index].m_bone_weights[2] = 0.0F;
            out_vertices[vertex_index].m_bone_weights[3] = 0.0F;
        }
        else
        {
            return false;
        }

        float unused_edge_scale;
        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_edge_scale)))
        {
            return false;
        }
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_faces(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t vertex_index_size, mcrt_vector<mmd_pmx_face_t> &out_faces)
{
    assert(out_faces.empty());
    out_faces = {};

    uint32_t vertex_index_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &vertex_index_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == vertex_index_count || (vertex_index_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    // Tolerance
    assert(0U == (vertex_index_count % 3U));

    uint32_t const face_count = vertex_index_count / 3U;

    out_faces.resize(face_count);

    uint32_t vertex_index_index = 0;
    for (uint32_t face_index = 0U; face_index < face_count; ++face_index)
    {
        assert(vertex_index_index < vertex_index_count);

        if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &out_faces[face_index].m_vertex_indices[0])))
        {
            return false;
        }
        ++vertex_index_index;

        if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &out_faces[face_index].m_vertex_indices[1])))
        {
            return false;
        }
        ++vertex_index_index;

        if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &out_faces[face_index].m_vertex_indices[2])))
        {
            return false;
        }
        ++vertex_index_index;
    }

    assert(vertex_index_count == vertex_index_index);

    // Tolerance
    while (vertex_index_index < vertex_index_count)
    {
        uint32_t unused_vertex_index;
        if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &unused_vertex_index)))
        {
            return false;
        }
        ++vertex_index_index;
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_textures(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, mcrt_vector<mmd_pmx_texture_t> &out_textures)
{
    // [Texture.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L787)

    assert(out_textures.empty());
    out_textures = {};

    uint32_t texture_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &texture_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == texture_count || (texture_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_textures.resize(texture_count);

    for (uint32_t texture_index = 0U; texture_index < texture_count; ++texture_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, out_textures[texture_index].m_path)))
        {
            return false;
        }
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_materials(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint8_t texture_index_size, mcrt_vector<mmd_pmx_material_t> &out_materials)
{
    // [Material.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L860)

    assert(out_materials.empty());
    out_materials = {};

    uint32_t material_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &material_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == material_count || (material_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_materials.resize(material_count);

    for (uint32_t material_index = 0U; material_index < material_count; ++material_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, out_materials[material_index].m_name)))
        {
            return false;
        }

        mcrt_string unused_name_e;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name_e)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &out_materials[material_index].m_diffuse)))
        {
            return false;
        }

        mmd_pmx_vec3_t unused_specular;
        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_specular)))
        {
            return false;
        }

        float unused_shininess;
        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_shininess)))
        {
            return false;
        }

        mmd_pmx_vec3_t unused_ambient;
        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_ambient)))
        {
            return false;
        }

        uint8_t drawing_flags;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &drawing_flags)))
        {
            return false;
        }

        out_materials[material_index].m_is_double_sided = (0U != (drawing_flags & 1U));

        mmd_pmx_vec4_t unused_edge_color;
        if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_edge_color)))
        {
            return false;
        }

        float unused_edge_size;
        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_edge_size)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, texture_index_size, &out_materials[material_index].m_texture_index)))
        {
            return false;
        }

        uint32_t unused_sphere_texture;
        if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, texture_index_size, &unused_sphere_texture)))
        {
            return false;
        }

        uint8_t unused_sphere_texture_mode;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_sphere_texture_mode)))
        {
            return false;
        }

        uint8_t is_shared_toon_texture;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &is_shared_toon_texture)))
        {
            return false;
        }

        if (0 == is_shared_toon_texture)
        {
            uint32_t unused_toon_texture;
            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, texture_index_size, &unused_toon_texture)))
            {
                return false;
            }
        }
        else if (1 == is_shared_toon_texture)
        {
            uint8_t unused_toon_texture;
            if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_toon_texture)))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        mcrt_string unused_meta_data;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_meta_data)))
        {
            return false;
        }

        uint32_t vertex_count;
        if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &vertex_count)))
        {
            return false;
        }

        // Tolerance
        assert(0U == (vertex_count % 3U));

        out_materials[material_index].m_face_count = ((vertex_count <= static_cast<uint32_t>(INT32_MAX)) ? (vertex_count / 3U) : 0U);
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_bones(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint8_t bone_index_size, mcrt_vector<mmd_pmx_bone_t> &out_bones)
{
    // [Bone.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L986)
    // [PMXImporter.__applyIk](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/importer.py#L315)

    // https://github.com/Nuthouse01/PMX-VMD-Scripting-Tools/blob/master/mmd_scripting/overall_cleanup/bonedeform_fix.py

    assert(out_bones.empty());
    out_bones = {};

    uint32_t bone_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &bone_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == bone_count || (bone_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_bones.resize(bone_count);

    for (uint32_t bone_index = 0U; bone_index < bone_count; ++bone_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, out_bones[bone_index].m_name)))
        {
            return false;
        }

        mcrt_string unused_name_e;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name_e)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_bones[bone_index].m_translation)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_bones[bone_index].m_parent_index)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &out_bones[bone_index].m_transformation_hierarchy)))
        {
            return false;
        }

        uint16_t bone_flags;
        if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &bone_flags)))
        {
            return false;
        }

        out_bones[bone_index].m_meta_physics = (0U != (bone_flags & 0X1000U));

        if ((0U != (bone_flags & 0X1U)))
        {
            uint32_t unused_display_destination;
            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &unused_display_destination)))
            {
                return false;
            }
        }
        else
        {
            mmd_pmx_vec3_t unused_display_destination;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_display_destination)))
            {
                return false;
            }
        }

        out_bones[bone_index].m_append_rotation = ((0U != (bone_flags & 0X0100U)));
        out_bones[bone_index].m_append_translation = ((0U != (bone_flags & 0X0200U)));
        out_bones[bone_index].m_append_local = ((0U != (bone_flags & 0X0080U)));

        if (out_bones[bone_index].m_append_rotation || out_bones[bone_index].m_append_translation)
        {
            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_bones[bone_index].m_append_parent_index)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_bones[bone_index].m_append_rate)))
            {
                return false;
            }
        }

        if ((0U != (bone_flags & 0X400U)))
        {
            mmd_pmx_vec3_t unused_axis_limit;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_axis_limit)))
            {
                return false;
            }
        }

        if ((0U != (bone_flags & 0X800U)))
        {
            mmd_pmx_vec3_t unused_local_axis_x;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_local_axis_x)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_local_axis_z;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_local_axis_z)))
            {
                return false;
            }
        }

        if ((0U != (bone_flags & 0X2000U)))
        {
            uint32_t unused_external_parent_key;
            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &unused_external_parent_key)))
            {
                return false;
            }
        }

        out_bones[bone_index].m_ik = (0U != (bone_flags & 0X20U));

        if (out_bones[bone_index].m_ik)
        {
            if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_bones[bone_index].m_ik_end_effector_index)))
            {
                return false;
            }

            uint32_t unused_loop;
            if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &unused_loop)))
            {
                return false;
            }

            float unused_unit_angle;
            if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_unit_angle)))
            {
                return false;
            }

            uint32_t ik_joint_count;
            if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &ik_joint_count)))
            {
                return false;
            }

            if (internal_unlikely(0U == ik_joint_count || (ik_joint_count > static_cast<uint32_t>(INT32_MAX))))
            {
                // Tolerance
                assert(false);
            }
            else
            {
                out_bones[bone_index].m_ik_link_indices.resize(ik_joint_count);

                uint32_t current_joint_index_plus_1 = ik_joint_count;
                for (uint32_t ik_joint_index = 0U; ik_joint_index < ik_joint_count; ++ik_joint_index)
                {
                    uint32_t const current_joint_index = current_joint_index_plus_1 - 1U;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_bones[bone_index].m_ik_link_indices[current_joint_index])))
                    {
                        return false;
                    }

                    uint8_t angle_limit;
                    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &angle_limit)))
                    {
                        return false;
                    }

                    mmd_pmx_vec3_t limit_angle_min;
                    mmd_pmx_vec3_t limit_angle_max;
                    if ((0U != angle_limit))
                    {
                        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &limit_angle_min)))
                        {
                            return false;
                        }

                        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &limit_angle_max)))
                        {
                            return false;
                        }
                    }

                    constexpr uint32_t const hinge_joint_index = 1U;
                    if (2U == ik_joint_count && hinge_joint_index == current_joint_index)
                    {
                        out_bones[bone_index].m_ik_two_links_hinge_limit_angle = (0U != angle_limit);

                        if (out_bones[bone_index].m_ik_two_links_hinge_limit_angle)
                        {
                            out_bones[bone_index].m_ik_two_links_hinge_limit_angle_min = limit_angle_min;

                            out_bones[bone_index].m_ik_two_links_hinge_limit_angle_max = limit_angle_max;
                        }
                    }

                    --current_joint_index_plus_1;
                }
            }
        }
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_morphs(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint8_t vertex_index_size, uint32_t material_index_size, uint32_t bone_index_size, uint8_t morph_index_size, uint8_t rigid_body_index_size, mcrt_vector<mmd_pmx_morph_t> &out_morphs)
{
    // [Morph.create](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L1155)

    assert(out_morphs.empty());
    out_morphs = {};

    uint32_t morph_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &morph_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == morph_count || (morph_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_morphs.resize(morph_count);

    for (uint32_t morph_index = 0U; morph_index < morph_count; ++morph_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, out_morphs[morph_index].m_name)))
        {
            return false;
        }

        mcrt_string unused_name_e;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name_e)))
        {
            return false;
        }

        uint8_t unused_panel;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_panel)))
        {
            return false;
        }

        uint8_t morph_type;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &morph_type)))
        {
            return false;
        }

        uint32_t offset_count;
        if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &offset_count)))
        {
            return false;
        }

        if (internal_unlikely(0U == offset_count || (offset_count > static_cast<uint32_t>(INT32_MAX))))
        {
            // Tolerance
            // assert(false);
        }
        else
        {
            if (0U == morph_type)
            {
                // group morph
                out_morphs[morph_index].m_morph_type = 0U;

                out_morphs[morph_index].m_offsets.resize(offset_count);

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, morph_index_size, &out_morphs[morph_index].m_offsets[offset_index].m_group.m_morph_index)))
                    {
                        return false;
                    }

                    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_morphs[morph_index].m_offsets[offset_index].m_group.m_morph_weight)))
                    {
                        return false;
                    }
                }
            }
            else if (1U == morph_type)
            {
                // vertex position morph
                out_morphs[morph_index].m_morph_type = 1U;

                out_morphs[morph_index].m_offsets.resize(offset_count);

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &out_morphs[morph_index].m_offsets[offset_index].m_vertex_position.m_vertex_index)))
                    {
                        return false;
                    }

                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_morphs[morph_index].m_offsets[offset_index].m_vertex_position.m_vertex_position)))
                    {
                        return false;
                    }
                }
            }
            else if (2U == morph_type)
            {
                // bone morph
                out_morphs[morph_index].m_morph_type = 3U;

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    uint32_t unused_bone_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &unused_bone_index)))
                    {
                        return false;
                    }

                    mmd_pmx_vec3_t unused_bone_translation;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_bone_translation)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_bone_rotation;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_bone_rotation)))
                    {
                        return false;
                    }
                }
            }
            else if (3U == morph_type)
            {
                // vertex uv morph
                out_morphs[morph_index].m_morph_type = 2U;

                out_morphs[morph_index].m_offsets.resize(offset_count);

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &out_morphs[morph_index].m_offsets[offset_index].m_vertex_uv.m_vertex_index)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t uv;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &uv)))
                    {
                        return false;
                    }

                    out_morphs[morph_index].m_offsets[offset_index].m_vertex_uv.m_vertex_uv.m_x = uv.m_x;
                    out_morphs[morph_index].m_offsets[offset_index].m_vertex_uv.m_vertex_uv.m_y = uv.m_y;
                    assert(0.0F == uv.m_z);
                    assert(0.0F == uv.m_w);
                }
            }
            else if (4U == morph_type || 5U == morph_type || 6U == morph_type || 7U == morph_type)
            {
                // vertex additional vec4 morph
                out_morphs[morph_index].m_morph_type = 3U;

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    uint32_t unused_vertex_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_unsigned_index(data_base, data_size, inout_data_offset, vertex_index_size, &unused_vertex_index)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_additional_vec4;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_additional_vec4)))
                    {
                        return false;
                    }
                }
            }
            else if (8U == morph_type)
            {
                // material morph
                out_morphs[morph_index].m_morph_type = 3U;

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    uint32_t unused_material_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, material_index_size, &unused_material_index)))
                    {
                        return false;
                    }

                    uint8_t unused_type;
                    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_type)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_diffuse;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_diffuse)))
                    {
                        return false;
                    }

                    mmd_pmx_vec3_t unused_specular;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_specular)))
                    {
                        return false;
                    }

                    float unused_shininess;
                    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_shininess)))
                    {
                        return false;
                    }

                    mmd_pmx_vec3_t unused_ambient;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_ambient)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_edge_color;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_edge_color)))
                    {
                        return false;
                    }

                    float unused_edge_size;
                    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_edge_size)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_texture_factor;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_texture_factor)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_sphere_texture_factor;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_sphere_texture_factor)))
                    {
                        return false;
                    }

                    mmd_pmx_vec4_t unused_toon_texture_factor;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec4(data_base, data_size, inout_data_offset, &unused_toon_texture_factor)))
                    {
                        return false;
                    }
                }
            }
            else if (9U == morph_type)
            {
                // flip morph
                out_morphs[morph_index].m_morph_type = 3U;

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    uint32_t unused_morph_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, morph_index_size, &unused_morph_index)))
                    {
                        return false;
                    }

                    float unused_morph_weight;
                    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &unused_morph_weight)))
                    {
                        return false;
                    }
                }
            }
            else if (10U == morph_type)
            {
                // impulse morph
                out_morphs[morph_index].m_morph_type = 3U;

                for (uint32_t offset_index = 0U; offset_index < offset_count; ++offset_index)
                {
                    uint32_t unused_rigid_body_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, rigid_body_index_size, &unused_rigid_body_index)))
                    {
                        return false;
                    }

                    uint8_t unused_local_flag;
                    if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_local_flag)))
                    {
                        return false;
                    }

                    mmd_pmx_vec3_t unused_translation;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation)))
                    {
                        return false;
                    }

                    mmd_pmx_vec3_t unused_rotation;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_rotation)))
                    {
                        return false;
                    }
                }
            }
            else
            {
                assert(false);
                return false;
            }
        }
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_display_frames(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint32_t bone_index_size, uint8_t morph_index_size)
{
    // [Display.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L1382)

    uint32_t display_frame_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &display_frame_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == display_frame_count || (display_frame_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    for (uint32_t display_frame_index = 0U; display_frame_index < display_frame_count; ++display_frame_index)
    {
        mcrt_string unused_name;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name)))
        {
            return false;
        }

        mcrt_string unused_name_e;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name_e)))
        {
            return false;
        }

        uint8_t unused_flag;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &unused_flag)))
        {
            return false;
        }

        uint32_t element_count;
        if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &element_count)))
        {
            return false;
        }

        if (internal_unlikely(0U == element_count || (element_count > static_cast<uint32_t>(INT32_MAX))))
        {
            // Tolerance
            // assert(false);
        }
        else
        {
            for (uint32_t element_index = 0U; element_index < element_count; ++element_index)
            {

                uint8_t element_type;
                if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &element_type)))
                {
                    return false;
                }

                if (0U == element_type)
                {
                    uint32_t unused_bone_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &unused_bone_index)))
                    {
                        return false;
                    }
                }
                else if (1U == element_type)
                {
                    uint32_t unused_morph_index;
                    if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, morph_index_size, &unused_morph_index)))
                    {
                        return false;
                    }
                }
                else
                {
                    assert(false);
                    return false;
                }
            }
        }
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_rigid_bodies(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint32_t bone_index_size, mcrt_vector<mmd_pmx_rigid_body_t> &out_rigid_bodies)
{
    // [Rigid.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L1453)

    assert(out_rigid_bodies.empty());
    out_rigid_bodies = {};

    uint32_t rigid_body_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &rigid_body_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == rigid_body_count || (rigid_body_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_rigid_bodies.resize(rigid_body_count);

    for (uint32_t rigid_body_index = 0U; rigid_body_index < rigid_body_count; ++rigid_body_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, out_rigid_bodies[rigid_body_index].m_name)))
        {
            return false;
        }

        mcrt_string unused_name_e;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name_e)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, bone_index_size, &out_rigid_bodies[rigid_body_index].m_bone_index)))
        {
            return false;
        }

        uint8_t collision_filter_group_uint8;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &collision_filter_group_uint8)))
        {
            return false;
        }

        out_rigid_bodies[rigid_body_index].m_collision_filter_group = collision_filter_group_uint8;

        uint16_t collision_filter_mask_uint16;
        if (internal_unlikely(!internal_data_read_uint16(data_base, data_size, inout_data_offset, &collision_filter_mask_uint16)))
        {
            return false;
        }

        out_rigid_bodies[rigid_body_index].m_collision_filter_mask = collision_filter_mask_uint16;

        uint8_t shape_type_uint8;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &shape_type_uint8)))
        {
            return false;
        }

        out_rigid_bodies[rigid_body_index].m_shape_type = shape_type_uint8;

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_shape_size)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_translation)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_rotation)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_mass)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_linear_damping)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_angular_damping)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_restitution)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_rigid_bodies[rigid_body_index].m_friction)))
        {
            return false;
        }

        uint8_t rigid_body_type_uint8;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &rigid_body_type_uint8)))
        {
            return false;
        }

        out_rigid_bodies[rigid_body_index].m_rigid_body_type = rigid_body_type_uint8;
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_constraints(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, uint32_t rigid_body_index_size, mcrt_vector<mmd_pmx_constraint_t> &out_joints)
{
    // [Joint.load](https://github.com/MMD-Blender/blender_mmd_tools/blob/main/mmd_tools/core/pmx/__init__.py#L1528)

    // https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CDB3638D-23AF-49EF-8EF6-53081EE4D39D

    assert(out_joints.empty());
    out_joints = {};

    uint32_t joint_count;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &joint_count)))
    {
        return false;
    }

    if (internal_unlikely(0U == joint_count || (joint_count > static_cast<uint32_t>(INT32_MAX))))
    {
        // Tolerance
        return true;
    }

    out_joints.resize(joint_count);

    for (uint32_t joint_index = 0U; joint_index < joint_count; ++joint_index)
    {
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, out_joints[joint_index].m_name)))
        {
            return false;
        }

        mcrt_string unused_name_e;
        if (internal_unlikely(!internal_data_read_mmd_pmx_text(data_base, data_size, inout_data_offset, text_encoding, unused_name_e)))
        {
            return false;
        }

        uint8_t joint_type;
        if (internal_unlikely(!internal_data_read_uint8(data_base, data_size, inout_data_offset, &joint_type)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, rigid_body_index_size, &out_joints[joint_index].m_rigid_body_a_index)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_signed_index(data_base, data_size, inout_data_offset, rigid_body_index_size, &out_joints[joint_index].m_rigid_body_b_index)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_translation)))
        {
            return false;
        }

        if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_rotation)))
        {
            return false;
        }

        if (1U == joint_type)
        {
            // 6 DOF

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_translation_limit_min)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_translation_limit_max)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_rotation_limit_min)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_rotation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_translation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_translation)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_rotation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_rotation)))
            {
                return false;
            }
        }
        else if (2U == joint_type)
        {
            // Ball and Socket

            mmd_pmx_vec3_t unused_translation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation_limit_min)))
            {
                return false;
            }

            constexpr float const INTERNAL_M_PI = 3.14159265358979323846264338327950288F;

            out_joints[joint_index].m_translation_limit_min.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_z = 0.0F;

            out_joints[joint_index].m_translation_limit_max.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_z = 0.0F;

            out_joints[joint_index].m_rotation_limit_min.m_x = -INTERNAL_M_PI;
            out_joints[joint_index].m_rotation_limit_min.m_y = -INTERNAL_M_PI;
            out_joints[joint_index].m_rotation_limit_min.m_z = -INTERNAL_M_PI;

            out_joints[joint_index].m_rotation_limit_max.m_x = INTERNAL_M_PI;
            out_joints[joint_index].m_rotation_limit_max.m_y = INTERNAL_M_PI;
            out_joints[joint_index].m_rotation_limit_max.m_z = INTERNAL_M_PI;

            mmd_pmx_vec3_t unused_translation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_rotation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_rotation_limit_min)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_rotation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_rotation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_translation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_translation)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_rotation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_rotation)))
            {
                return false;
            }
        }
        else if (3U == joint_type)
        {
            // Ragdoll

            out_joints[joint_index].m_translation_limit_min.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_z = 0.0F;

            out_joints[joint_index].m_translation_limit_max.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_z = 0.0F;

            mmd_pmx_vec3_t unused_translation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation_limit_min)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_translation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t rotation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &rotation_limit_min)))
            {
                return false;
            }

            out_joints[joint_index].m_rotation_limit_min.m_x = -rotation_limit_min.m_x;
            out_joints[joint_index].m_rotation_limit_min.m_y = -rotation_limit_min.m_y;
            out_joints[joint_index].m_rotation_limit_min.m_z = -rotation_limit_min.m_z;

            out_joints[joint_index].m_rotation_limit_max.m_x = rotation_limit_min.m_x;
            out_joints[joint_index].m_rotation_limit_max.m_y = rotation_limit_min.m_y;
            out_joints[joint_index].m_rotation_limit_max.m_z = rotation_limit_min.m_z;

            mmd_pmx_vec3_t unused_rotation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_rotation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t spring_translation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &spring_translation)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_rotation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_rotation)))
            {
                return false;
            }
        }
        else if (4U == joint_type)
        {
            // Prismatic

            mmd_pmx_vec3_t translation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &translation_limit_min)))
            {
                return false;
            }

            mmd_pmx_vec3_t translation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &translation_limit_max)))
            {
                return false;
            }

            out_joints[joint_index].m_translation_limit_min.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_z = translation_limit_min.m_x;

            out_joints[joint_index].m_translation_limit_max.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_z = translation_limit_max.m_x;

            mmd_pmx_vec3_t rotation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &rotation_limit_min)))
            {
                return false;
            }

            mmd_pmx_vec3_t rotation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &rotation_limit_max)))
            {
                return false;
            }

            out_joints[joint_index].m_rotation_limit_min.m_x = 0.0F;
            out_joints[joint_index].m_rotation_limit_min.m_y = 0.0F;
            out_joints[joint_index].m_rotation_limit_min.m_z = rotation_limit_min.m_x;

            out_joints[joint_index].m_rotation_limit_max.m_x = 0.0F;
            out_joints[joint_index].m_rotation_limit_max.m_y = 0.0F;
            out_joints[joint_index].m_rotation_limit_max.m_z = rotation_limit_max.m_x;

            mmd_pmx_vec3_t unused_spring_translation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_translation)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_rotation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_rotation)))
            {
                return false;
            }
        }
        else if (5U == joint_type)
        {
            // Hinge

            out_joints[joint_index].m_translation_limit_min.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_min.m_z = 0.0F;

            out_joints[joint_index].m_translation_limit_max.m_x = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_y = 0.0F;
            out_joints[joint_index].m_translation_limit_max.m_z = 0.0F;

            mmd_pmx_vec3_t unused_translation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation_limit_min)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_translation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_translation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t rotation_limit_min;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &rotation_limit_min)))
            {
                return false;
            }

            mmd_pmx_vec3_t rotation_limit_max;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &rotation_limit_max)))
            {
                return false;
            }

            out_joints[joint_index].m_rotation_limit_min.m_x = 0.0F;
            out_joints[joint_index].m_rotation_limit_min.m_y = 0.0F;
            out_joints[joint_index].m_rotation_limit_min.m_z = rotation_limit_min.m_x;

            out_joints[joint_index].m_rotation_limit_max.m_x = 0.0F;
            out_joints[joint_index].m_rotation_limit_max.m_y = 0.0F;
            out_joints[joint_index].m_rotation_limit_max.m_z = rotation_limit_max.m_x;

            mmd_pmx_vec3_t unused_spring_translation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_translation)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_rotation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_rotation)))
            {
                return false;
            }
        }
        else
        {
            // Tolerance
            assert(0 == joint_type);

            // bounce 6DOF

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_translation_limit_min)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_translation_limit_max)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_rotation_limit_min)))
            {
                return false;
            }

            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &out_joints[joint_index].m_rotation_limit_max)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_translation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_translation)))
            {
                return false;
            }

            mmd_pmx_vec3_t unused_spring_rotation;
            if (internal_unlikely(!internal_data_read_mmd_pmx_vec3(data_base, data_size, inout_data_offset, &unused_spring_rotation)))
            {
                return false;
            }
        }
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_vec2(void const *data_base, size_t data_size, size_t &inout_data_offset, mmd_pmx_vec2_t *out_vec3)
{
    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_x)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_y)))
    {
        return false;
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_vec3(void const *data_base, size_t data_size, size_t &inout_data_offset, mmd_pmx_vec3_t *out_vec3)
{
    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_x)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_y)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_z)))
    {
        return false;
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_vec4(void const *data_base, size_t data_size, size_t &inout_data_offset, mmd_pmx_vec4_t *out_vec3)
{
    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_x)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_y)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_z)))
    {
        return false;
    }

    if (internal_unlikely(!internal_data_read_float(data_base, data_size, inout_data_offset, &out_vec3->m_w)))
    {
        return false;
    }

    return true;
}

static inline bool internal_data_read_mmd_pmx_text(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t text_encoding, mcrt_string &out_text)
{
    // use "out_text, s8" to display the UTF-8 string
    // https://learn.microsoft.com/en-us/visualstudio/debugger/format-specifiers-in-cpp

    assert(out_text.empty());
    out_text = {};

    uint32_t length;
    if (internal_unlikely(!internal_data_read_uint32(data_base, data_size, inout_data_offset, &length)))
    {
        return false;
    }

    if (internal_unlikely(0U == length))
    {
        return true;
    }

    if (0U == text_encoding)
    {
        static_assert(sizeof(uint16_t) == 2U, "");

        // Tolerance
        assert(0U == (length & (2U - 1U)));

        mcrt_vector<uint16_t> value;
        value.resize(static_cast<size_t>(((length + 1U) >> 1U) + 1U), static_cast<uint8_t>(0U));
        if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, length, value.data())))
        {
            return false;
        }

        mcrt_vector<uint16_t> &src_utf16 = value;
        mcrt_vector<uint8_t> dst_utf8;

        constexpr size_t const UNI_MAX_UTF8_BYTES_PER_CODE_POINT = 4U;
        dst_utf8.resize(src_utf16.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1U);

        char *in_buf = reinterpret_cast<char *>(src_utf16.data());
        static_assert(sizeof(uint16_t) == sizeof(decltype(src_utf16[0])), "");
        size_t in_bytes_left = sizeof(uint16_t) * src_utf16.size();

        char *out_buf = reinterpret_cast<char *>(dst_utf8.data());
        static_assert(sizeof(uint8_t) == sizeof(decltype(dst_utf8[0])), "");
        size_t out_bytes_left = sizeof(uint8_t) * dst_utf8.size();

        iconv_t conversion_descriptor = iconv_open("UTF-8//IGNORE", "UTF-16LE");
        if (internal_unlikely(reinterpret_cast<iconv_t>(-1) == conversion_descriptor))
        {
            // Tolerance
            assert(false);
            return true;
        }

        size_t conversion_result = iconv(conversion_descriptor, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);

        int result = iconv_close(conversion_descriptor);
        assert(-1 != result);

        if (internal_unlikely(static_cast<size_t>(-1) == conversion_result))
        {
            // Tolerance
            assert(false);
            return true;
        }

        size_t conversion_size = reinterpret_cast<decltype(&dst_utf8[0])>(out_buf) - &dst_utf8[0];
        dst_utf8.resize(conversion_size + 1U);

        dst_utf8[conversion_size] = '\0';

        // do not use "std::move"
        // the "memory" of "dst_utf8" is greater than the "size" of "dst_utf8"
        out_text = reinterpret_cast<char *>(&dst_utf8[0]);

        return true;
    }
    else if (1U == text_encoding)
    {
        mcrt_vector<uint8_t> value;
        value.resize(length + 1U);
        if (internal_unlikely(!internal_data_read_bytes(data_base, data_size, inout_data_offset, length, value.data())))
        {
            return false;
        }
        value[length] = '\0';

        out_text = reinterpret_cast<char *>(&value[0]);

        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_mmd_pmx_signed_index(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t index_size, uint32_t *out_index)
{
    if (1U == index_size)
    {
        uint8_t index;
        if (internal_data_read_uint8(data_base, data_size, inout_data_offset, &index))
        {
            if (index <= INT8_MAX)
            {
                (*out_index) = index;
            }
            else
            {
                assert(UINT8_MAX == index);
                (*out_index) = BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (2U == index_size)
    {
        uint16_t index;
        if (internal_data_read_uint16(data_base, data_size, inout_data_offset, &index))
        {
            if (index <= INT16_MAX)
            {
                (*out_index) = index;
            }
            else
            {
                assert(UINT16_MAX == index);
                (*out_index) = BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (4U == index_size)
    {
        uint32_t index;
        if (internal_data_read_uint32(data_base, data_size, inout_data_offset, &index))
        {
            if (index <= INT32_MAX)
            {
                (*out_index) = index;
            }
            else
            {
                assert(UINT32_MAX == index);
                (*out_index) = BRX_ASSET_IMPORT_UINT32_INDEX_INVALID;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_mmd_pmx_unsigned_index(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t index_size, uint32_t *out_index)
{
    if (1U == index_size)
    {
        uint8_t index;
        if (internal_data_read_uint8(data_base, data_size, inout_data_offset, &index))
        {
            (*out_index) = index;
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (2U == index_size)
    {
        uint16_t index;
        if (internal_data_read_uint16(data_base, data_size, inout_data_offset, &index))
        {
            (*out_index) = index;
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (4U == index_size)
    {
        uint32_t index;
        if (internal_data_read_uint32(data_base, data_size, inout_data_offset, &index))
        {
            (*out_index) = index;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_uint8(void const *data_base, size_t data_size, size_t &inout_data_offset, uint8_t *out_uint8)
{
    if (data_size >= (inout_data_offset + sizeof(uint8_t)))
    {
        (*out_uint8) = (*reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset));
        inout_data_offset += sizeof(uint8_t);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_uint16(void const *data_base, size_t data_size, size_t &inout_data_offset, uint16_t *out_uint16)
{
    if (data_size >= (inout_data_offset + (sizeof(uint8_t) * 2U)))
    {
        uint8_t const *const not_aligned_le_uint16_base = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset);
        uint8_t const not_aligned_le_uint16[2] = {not_aligned_le_uint16_base[0], not_aligned_le_uint16_base[1]};
        inout_data_offset += (sizeof(uint8_t) * 2U);
        uint32_t const aligned_le_uint16 = std::bit_cast<uint16_t>(not_aligned_le_uint16);
        (*out_uint16) = internal_le_to_h_16(aligned_le_uint16);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_uint32(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t *out_uint32)
{
    if (data_size >= (inout_data_offset + (sizeof(uint8_t) * 4U)))
    {
        uint8_t const *const not_aligned_le_uint32_base = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset);
        uint8_t const not_aligned_le_uint32[4] = {not_aligned_le_uint32_base[0], not_aligned_le_uint32_base[1], not_aligned_le_uint32_base[2], not_aligned_le_uint32_base[3]};
        inout_data_offset += (sizeof(uint8_t) * 4U);
        uint32_t const aligned_le_uint32 = std::bit_cast<uint32_t>(not_aligned_le_uint32);
        (*out_uint32) = internal_le_to_h_32(aligned_le_uint32);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_float(void const *data_base, size_t data_size, size_t &inout_data_offset, float *out_float)
{
    if (data_size >= (inout_data_offset + (sizeof(uint8_t) * 4U)))
    {
        uint8_t const *const not_aligned_le_uint32_base = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset);
        uint8_t const not_aligned_le_uint32[4] = {not_aligned_le_uint32_base[0], not_aligned_le_uint32_base[1], not_aligned_le_uint32_base[2], not_aligned_le_uint32_base[3]};
        inout_data_offset += (sizeof(uint8_t) * 4U);
        uint32_t const aligned_le_uint32 = std::bit_cast<uint32_t>(not_aligned_le_uint32);
        uint32_t const aligned_host_uint32 = internal_le_to_h_32(aligned_le_uint32);
        (*out_float) = std::bit_cast<float>(aligned_host_uint32);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool internal_data_read_bytes(void const *data_base, size_t data_size, size_t &inout_data_offset, uint32_t length, void *out_bytes)
{
    if (data_size >= (inout_data_offset + (sizeof(uint8_t) * length)))
    {
        std::memcpy(out_bytes, reinterpret_cast<void const *>(reinterpret_cast<uintptr_t>(data_base) + inout_data_offset), length);
        inout_data_offset += (sizeof(uint8_t) * length);
        return true;
    }
    else
    {
        return false;
    }
}
