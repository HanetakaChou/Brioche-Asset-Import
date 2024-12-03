//-------------------------------------------------------------------------------------
// DirectXMeshNormals.cpp
//
// DirectX Mesh Geometry Library - Normal computation
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//-------------------------------------------------------------------------------------

#include "DirectXMesh.h"
#include "scoped.h"
#include <cstring>

namespace
{
    //---------------------------------------------------------------------------------
    // Compute normals with equal weighting
    //---------------------------------------------------------------------------------
    template <class index_t>
    bool inline ComputeNormalsEqualWeight(
        _In_reads_(nFaces * 3) const index_t *indices, size_t nFaces,
        _In_reads_(nVerts) const DirectX::XMFLOAT3 *positions, size_t nVerts,
        bool cw, _Out_writes_(nVerts) DirectX::XMFLOAT3 *normals) noexcept
    {
        auto temp = make_AlignedArrayXMVECTOR(nVerts);
        if (!temp)
        {
            // E_OUTOFMEMORY
            return false;
        }

        DirectX::XMVECTOR *vertNormals = temp.get();
        std::memset(vertNormals, 0, sizeof(DirectX::XMVECTOR) * nVerts);

        for (size_t face = 0; face < nFaces; ++face)
        {
            index_t i0 = indices[face * 3];
            index_t i1 = indices[face * 3 + 1];
            index_t i2 = indices[face * 3 + 2];

            if (i0 == index_t(-1) || i1 == index_t(-1) || i2 == index_t(-1))
            {
                continue;
            }

            if (i0 >= nVerts || i1 >= nVerts || i2 >= nVerts)
            {
                // E_UNEXPECTED
                return false;
            }

            const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&positions[i0]);
            const DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&positions[i1]);
            const DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&positions[i2]);

            const DirectX::XMVECTOR u = DirectX::XMVectorSubtract(p2, p1);
            const DirectX::XMVECTOR v = DirectX::XMVectorSubtract(p3, p1);

            const DirectX::XMVECTOR faceNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));

            vertNormals[i0] = DirectX::XMVectorAdd(vertNormals[i0], faceNormal);
            vertNormals[i1] = DirectX::XMVectorAdd(vertNormals[i1], faceNormal);
            vertNormals[i2] = DirectX::XMVectorAdd(vertNormals[i2], faceNormal);
        }

        // Store results
        if (cw)
        {
            for (size_t vert = 0; vert < nVerts; ++vert)
            {
                DirectX::XMVECTOR n = DirectX::XMVector3Normalize(vertNormals[vert]);
                n = DirectX::XMVectorNegate(n);
                DirectX::XMStoreFloat3(&normals[vert], n);
            }
        }
        else
        {
            for (size_t vert = 0; vert < nVerts; ++vert)
            {
                const DirectX::XMVECTOR n = DirectX::XMVector3Normalize(vertNormals[vert]);
                DirectX::XMStoreFloat3(&normals[vert], n);
            }
        }

        // S_OK
        return true;
    }

    //---------------------------------------------------------------------------------
    // Compute normals with weighting by angle
    //---------------------------------------------------------------------------------
    template <class index_t>
    bool ComputeNormalsWeightedByAngle(
        _In_reads_(nFaces * 3) const index_t *indices, size_t nFaces,
        _In_reads_(nVerts) const DirectX::XMFLOAT3 *positions, size_t nVerts,
        bool cw, _Out_writes_(nVerts) DirectX::XMFLOAT3 *normals) noexcept
    {
        auto temp = make_AlignedArrayXMVECTOR(nVerts);
        if (!temp)
        {
            // E_OUTOFMEMORY
            return false;
        }

        DirectX::XMVECTOR *vertNormals = temp.get();
        std::memset(vertNormals, 0, sizeof(DirectX::XMVECTOR) * nVerts);

        for (size_t face = 0; face < nFaces; ++face)
        {
            index_t i0 = indices[face * 3];
            index_t i1 = indices[face * 3 + 1];
            index_t i2 = indices[face * 3 + 2];

            if (i0 == index_t(-1) || i1 == index_t(-1) || i2 == index_t(-1))
            {
                continue;
            }

            if (i0 >= nVerts || i1 >= nVerts || i2 >= nVerts)
            {
                // E_UNEXPECTED
                return false;
            }

            const DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&positions[i0]);
            const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&positions[i1]);
            const DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&positions[i2]);

            const DirectX::XMVECTOR u = DirectX::XMVectorSubtract(p1, p0);
            const DirectX::XMVECTOR v = DirectX::XMVectorSubtract(p2, p0);

            const DirectX::XMVECTOR faceNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));

            // Corner 0 -> 1 - 0, 2 - 0
            const DirectX::XMVECTOR a = DirectX::XMVector3Normalize(u);
            const DirectX::XMVECTOR b = DirectX::XMVector3Normalize(v);
            DirectX::XMVECTOR w0 = DirectX::XMVector3Dot(a, b);
            w0 = DirectX::XMVectorClamp(w0, DirectX::g_XMNegativeOne, DirectX::g_XMOne);
            w0 = DirectX::XMVectorACos(w0);

            // Corner 1 -> 2 - 1, 0 - 1
            const DirectX::XMVECTOR c = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(p2, p1));
            const DirectX::XMVECTOR d = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(p0, p1));
            DirectX::XMVECTOR w1 = DirectX::XMVector3Dot(c, d);
            w1 = DirectX::XMVectorClamp(w1, DirectX::g_XMNegativeOne, DirectX::g_XMOne);
            w1 = DirectX::XMVectorACos(w1);

            // Corner 2 -> 0 - 2, 1 - 2
            const DirectX::XMVECTOR e = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(p0, p2));
            const DirectX::XMVECTOR f = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(p1, p2));
            DirectX::XMVECTOR w2 = DirectX::XMVector3Dot(e, f);
            w2 = DirectX::XMVectorClamp(w2, DirectX::g_XMNegativeOne, DirectX::g_XMOne);
            w2 = DirectX::XMVectorACos(w2);

            vertNormals[i0] = DirectX::XMVectorMultiplyAdd(faceNormal, w0, vertNormals[i0]);
            vertNormals[i1] = DirectX::XMVectorMultiplyAdd(faceNormal, w1, vertNormals[i1]);
            vertNormals[i2] = DirectX::XMVectorMultiplyAdd(faceNormal, w2, vertNormals[i2]);
        }

        // Store results
        if (cw)
        {
            for (size_t vert = 0; vert < nVerts; ++vert)
            {
                DirectX::XMVECTOR n = DirectX::XMVector3Normalize(vertNormals[vert]);
                n = DirectX::XMVectorNegate(n);
                DirectX::XMStoreFloat3(&normals[vert], n);
            }
        }
        else
        {
            for (size_t vert = 0; vert < nVerts; ++vert)
            {
                const DirectX::XMVECTOR n = DirectX::XMVector3Normalize(vertNormals[vert]);
                DirectX::XMStoreFloat3(&normals[vert], n);
            }
        }

        // S_OK
        return true;
    }

    //---------------------------------------------------------------------------------
    // Compute normals with weighting by face area
    //---------------------------------------------------------------------------------
    template <class index_t>
    bool ComputeNormalsWeightedByArea(
        _In_reads_(nFaces * 3) const index_t *indices, size_t nFaces,
        _In_reads_(nVerts) const DirectX::XMFLOAT3 *positions, size_t nVerts,
        bool cw, _Out_writes_(nVerts) DirectX::XMFLOAT3 *normals) noexcept
    {
        auto temp = make_AlignedArrayXMVECTOR(nVerts);
        if (!temp)
        {
            // E_OUTOFMEMORY
            return false;
        }

        DirectX::XMVECTOR *vertNormals = temp.get();
        std::memset(vertNormals, 0, sizeof(DirectX::XMVECTOR) * nVerts);

        for (size_t face = 0; face < nFaces; ++face)
        {
            index_t i0 = indices[face * 3];
            index_t i1 = indices[face * 3 + 1];
            index_t i2 = indices[face * 3 + 2];

            if (i0 == index_t(-1) || i1 == index_t(-1) || i2 == index_t(-1))
            {
                continue;
            }

            if (i0 >= nVerts || i1 >= nVerts || i2 >= nVerts)
            {
                // E_UNEXPECTED
                return false;
            }

            const DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&positions[i0]);
            const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&positions[i1]);
            const DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&positions[i2]);

            const DirectX::XMVECTOR u = DirectX::XMVectorSubtract(p1, p0);
            const DirectX::XMVECTOR v = DirectX::XMVectorSubtract(p2, p0);

            const DirectX::XMVECTOR faceNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));

            // Corner 0 -> 1 - 0, 2 - 0
            DirectX::XMVECTOR w0 = DirectX::XMVector3Cross(u, v);
            w0 = DirectX::XMVector3Length(w0);

            // Corner 1 -> 2 - 1, 0 - 1
            const DirectX::XMVECTOR c = DirectX::XMVectorSubtract(p2, p1);
            const DirectX::XMVECTOR d = DirectX::XMVectorSubtract(p0, p1);
            DirectX::XMVECTOR w1 = DirectX::XMVector3Cross(c, d);
            w1 = DirectX::XMVector3Length(w1);

            // Corner 2 -> 0 - 2, 1 - 2
            const DirectX::XMVECTOR e = DirectX::XMVectorSubtract(p0, p2);
            const DirectX::XMVECTOR f = DirectX::XMVectorSubtract(p1, p2);
            DirectX::XMVECTOR w2 = DirectX::XMVector3Cross(e, f);
            w2 = DirectX::XMVector3Length(w2);

            vertNormals[i0] = DirectX::XMVectorMultiplyAdd(faceNormal, w0, vertNormals[i0]);
            vertNormals[i1] = DirectX::XMVectorMultiplyAdd(faceNormal, w1, vertNormals[i1]);
            vertNormals[i2] = DirectX::XMVectorMultiplyAdd(faceNormal, w2, vertNormals[i2]);
        }

        // Store results
        if (cw)
        {
            for (size_t vert = 0; vert < nVerts; ++vert)
            {
                DirectX::XMVECTOR n = DirectX::XMVector3Normalize(vertNormals[vert]);
                n = DirectX::XMVectorNegate(n);
                DirectX::XMStoreFloat3(&normals[vert], n);
            }
        }
        else
        {
            for (size_t vert = 0; vert < nVerts; ++vert)
            {
                const DirectX::XMVECTOR n = DirectX::XMVector3Normalize(vertNormals[vert]);
                DirectX::XMStoreFloat3(&normals[vert], n);
            }
        }

        // S_OK
        return true;
    }
}

