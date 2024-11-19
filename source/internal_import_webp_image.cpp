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

#include "internal_import_png_image.h"
#include "internal_import_image.h"
#include <cstring>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include "../thirdparty/libwebp/src/webp/decode.h"

extern bool internal_import_webp_image(void const *data_base, size_t data_size, mcrt_vector<uint32_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
	int width = -1;
	int height = -1;
	uint8_t *rgba_data = WebPDecodeRGBA(static_cast<uint8_t const *>(data_base), data_size, &width, &height);
	if (NULL != rgba_data)
	{
		if ((width >= 1) && (width <= k_max_image_width_or_height) && (height >= 1) && (height < k_max_image_width_or_height))
		{
			out_pixel_data.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
			std::memcpy(out_pixel_data.data(), rgba_data, sizeof(uint32_t) * static_cast<size_t>(width) * static_cast<size_t>(height));
			WebPFree(rgba_data);
			(*out_width) = width;
			(*out_height) = height;
			return true;
		}
		else
		{
			WebPFree(rgba_data);
			return false;
		}
	}
	else
	{
		return false;
	}
}