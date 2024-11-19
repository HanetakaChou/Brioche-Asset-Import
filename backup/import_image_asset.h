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

#ifndef _IMPORT_IMAGE_ASSET_H_
#define _IMPORT_IMAGE_ASSET_H_ 1

#include <stddef.h>
#include <stdint.h>
#include "brx_asset_import_input_stream.h"
#include "../../Brioche/include/brx_sampled_asset_image_format.h"

enum IMPORT_ASSET_IMAGE_TYPE
{
    IMPORT_ASSET_IMAGE_TYPE_1D = 1,
    IMPORT_ASSET_IMAGE_TYPE_2D = 2,
    IMPORT_ASSET_IMAGE_TYPE_3D = 3
};

struct IMPORT_ASSET_IMAGE_HEADER
{
    bool is_cube_map;
    IMPORT_ASSET_IMAGE_TYPE type;
    BRX_SAMPLED_ASSET_IMAGE_FORMAT format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_layers;
};

extern bool import_dds_image_asset_header_from_input_stream(brx_asset_import_input_stream *input_stream, IMPORT_ASSET_IMAGE_HEADER *image_asset_header, size_t *image_asset_data_offset);

extern bool import_dds_image_asset_data_from_input_stream(brx_asset_import_input_stream *input_stream, IMPORT_ASSET_IMAGE_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_SAMPLED_ASSET_IMAGE_IMPORT_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests);

extern bool import_pvr_image_asset_header_from_input_stream(brx_asset_import_input_stream *input_stream, IMPORT_ASSET_IMAGE_HEADER *image_asset_header, size_t *image_asset_data_offset);

extern bool import_pvr_image_asset_data_from_input_stream(brx_asset_import_input_stream *input_stream, IMPORT_ASSET_IMAGE_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_SAMPLED_ASSET_IMAGE_IMPORT_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests);

#endif
