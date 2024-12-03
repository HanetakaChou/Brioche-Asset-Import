//-------------------------------------------------------------------------------------
// scoped.h
//
// Utility header with helper classes for exception-safe handling of resources
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------

#ifndef _SCOPED_H_
#define _SCOPED_H_ 1

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory>

struct aligned_deleter
{
    void operator()(void *p) noexcept
    {
#if defined(__GNUC__)
        free(p);
#elif defined(_MSC_VER)
        _aligned_free(p);
#else
#error Unknown Compiler
#endif
    }
};

using ScopedAlignedArrayFloat = std::unique_ptr<float[], aligned_deleter>;

static inline ScopedAlignedArrayFloat make_AlignedArrayFloat(uint64_t count)
{
    constexpr uint64_t const alignment = 16;

    // [alignUp](https://github.com/oneapi-src/oneTBB/blob/tbb_2019/src/tbbmalloc/shared_utils.h#L42)
    uint64_t const size = ((((static_cast<uint64_t>(sizeof(float)) * count) - static_cast<uint64_t>(1U)) | (alignment - static_cast<uint64_t>(1U))) + static_cast<uint64_t>(1U));

    if (size > static_cast<uint64_t>(UINT32_MAX))
    {
        return nullptr;
    }

#if defined(__GNUC__)
    auto ptr = aligned_alloc(static_cast<size_t>(alignment), static_cast<size_t>(size));
#elif defined(_MSC_VER)
    auto ptr = _aligned_malloc(static_cast<size_t>(size), static_cast<size_t>(alignment));
#else
#error Unknown Compiler
#endif

    return ScopedAlignedArrayFloat(static_cast<float *>(ptr));
}

using ScopedAlignedArrayXMVECTOR = std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>;

static inline ScopedAlignedArrayXMVECTOR make_AlignedArrayXMVECTOR(uint64_t count)
{
    constexpr uint64_t const alignment = 16;

    uint64_t const size = sizeof(DirectX::XMVECTOR) * count;
    static_assert(0U == (static_cast<uint64_t>(sizeof(DirectX::XMVECTOR)) % alignment), "");

    if (size > static_cast<uint64_t>(UINT32_MAX))
    {
        return nullptr;
    }

#if defined(__GNUC__)
    auto ptr = aligned_alloc(static_cast<size_t>(alignment), static_cast<size_t>(size));
#elif defined(_MSC_VER)
    auto ptr = _aligned_malloc(static_cast<size_t>(size), static_cast<size_t>(alignment));
#else
#error Unknown Compiler
#endif

    return ScopedAlignedArrayXMVECTOR(static_cast<DirectX::XMVECTOR *>(ptr));
}

#endif
