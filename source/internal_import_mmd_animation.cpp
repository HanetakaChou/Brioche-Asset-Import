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

#include "internal_import_mmd_model.h"
#include "internal_mmd_vmd.h"
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

static inline float cubic_bezier(uint8_t const in_packed_k_1_x, uint8_t const in_packed_k_1_y, uint8_t const in_packed_k_2_x, uint8_t const in_packed_k_2_y, float const in_x);

extern bool internal_import_mmd_animation(void const *data_base, size_t data_size, mcrt_vector<brx_asset_import_model_surface_group> &out_surface_groups)
{
    mmd_vmd_t mmd_vmd;
    if (!internal_data_read_mmd_vmd(data_base, data_size, &mmd_vmd))
    {
        return false;
    }

    return true;
}

static inline float cubic_bezier(uint8_t const in_packed_k_1_x, uint8_t const in_packed_k_1_y, uint8_t const in_packed_k_2_x, uint8_t const in_packed_k_2_y, float const in_x)
{
    // https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function/cubic-bezier

    assert(in_packed_k_1_x <= static_cast<uint8_t>(INT8_MAX));
    assert(in_packed_k_1_y <= static_cast<uint8_t>(INT8_MAX));
    assert(in_packed_k_2_x <= static_cast<uint8_t>(INT8_MAX));
    assert(in_packed_k_2_y <= static_cast<uint8_t>(INT8_MAX));

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
                        t = std::min(std::max(0.0F, t2), 1.0F);
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
