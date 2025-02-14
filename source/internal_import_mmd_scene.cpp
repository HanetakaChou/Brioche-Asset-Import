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

#include "internal_import_mmd_scene.h"
#include "internal_mmd_pmx.h"

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


// TODO: change to static
extern bool internal_import_mmd_model(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_model_surface_group> &out_surface_groups)
{
    mmd_pmx_t mmd_pmx;
    if (!internal_data_read_mmd_pmx(data_base, data_size, &mmd_pmx))
    {
        return false;
    }


    // normalize vertex

    // normalize joint weights

    return true;
}

