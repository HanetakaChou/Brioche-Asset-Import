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

#ifndef _BRX_ASSET_IMPORT_FILE_INPUT_STREAM_H_
#define _BRX_ASSET_IMPORT_FILE_INPUT_STREAM_H_ 1

#include "../include/brx_asset_import_input_stream.h"

#if defined(__GNUC__)

class brx_asset_import_file_input_stream_factory final : public brx_asset_import_input_stream_factory
{
public:
	brx_asset_import_file_input_stream_factory();
	void init();
	void uninit();
	~brx_asset_import_file_input_stream_factory();

private:
	virtual brx_asset_import_input_stream *create_instance(char const *file_name) override;
	virtual void destory_instance(brx_asset_import_input_stream *input_stream) override;
};

class brx_asset_import_file_input_stream final : public brx_asset_import_input_stream
{
	int m_file;

public:
	brx_asset_import_file_input_stream();
	void init(int file);
	void uninit();
	~brx_asset_import_file_input_stream();

private:
	int stat_size(int64_t *size) override;
	intptr_t read(void *data, size_t size) override;
	int64_t seek(int64_t offset, int whence) override;
};

#elif defined(_MSC_VER)

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <sdkddkver.h>
#include <Windows.h>

class brx_asset_import_file_input_stream_factory final : public brx_asset_import_input_stream_factory
{
public:
	brx_asset_import_file_input_stream_factory();
	void init();
	void uninit();
	~brx_asset_import_file_input_stream_factory();

private:
	virtual brx_asset_import_input_stream *create_instance(char const *file_name) override;
	virtual void destory_instance(brx_asset_import_input_stream *input_stream) override;
};

class brx_asset_import_file_input_stream final : public brx_asset_import_input_stream
{
	HANDLE m_file;

public:
	brx_asset_import_file_input_stream();
	void init(HANDLE file);
	void uninit();
	~brx_asset_import_file_input_stream();
	int stat_size(int64_t *size) override;
	intptr_t read(void *data, size_t size) override;
	int64_t seek(int64_t offset, int whence) override;
};

#else
#error Unknown Compiler
#endif

#endif