//=====================================================================================
// Entry-points
//=====================================================================================

//-------------------------------------------------------------------------------------
_Use_decl_annotations_ extern bool DirectX::ComputeNormals(
    const uint16_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    size_t nVerts,
    CNORM_FLAGS flags,
    DirectX::XMFLOAT3 *normals) noexcept
{
    if (!indices || !positions || !nFaces || !nVerts || !normals)
    {
        // E_INVALIDARG
        return false;
    }

    if (nVerts >= UINT16_MAX)
    {
        // E_INVALIDARG
        return false;
    }

    if ((uint64_t(nFaces) * 3) >= UINT32_MAX)
    {
        // HRESULT_E_ARITHMETIC_OVERFLOW
        return false;
    }

    const bool cw = (flags & CNORM_WIND_CW) ? true : false;

    if (flags & CNORM_WEIGHT_BY_AREA)
    {
        return ComputeNormalsWeightedByArea<uint16_t>(indices, nFaces, positions, nVerts, cw, normals);
    }
    else if (flags & CNORM_WEIGHT_EQUAL)
    {
        return ComputeNormalsEqualWeight<uint16_t>(indices, nFaces, positions, nVerts, cw, normals);
    }
    else
    {
        return ComputeNormalsWeightedByAngle<uint16_t>(indices, nFaces, positions, nVerts, cw, normals);
    }
}

_Use_decl_annotations_ extern bool DirectX::ComputeNormals(
    const uint32_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    size_t nVerts,
    CNORM_FLAGS flags,
    DirectX::XMFLOAT3 *normals) noexcept
{
    if (!indices || !positions || !nFaces || !nVerts || !normals)
    {
        // E_INVALIDARG
        return false;
    }

    if (nVerts >= UINT32_MAX)
    {
        // E_INVALIDARG
        return false;
    }

    if ((uint64_t(nFaces) * 3) >= UINT32_MAX)
    {
        // HRESULT_E_ARITHMETIC_OVERFLOW
        return false;
    }

    const bool cw = (flags & CNORM_WIND_CW) ? true : false;

    if (flags & CNORM_WEIGHT_BY_AREA)
    {
        return ComputeNormalsWeightedByArea<uint32_t>(indices, nFaces, positions, nVerts, cw, normals);
    }
    else if (flags & CNORM_WEIGHT_EQUAL)
    {
        return ComputeNormalsEqualWeight<uint32_t>(indices, nFaces, positions, nVerts, cw, normals);
    }
    else
    {
        return ComputeNormalsWeightedByAngle<uint32_t>(indices, nFaces, positions, nVerts, cw, normals);
    }
}
