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

#include <stddef.h>
#include <stdint.h>

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

#endif
