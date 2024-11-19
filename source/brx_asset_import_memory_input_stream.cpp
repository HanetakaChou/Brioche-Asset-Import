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

#include "brx_asset_import_memory_input_stream.h"
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include <new>
#include <cstring>
#include <assert.h>

extern brx_asset_import_input_stream_factory *brx_asset_import_create_memory_input_stream_factory(size_t input_stream_count, char const *const *input_stream_file_names, void const *const *input_stream_memory_range_bases, size_t const *input_stream_memory_range_sizes)
{
	void *new_unwrapped_input_stream_factory_base = mcrt_malloc(sizeof(brx_asset_import_memory_input_stream_factory), alignof(brx_asset_import_memory_input_stream_factory));
	assert(NULL != new_unwrapped_input_stream_factory_base);

	brx_asset_import_memory_input_stream_factory *new_unwrapped_input_stream_factory = new (new_unwrapped_input_stream_factory_base) brx_asset_import_memory_input_stream_factory{};
	new_unwrapped_input_stream_factory->init(input_stream_count, input_stream_file_names, input_stream_memory_range_bases, input_stream_memory_range_sizes);
	return new_unwrapped_input_stream_factory;
}

extern void brx_asset_import_destroy_memory_input_stream_factory(brx_asset_import_input_stream_factory *wrapped_input_stream_factory)
{
	assert(NULL != wrapped_input_stream_factory);
	brx_asset_import_memory_input_stream_factory *delete_unwrapped_input_stream_factory = static_cast<brx_asset_import_memory_input_stream_factory *>(wrapped_input_stream_factory);

	delete_unwrapped_input_stream_factory->uninit();

	delete_unwrapped_input_stream_factory->~brx_asset_import_memory_input_stream_factory();
	mcrt_free(delete_unwrapped_input_stream_factory);
}

brx_asset_import_memory_input_stream_factory::brx_asset_import_memory_input_stream_factory()
{
}

void brx_asset_import_memory_input_stream_factory::init(size_t input_stream_count, char const *const *input_stream_file_names, void const *const *input_stream_memory_range_bases, size_t const *input_stream_memory_range_sizes)
{
	this->m_memory_ranges.reserve(input_stream_count);

	for (size_t input_stream_index = 0U; input_stream_index < input_stream_count; ++input_stream_index)
	{
		mcrt_string input_stream_file_name = input_stream_file_names[input_stream_index];

		import_asset_memory_range input_stream_memory_range = {input_stream_memory_range_bases[input_stream_index], input_stream_memory_range_sizes[input_stream_index]};

		this->m_memory_ranges.emplace_hint(this->m_memory_ranges.end(), std::move(input_stream_file_name), input_stream_memory_range);
	}
}

void brx_asset_import_memory_input_stream_factory::uninit()
{
	this->m_memory_ranges.clear();
}

brx_asset_import_memory_input_stream_factory::~brx_asset_import_memory_input_stream_factory()
{
	assert(this->m_memory_ranges.empty());
}

brx_asset_import_input_stream *brx_asset_import_memory_input_stream_factory::create_instance(char const *file_name)
{
	mcrt_string input_stream_file_name = file_name;

	auto found = this->m_memory_ranges.find(input_stream_file_name);

	if (this->m_memory_ranges.end() != found)
	{
		import_asset_memory_range const &input_stream_memory_range = found->second;

		void *new_unwrapped_input_stream_base = mcrt_malloc(sizeof(brx_asset_import_memory_input_stream), alignof(brx_asset_import_memory_input_stream));
		assert(NULL != new_unwrapped_input_stream_base);

		brx_asset_import_memory_input_stream *new_unwrapped_input_stream = new (new_unwrapped_input_stream_base) brx_asset_import_memory_input_stream{};
		new_unwrapped_input_stream->init(input_stream_memory_range.m_memory_range_base, input_stream_memory_range.m_memory_range_size);
		return new_unwrapped_input_stream;
	}
	else
	{

		return NULL;
	}
}

void brx_asset_import_memory_input_stream_factory::destory_instance(brx_asset_import_input_stream *wrapped_input_stream)
{
	assert(NULL != wrapped_input_stream);
	brx_asset_import_memory_input_stream *delete_unwrapped_input_stream = static_cast<brx_asset_import_memory_input_stream *>(wrapped_input_stream);

	delete_unwrapped_input_stream->uninit();

	delete_unwrapped_input_stream->~brx_asset_import_memory_input_stream();
	mcrt_free(delete_unwrapped_input_stream);
}

brx_asset_import_memory_input_stream::brx_asset_import_memory_input_stream() : m_memory_range_base(NULL), m_memory_range_size(0), m_memory_range_offset(0)
{
}

void brx_asset_import_memory_input_stream::init(void const *memory_range_base, size_t memory_range_size)
{
	assert(NULL == this->m_memory_range_base);
	this->m_memory_range_base = memory_range_base;

	assert(0 == this->m_memory_range_size);
	this->m_memory_range_size = memory_range_size;

	assert(0 == this->m_memory_range_offset);
}

void brx_asset_import_memory_input_stream::uninit()
{
	assert(NULL != this->m_memory_range_base);
	this->m_memory_range_base = NULL;

	assert(0 != this->m_memory_range_size);
	this->m_memory_range_size = 0;

	this->m_memory_range_offset = 0;
}

brx_asset_import_memory_input_stream::~brx_asset_import_memory_input_stream()
{
	assert(NULL == this->m_memory_range_base);
	assert(0 == this->m_memory_range_size);
	assert(0 == this->m_memory_range_offset);
}

int brx_asset_import_memory_input_stream::stat_size(int64_t *size)
{
	(*size) = this->m_memory_range_size;
	return 0;
}

intptr_t brx_asset_import_memory_input_stream::read(void *data, size_t size)
{
	intptr_t size_read = ((this->m_memory_range_offset + static_cast<int64_t>(size)) <= this->m_memory_range_size) ? static_cast<int64_t>(size) : ((this->m_memory_range_offset < this->m_memory_range_size) ? (this->m_memory_range_size - this->m_memory_range_offset) : 0);
	if (size_read > 0)
	{
		std::memcpy(data, reinterpret_cast<void const *>(reinterpret_cast<intptr_t>(this->m_memory_range_base) + this->m_memory_range_offset), size_read);
		this->m_memory_range_offset += size_read;
	}
	return size_read;
}

int64_t brx_asset_import_memory_input_stream::seek(int64_t offset, int whence)
{
	switch (whence)
	{
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_SET:
		this->m_memory_range_offset = offset;
		break;
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_CUR:
		this->m_memory_range_offset += offset;
		break;
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_END:
		this->m_memory_range_offset = this->m_memory_range_size + offset;
		break;
	default:
		this->m_memory_range_offset = -1;
	}

	return this->m_memory_range_offset;
}
