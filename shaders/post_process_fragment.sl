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

#include "post_process_pipeline_resource_binding.sli"
#include "../thirdparty/Packed-Vector/shaders/brx_packed_vector.bsli"
#include "../thirdparty/Environment-Lighting/shaders/brx_octahedral_mapping.bsli"
#include "../thirdparty/Brioche-Shader-Language/shaders/brx_math_consts.bsli"

#define INVALID_GBUFFER_DEPTH 0.0

brx_root_signature(post_process_root_signature_macro, post_process_root_signature_name)
brx_early_depth_stencil
brx_pixel_shader_parameter_begin(main)
brx_pixel_shader_parameter_in_frag_coord brx_pixel_shader_parameter_split
brx_pixel_shader_parameter_out(brx_float4, out_color, 0)
brx_pixel_shader_parameter_end(main)
{
    brx_float4 debug_view_color;

    brx_branch 
    if (g_view_index < 0.5)
    {
        // TODO: HDR swapchain
        //
        // DXGI_COLOR_SPACE_TYPE: https://learn.microsoft.com/en-us/windows/win32/direct3darticles/high-dynamic-range#setting-up-your-directx-swap-chain
        // Direct3D 12 HDR sample: https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-hdr-sample-win32/
        // VK_EXT_swapchain_colorspace: https://registry.khronos.org/DataFormat/specs/1.3/dataformat.1.3.html#TRANSFER_CONVERSION
        // https://developer.nvidia.com/high-dynamic-range-display-development
        // https://gpuopen.com/learn/using-amd-freesync-2-hdr-color-spaces/
        //
        // Options       | hardware OETF input | hardware OETF output | Direct3D12                                                                  | Vulkan
        // sRGB          | sRGB                | Bt709                | DXGI_FORMAT_R8G8B8A8_UNORM     + DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709    | VK_FORMAT_B8G8R8A8_UNORM           + VK_COLOR_SPACE_BT709_NONLINEAR_EXT
        // sRGB          | sRGB                | Bt709                | DXGI_FORMAT_R10G10B10A2_UNORM  + DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709    | VK_FORMAT_A2B10G10R10_UNORM_PACK32 + VK_COLOR_SPACE_BT709_NONLINEAR_EXT
        // HDR10         | ST2084              | Bt2020               | DXGI_FORMAT_R10G10B10A2_UNORM  + DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 | VK_FORMAT_A2B10G10R10_UNORM_PACK32 + VK_COLOR_SPACE_HDR10_ST2084_EXT
        // scRGB         | scRGB               | Bt709                | N/A                                                                         | VK_FORMAT_R16G16B16A16_SFLOAT      + VK_COLOR_SPACE_BT709_LINEAR_EXT
        // Linear BT2020 | Linear BT2020       | Bt2020               | DXGI_FORMAT_R16G16B16A16_FLOAT + DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709    | VK_FORMAT_R16G16B16A16_SFLOAT      + VK_COLOR_SPACE_BT2020_LINEAR_EXT

        brx_float4 color_linear = brx_load_2d(g_scene_color_texture, brx_int3(brx_frag_coord.xy, 0));
        brx_float4 color_srgb = brx_float4(brx_pow(brx_clamp(color_linear.x, 0.0, 1.0), (1.0 / 2.2)), brx_pow(brx_clamp(color_linear.y, 0.0, 1.0), (1.0 / 2.2)), brx_pow(brx_clamp(color_linear.z, 0.0, 1.0), (1.0 / 2.2)), color_linear.w);
        debug_view_color = color_srgb;
    }
    else if (g_view_index < 1.5)
    {
        brx_uint4 gbuffer_metarial = brx_load_2d(g_gbuffer_metarial_texture, brx_int3(brx_frag_coord.xy, 0));
        brx_float3 shading_normal_world_space = brx_octahedral_unmap(brx_R16G16_SNORM_to_FLOAT2(gbuffer_metarial.x));
        debug_view_color = brx_float4((shading_normal_world_space + brx_float3(1.0, 1.0, 1.0)) * 0.5, 1.0);
    }
    else if (g_view_index < 2.5)
    {
        brx_uint4 gbuffer_metarial = brx_load_2d(g_gbuffer_metarial_texture, brx_int3(brx_frag_coord.xy, 0));
        brx_float2 roughness_scaled_linear_depth = brx_R16G16_UNORM_to_FLOAT2(gbuffer_metarial.y);
        brx_float roughness = roughness_scaled_linear_depth.x;
        debug_view_color = brx_float4(roughness, roughness, roughness, 1.0);
    }
    else if (g_view_index < 3.5)
    {
        brx_uint4 gbuffer_metarial = brx_load_2d(g_gbuffer_metarial_texture, brx_int3(brx_frag_coord.xy, 0));
        brx_float2 roughness_scaled_linear_depth = brx_R16G16_UNORM_to_FLOAT2(gbuffer_metarial.y);
        brx_float scaled_linear_depth = roughness_scaled_linear_depth.y;
        debug_view_color = brx_float4(scaled_linear_depth, scaled_linear_depth, scaled_linear_depth, 1.0);
    }
    else
    {
        brx_float gbuffer_depth = brx_load_2d(g_gbuffer_depth_texture, brx_int3(brx_frag_coord.xy, 0)).x;
        brx_branch
        if (INVALID_GBUFFER_DEPTH != gbuffer_depth)
        {
            brx_float2 uv = brx_frag_coord.xy / brx_float2(g_screen_width, g_screen_height);

            brx_float surface_position_depth = gbuffer_depth;

            brx_float3 surface_position_ndc_space = brx_float3(uv * brx_float2(2.0, -2.0) + brx_float2(-1.0, 1.0), surface_position_depth);

            brx_float4 surface_position_view_space_with_w = brx_mul(g_inverse_projection_transform, brx_float4(surface_position_ndc_space, 1.0));

            brx_float3 surface_position_view_space = surface_position_view_space_with_w.xyz / surface_position_view_space_with_w.w;

            // w component is always positive no matter whether left-handed or right-handed
            brx_float linear_depth = brx_mul(g_projection_transform, brx_float4(surface_position_view_space, 1.0)).w;

            brx_float scaled_linear_depth = linear_depth * g_linear_depth_scale;

            debug_view_color = brx_float4(scaled_linear_depth, scaled_linear_depth, scaled_linear_depth, 1.0);
        }
        else
        {
            debug_view_color = brx_float4(0.0, 0.0, 0.0, 1.0);
        }
    }
    
    out_color = debug_view_color;
}
