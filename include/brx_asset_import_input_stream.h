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

#ifndef _BRX_ASSET_IMPORT_NPUT_STREAM_H_
#define _BRX_ASSET_IMPORT_NPUT_STREAM_H_ 1

#include <cstddef>
#include <cstdint>

class brx_asset_import_input_stream_factory;
class brx_asset_import_input_stream;

enum
{
	BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_SET = 0,
	BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_CUR = 1,
	BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_END = 2
};

extern "C" brx_asset_import_input_stream_factory *brx_asset_import_create_file_input_stream_factory();

extern "C" void brx_asset_import_destroy_file_input_stream_factory(brx_asset_import_input_stream_factory *input_stream_factory);

extern "C" brx_asset_import_input_stream_factory *brx_asset_import_create_memory_input_stream_factory(size_t input_stream_count, char const *const *input_stream_file_names, void const *const *input_stream_memory_range_bases, size_t const *input_stream_memory_range_sizes);

extern "C" void brx_asset_import_destroy_memory_input_stream_factory(brx_asset_import_input_stream_factory *input_stream_factory);

class brx_asset_import_input_stream_factory
{
public:
	virtual brx_asset_import_input_stream *create_instance(char const *file_name) = 0;
	virtual void destroy_instance(brx_asset_import_input_stream *input_stream) = 0;
};

class brx_asset_import_input_stream
{
public:
	virtual int stat_size(int64_t *size) = 0;
	virtual intptr_t read(void *data, size_t size) = 0;
	virtual int64_t seek(int64_t offset, int whence) = 0;
};

#include "brx_asset_import_scene.h"
#include "brx_asset_import_image.h"
#include "../../McRT-Malloc/include/mcrt_vector.h"
#include <cassert>

static inline brx_asset_import_scene *brx_asset_import_create_scene_from_input_stream(brx_asset_import_input_stream_factory *input_stream_factory, char const *input_stream_name)
{
	mcrt_vector<uint8_t> input_stream_data;
	{
		brx_asset_import_input_stream *input_stream;
		if (NULL == (input_stream = input_stream_factory->create_instance(input_stream_name)))
		{
			return NULL;
		}

		int64_t length;
		if (-1 == input_stream->stat_size(&length))
		{
			input_stream_factory->destroy_instance(input_stream);
			return NULL;
		}

		if ((length <= 0) || (length >= static_cast<int64_t>(INTPTR_MAX)))
		{
			input_stream_factory->destroy_instance(input_stream);
			return NULL;
		}

		size_t input_stream_size = static_cast<uint32_t>(length);
		assert(static_cast<int64_t>(input_stream_size) == length);

		input_stream_data.resize(input_stream_size);

		intptr_t read_size = input_stream->read(input_stream_data.data(), input_stream_data.size());

		input_stream_factory->destroy_instance(input_stream);

		if (-1 == read_size || read_size != input_stream_size)
		{
			input_stream_data.clear();
			return NULL;
		}
	}
	assert(!input_stream_data.empty());

	return brx_asset_import_create_scene_from_memory(input_stream_data.data(), input_stream_data.size());
}

static inline brx_asset_import_image *brx_asset_import_create_image_from_input_stream(brx_asset_import_input_stream_factory *input_stream_factory, char const *input_stream_name)
{
	mcrt_vector<uint8_t> input_stream_data;
	{
		brx_asset_import_input_stream *input_stream;
		if (NULL == (input_stream = input_stream_factory->create_instance(input_stream_name)))
		{
			return NULL;
		}

		int64_t length;
		if (-1 == input_stream->stat_size(&length))
		{
			input_stream_factory->destroy_instance(input_stream);
			return NULL;
		}

		if ((length <= 0) || (length >= static_cast<int64_t>(static_cast<int64_t>(1) << static_cast<int64_t>(24))))
		{
			input_stream_factory->destroy_instance(input_stream);
			return NULL;
		}

		size_t input_stream_size = static_cast<uint32_t>(length);
		assert(static_cast<int64_t>(input_stream_size) == length);

		input_stream_data.resize(input_stream_size);

		intptr_t read_size = input_stream->read(input_stream_data.data(), input_stream_data.size());

		input_stream_factory->destroy_instance(input_stream);

		if (-1 == read_size || read_size != input_stream_size)
		{
			input_stream_data.clear();
			return NULL;
		}
	}
	assert(!input_stream_data.empty());

	return brx_asset_import_create_image_from_memory(input_stream_data.data(), input_stream_data.size());
}

#endif
