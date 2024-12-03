//-------------------------------------------------------------------------------------
// DirectXMesh.h
//
// DirectX Mesh Geometry Library
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//-------------------------------------------------------------------------------------

#ifndef _DIRECTX_MESH_H_
#define _DIRECTX_MESH_H_ 1

#include <stddef.h>
#include <stdint.h>

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

#define DIRECTX_MESH_VERSION 167

namespace DirectX
{
    //---------------------------------------------------------------------------------
    // Normals, Tangents, and Bi-Tangents Computation

    enum CNORM_FLAGS : uint32_t
    {
        CNORM_DEFAULT = 0x0,
        // Default is to compute normals using weight-by-angle

        CNORM_WEIGHT_BY_AREA = 0x1,
        // Computes normals using weight-by-area

        CNORM_WEIGHT_EQUAL = 0x2,
        // Compute normals with equal weights

        CNORM_WIND_CW = 0x4,
        // Vertices are clock-wise (defaults to CCW)
    };

    extern bool ComputeNormals(
        _In_reads_(nFaces * 3) const uint16_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions, _In_ size_t nVerts,
        _In_ CNORM_FLAGS flags,
        _Out_writes_(nVerts) XMFLOAT3 *normals) noexcept;
    extern bool ComputeNormals(
        _In_reads_(nFaces * 3) const uint32_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions, _In_ size_t nVerts,
        _In_ CNORM_FLAGS flags,
        _Out_writes_(nVerts) XMFLOAT3 *normals) noexcept;
    // Computes vertex normals

    extern bool ComputeTangentFrame(
        _In_reads_(nFaces * 3) const uint16_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions,
        _In_reads_(nVerts) const XMFLOAT3 *normals,
        _In_reads_(nVerts) const XMFLOAT2 *texcoords, _In_ size_t nVerts,
        _Out_writes_opt_(nVerts) XMFLOAT3 *tangents,
        _Out_writes_opt_(nVerts) XMFLOAT3 *bitangents) noexcept;

    extern bool ComputeTangentFrame(
        _In_reads_(nFaces * 3) const uint32_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions,
        _In_reads_(nVerts) const XMFLOAT3 *normals,
        _In_reads_(nVerts) const XMFLOAT2 *texcoords, _In_ size_t nVerts,
        _Out_writes_opt_(nVerts) XMFLOAT3 *tangents,
        _Out_writes_opt_(nVerts) XMFLOAT3 *bitangents) noexcept;

    extern bool ComputeTangentFrame(
        _In_reads_(nFaces * 3) const uint16_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions,
        _In_reads_(nVerts) const XMFLOAT3 *normals,
        _In_reads_(nVerts) const XMFLOAT2 *texcoords, _In_ size_t nVerts,
        _Out_writes_opt_(nVerts) XMFLOAT4 *tangents,
        _Out_writes_opt_(nVerts) XMFLOAT3 *bitangents) noexcept;

    extern bool ComputeTangentFrame(
        _In_reads_(nFaces * 3) const uint32_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions,
        _In_reads_(nVerts) const XMFLOAT3 *normals,
        _In_reads_(nVerts) const XMFLOAT2 *texcoords, _In_ size_t nVerts,
        _Out_writes_opt_(nVerts) XMFLOAT4 *tangents,
        _Out_writes_opt_(nVerts) XMFLOAT3 *bitangents) noexcept;

    extern bool ComputeTangentFrame(
        _In_reads_(nFaces * 3) const uint16_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions,
        _In_reads_(nVerts) const XMFLOAT3 *normals,
        _In_reads_(nVerts) const XMFLOAT2 *texcoords, _In_ size_t nVerts,
        _Out_writes_(nVerts) XMFLOAT4 *tangents) noexcept;

    extern bool ComputeTangentFrame(
        _In_reads_(nFaces * 3) const uint32_t *indices, _In_ size_t nFaces,
        _In_reads_(nVerts) const XMFLOAT3 *positions,
        _In_reads_(nVerts) const XMFLOAT3 *normals,
        _In_reads_(nVerts) const XMFLOAT2 *texcoords, _In_ size_t nVerts,
        _Out_writes_(nVerts) XMFLOAT4 *tangents) noexcept;
    // Computes tangents and/or bi-tangents (optionally with handedness stored in .w)

} // namespace

#endif
