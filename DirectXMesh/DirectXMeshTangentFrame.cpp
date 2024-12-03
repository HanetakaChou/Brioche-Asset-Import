//-------------------------------------------------------------------------------------
// DirectXMeshTangentFrame.cpp
//
// DirectX Mesh Geometry Library - Normals, Tangents, and Bi-Tangents Computation
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//-------------------------------------------------------------------------------------

#include "DirectXMesh.h"
#include "scoped.h"
#include <cmath>
#include <cstring>

namespace
{
    //---------------------------------------------------------------------------------
    // Compute tangent and bi-tangent for each vertex
    //---------------------------------------------------------------------------------
    template <class index_t>
    inline bool ComputeTangentFrameImpl(
        _In_reads_(nFaces * 3) const index_t *indices, size_t nFaces,
        _In_reads_(nVerts) const DirectX::XMFLOAT3 *positions,
        _In_reads_(nVerts) const DirectX::XMFLOAT3 *normals,
        _In_reads_(nVerts) const DirectX::XMFLOAT2 *texcoords,
        size_t nVerts,
        _Out_writes_opt_(nVerts) DirectX::XMFLOAT3 *tangents3,
        _Out_writes_opt_(nVerts) DirectX::XMFLOAT4 *tangents4,
        _Out_writes_opt_(nVerts) DirectX::XMFLOAT3 *bitangents) noexcept
    {
        if (!indices || !nFaces || !positions || !normals || !texcoords || !nVerts)
        {
            // E_INVALIDARG
            return false;
        }

        if (nVerts >= index_t(-1))
        {
            // E_INVALIDARG
            return false;
        }

        if ((uint64_t(nFaces) * 3) >= UINT32_MAX)
        {
            // HRESULT_E_ARITHMETIC_OVERFLOW
            return false;
        }

        static constexpr float EPSILON = 0.0001f;
        static const DirectX::XMVECTORF32 s_flips = {{{1.f, -1.f, -1.f, 1.f}}};

        auto temp = make_AlignedArrayXMVECTOR(uint64_t(nVerts) * 2);
        if (!temp)
        {
            // E_OUTOFMEMORY
            return false;
        }

        std::memset(temp.get(), 0, sizeof(DirectX::XMVECTOR) * nVerts * 2);

        DirectX::XMVECTOR *tangent1 = temp.get();
        DirectX::XMVECTOR *tangent2 = temp.get() + nVerts;

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

            const DirectX::XMVECTOR t0 = DirectX::XMLoadFloat2(&texcoords[i0]);
            const DirectX::XMVECTOR t1 = DirectX::XMLoadFloat2(&texcoords[i1]);
            const DirectX::XMVECTOR t2 = DirectX::XMLoadFloat2(&texcoords[i2]);

            DirectX::XMVECTOR s = DirectX::XMVectorMergeXY(DirectX::XMVectorSubtract(t1, t0), DirectX::XMVectorSubtract(t2, t0));

            DirectX::XMFLOAT4A tmp;
            DirectX::XMStoreFloat4A(&tmp, s);

            float d = tmp.x * tmp.w - tmp.z * tmp.y;
            d = (std::abs(d) <= EPSILON) ? 1.f : (1.f / d);
            s = DirectX::XMVectorScale(s, d);
            s = DirectX::XMVectorMultiply(s, s_flips);

            DirectX::XMMATRIX m0;
            m0.r[0] = DirectX::XMVectorPermute<3, 2, 6, 7>(s, DirectX::g_XMZero);
            m0.r[1] = DirectX::XMVectorPermute<1, 0, 4, 5>(s, DirectX::g_XMZero);
            m0.r[2] = m0.r[3] = DirectX::g_XMZero;

            const DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&positions[i0]);
            const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&positions[i1]);
            DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&positions[i2]);

            DirectX::XMMATRIX m1;
            m1.r[0] = DirectX::XMVectorSubtract(p1, p0);
            m1.r[1] = DirectX::XMVectorSubtract(p2, p0);
            m1.r[2] = m1.r[3] = DirectX::g_XMZero;

            const DirectX::XMMATRIX uv = XMMatrixMultiply(m0, m1);

            tangent1[i0] = DirectX::XMVectorAdd(tangent1[i0], uv.r[0]);
            tangent1[i1] = DirectX::XMVectorAdd(tangent1[i1], uv.r[0]);
            tangent1[i2] = DirectX::XMVectorAdd(tangent1[i2], uv.r[0]);

            tangent2[i0] = DirectX::XMVectorAdd(tangent2[i0], uv.r[1]);
            tangent2[i1] = DirectX::XMVectorAdd(tangent2[i1], uv.r[1]);
            tangent2[i2] = DirectX::XMVectorAdd(tangent2[i2], uv.r[1]);
        }

        for (size_t j = 0; j < nVerts; ++j)
        {
            // Gram-Schmidt orthonormalization
            DirectX::XMVECTOR b0 = DirectX::XMLoadFloat3(&normals[j]);
            b0 = DirectX::XMVector3Normalize(b0);

            const DirectX::XMVECTOR tan1 = tangent1[j];
            DirectX::XMVECTOR b1 = DirectX::XMVectorSubtract(tan1, DirectX::XMVectorMultiply(DirectX::XMVector3Dot(b0, tan1), b0));
            b1 = DirectX::XMVector3Normalize(b1);

            const DirectX::XMVECTOR tan2 = tangent2[j];
            DirectX::XMVECTOR b2 = DirectX::XMVectorSubtract(DirectX::XMVectorSubtract(tan2, DirectX::XMVectorMultiply(DirectX::XMVector3Dot(b0, tan2), b0)), DirectX::XMVectorMultiply(DirectX::XMVector3Dot(b1, tan2), b1));
            b2 = DirectX::XMVector3Normalize(b2);

            // handle degenerate vectors
            const float len1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(b1));
            const float len2 = DirectX::XMVectorGetY(DirectX::XMVector3Length(b2));

            if ((len1 <= EPSILON) || (len2 <= EPSILON))
            {
                if (len1 > 0.5f)
                {
                    // Reset bi-tangent from tangent and normal
                    b2 = DirectX::XMVector3Cross(b0, b1);
                }
                else if (len2 > 0.5f)
                {
                    // Reset tangent from bi-tangent and normal
                    b1 = DirectX::XMVector3Cross(b2, b0);
                }
                else
                {
                    // Reset both tangent and bi-tangent from normal
                    DirectX::XMVECTOR axis;

                    const float d0 = std::abs(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::g_XMIdentityR0, b0)));
                    const float d1 = std::abs(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::g_XMIdentityR1, b0)));
                    const float d2 = std::abs(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::g_XMIdentityR2, b0)));
                    if (d0 < d1)
                    {
                        axis = (d0 < d2) ? DirectX::g_XMIdentityR0 : DirectX::g_XMIdentityR2;
                    }
                    else if (d1 < d2)
                    {
                        axis = DirectX::g_XMIdentityR1;
                    }
                    else
                    {
                        axis = DirectX::g_XMIdentityR2;
                    }

                    b1 = DirectX::XMVector3Cross(b0, axis);
                    b2 = DirectX::XMVector3Cross(b0, b1);
                }
            }

            if (tangents3)
            {
                DirectX::XMStoreFloat3(&tangents3[j], b1);
            }

            if (tangents4)
            {
                DirectX::XMVECTOR bi = DirectX::XMVector3Cross(b0, tan1);
                const float w = DirectX::XMVector3Less(DirectX::XMVector3Dot(bi, tan2), DirectX::g_XMZero) ? -1.f : 1.f;

                bi = DirectX::XMVectorSetW(b1, w);
                DirectX::XMStoreFloat4(&tangents4[j], bi);
            }

            if (bitangents)
            {
                DirectX::XMStoreFloat3(&bitangents[j], b2);
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
_Use_decl_annotations_ extern bool
DirectX::ComputeTangentFrame(
    const uint16_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    const DirectX::XMFLOAT3 *normals,
    const DirectX::XMFLOAT2 *texcoords,
    size_t nVerts,
    DirectX::XMFLOAT3 *tangents,
    DirectX::XMFLOAT3 *bitangents) noexcept
{
    if (!tangents && !bitangents)
    {
        // E_INVALIDARG
        return false;
    }

    return ComputeTangentFrameImpl<uint16_t>(indices, nFaces, positions, normals, texcoords, nVerts, tangents, nullptr, bitangents);
}

//-------------------------------------------------------------------------------------
_Use_decl_annotations_ extern bool
DirectX::ComputeTangentFrame(
    const uint32_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    const DirectX::XMFLOAT3 *normals, const DirectX::XMFLOAT2 *texcoords,
    size_t nVerts,
    DirectX::XMFLOAT3 *tangents,
    DirectX::XMFLOAT3 *bitangents) noexcept
{
    if (!tangents && !bitangents)
    {
        // E_INVALIDARG
        return false;
    }

    return ComputeTangentFrameImpl<uint32_t>(indices, nFaces, positions, normals, texcoords, nVerts, tangents, nullptr, bitangents);
}

//-------------------------------------------------------------------------------------
_Use_decl_annotations_ extern bool
DirectX::ComputeTangentFrame(
    const uint16_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    const DirectX::XMFLOAT3 *normals,
    const DirectX::XMFLOAT2 *texcoords,
    size_t nVerts,
    DirectX::XMFLOAT4 *tangents,
    DirectX::XMFLOAT3 *bitangents) noexcept
{
    if (!tangents && !bitangents)
    {
        // E_INVALIDARG
        return false;
    }

    return ComputeTangentFrameImpl<uint16_t>(indices, nFaces, positions, normals, texcoords, nVerts, nullptr, tangents, bitangents);
}

//-------------------------------------------------------------------------------------
_Use_decl_annotations_ extern bool
DirectX::ComputeTangentFrame(
    const uint32_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    const DirectX::XMFLOAT3 *normals,
    const DirectX::XMFLOAT2 *texcoords,
    size_t nVerts,
    DirectX::XMFLOAT4 *tangents,
    DirectX::XMFLOAT3 *bitangents) noexcept
{
    if (!tangents && !bitangents)
    {
        // E_INVALIDARG
        return false;
    }

    return ComputeTangentFrameImpl<uint32_t>(indices, nFaces, positions, normals, texcoords, nVerts, nullptr, tangents, bitangents);
}

//-------------------------------------------------------------------------------------
_Use_decl_annotations_ extern bool
DirectX::ComputeTangentFrame(
    const uint16_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    const DirectX::XMFLOAT3 *normals,
    const DirectX::XMFLOAT2 *texcoords,
    size_t nVerts,
    DirectX::XMFLOAT4 *tangents) noexcept
{
    if (!tangents)
    {
        // E_INVALIDARG
        return false;
    }

    return ComputeTangentFrameImpl<uint16_t>(indices, nFaces, positions, normals, texcoords, nVerts, nullptr, tangents, nullptr);
}

//-------------------------------------------------------------------------------------
_Use_decl_annotations_ extern bool
DirectX::ComputeTangentFrame(
    const uint32_t *indices,
    size_t nFaces,
    const DirectX::XMFLOAT3 *positions,
    const DirectX::XMFLOAT3 *normals,
    const DirectX::XMFLOAT2 *texcoords,
    size_t nVerts,
    DirectX::XMFLOAT4 *tangents) noexcept
{
    if (!tangents)
    {
        // E_INVALIDARG
        return false;
    }

    return ComputeTangentFrameImpl<uint32_t>(indices, nFaces, positions, normals, texcoords, nVerts, nullptr, tangents, nullptr);
}
