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

#include "brx_asset_import_file_input_stream.h"
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include <new>
#include <assert.h>

#if defined(__GNUC__)

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#include <arm_acle.h>
#else
#error Unknown Architecture
#endif

extern "C" brx_asset_import_input_stream_factory *brx_asset_import_create_file_input_stream_factory()
{
	void *new_unwrapped_input_stream_factory_base = mcrt_malloc(sizeof(brx_asset_import_file_input_stream_factory), alignof(brx_asset_import_file_input_stream_factory));
	assert(NULL != new_unwrapped_input_stream_factory_base);

	brx_asset_import_file_input_stream_factory *new_unwrapped_input_stream_factory = new (new_unwrapped_input_stream_factory_base) brx_asset_import_file_input_stream_factory{};
	new_unwrapped_input_stream_factory->init();
	return new_unwrapped_input_stream_factory;
}

extern "C" void brx_asset_import_destroy_file_input_stream_factory(brx_asset_import_input_stream_factory *wrapped_input_stream_factory)
{
	assert(NULL != wrapped_input_stream_factory);
	brx_asset_import_file_input_stream_factory *delete_unwrapped_input_stream_factory = static_cast<brx_asset_import_file_input_stream_factory *>(wrapped_input_stream_factory);

	delete_unwrapped_input_stream_factory->uninit();

	delete_unwrapped_input_stream_factory->~brx_asset_import_file_input_stream_factory();
	mcrt_free(delete_unwrapped_input_stream_factory);
}

brx_asset_import_file_input_stream_factory::brx_asset_import_file_input_stream_factory()
{
}

void brx_asset_import_file_input_stream_factory::init()
{
}

void brx_asset_import_file_input_stream_factory::uninit()
{
}

brx_asset_import_file_input_stream_factory::~brx_asset_import_file_input_stream_factory()
{
}

brx_asset_import_input_stream *brx_asset_import_file_input_stream_factory::create_instance(char const *file_name)
{
	// NOTE: we can not share the "fd" between different instances, since the results of "lseek" can NOT be shared.

	int file = openat(AT_FDCWD, file_name, O_RDONLY);
	if (-1 != file)
	{
		void *new_unwrapped_input_stream_base = mcrt_malloc(sizeof(brx_asset_import_file_input_stream), alignof(brx_asset_import_file_input_stream));
		assert(NULL != new_unwrapped_input_stream_base);

		brx_asset_import_file_input_stream *new_unwrapped_input_stream = new (new_unwrapped_input_stream_base) brx_asset_import_file_input_stream{};
		new_unwrapped_input_stream->init(file);
		return new_unwrapped_input_stream;
	}
	else
	{
		return NULL;
	}
}

void brx_asset_import_file_input_stream_factory::destroy_instance(brx_asset_import_input_stream *wrapped_input_stream)
{
	assert(NULL != wrapped_input_stream);
	brx_asset_import_file_input_stream *delete_unwrapped_input_stream = static_cast<brx_asset_import_file_input_stream *>(wrapped_input_stream);

	delete_unwrapped_input_stream->uninit();

	delete_unwrapped_input_stream->~brx_asset_import_file_input_stream();
	mcrt_free(delete_unwrapped_input_stream);
}

brx_asset_import_file_input_stream::brx_asset_import_file_input_stream() : m_file(-1)
{
}

void brx_asset_import_file_input_stream::init(int file)
{
	assert(-1 == this->m_file);

	this->m_file = file;
}

void brx_asset_import_file_input_stream::uninit()
{
	assert(-1 != this->m_file);

	int res_close = close(this->m_file);
	assert(0 == res_close);

	this->m_file = -1;
}

brx_asset_import_file_input_stream::~brx_asset_import_file_input_stream()
{
	assert(-1 == this->m_file);
}

int brx_asset_import_file_input_stream::stat_size(int64_t *size)
{
	assert(-1 != this->m_file);

	struct stat buf;
	int res_fstat = fstat(this->m_file, &buf);

	(*size) = static_cast<int64_t>(buf.st_size);

	return res_fstat;
}

intptr_t brx_asset_import_file_input_stream::read(void *data, size_t size)
{
	assert(-1 != this->m_file);

	ssize_t res_read;
	while ((-1 == (res_read = ::read(this->m_file, data, size))) && (EINTR == errno))
	{
#if defined(__x86_64__) || defined(__i386__)
		_mm_pause();
#elif defined(__aarch64__) || defined(__arm__)
		__yield();
#else
#error Unknown Architecture
#endif
	}

	return res_read;
}

int64_t brx_asset_import_file_input_stream::seek(int64_t offset, int wrapped_whence)
{
	assert(-1 != this->m_file);

	int unwrapped_whence;
	switch (wrapped_whence)
	{
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_SET:
	{
		unwrapped_whence = SEEK_SET;
	}
	break;
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_CUR:
	{
		unwrapped_whence = SEEK_CUR;
	}
	break;
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_END:
	{
		unwrapped_whence = SEEK_END;
	}
	break;
	default:
		assert(false);
		unwrapped_whence = -1;
	}

	off_t res_lseek = lseek(this->m_file, offset, unwrapped_whence);
	return res_lseek;
}

#elif defined(_MSC_VER)

#include "../../libiconv/include/iconv.h"
#include "../../McRT-Malloc/include/mcrt_string.h"

extern "C" brx_asset_import_input_stream_factory *brx_asset_import_create_file_input_stream_factory()
{
	void *new_unwrapped_input_stream_factory_base = mcrt_malloc(sizeof(brx_asset_import_file_input_stream_factory), alignof(brx_asset_import_file_input_stream_factory));
	assert(NULL != new_unwrapped_input_stream_factory_base);

	brx_asset_import_file_input_stream_factory *new_unwrapped_input_stream_factory = new (new_unwrapped_input_stream_factory_base) brx_asset_import_file_input_stream_factory{};
	new_unwrapped_input_stream_factory->init();
	return new_unwrapped_input_stream_factory;
}

extern "C" void brx_asset_import_destroy_file_input_stream_factory(brx_asset_import_input_stream_factory *wrapped_input_stream_factory)
{
	assert(NULL != wrapped_input_stream_factory);
	brx_asset_import_file_input_stream_factory *delete_unwrapped_input_stream_factory = static_cast<brx_asset_import_file_input_stream_factory *>(wrapped_input_stream_factory);

	delete_unwrapped_input_stream_factory->uninit();

	delete_unwrapped_input_stream_factory->~brx_asset_import_file_input_stream_factory();
	mcrt_free(delete_unwrapped_input_stream_factory);
}

brx_asset_import_file_input_stream_factory::brx_asset_import_file_input_stream_factory()
{
}

void brx_asset_import_file_input_stream_factory::init()
{
}

void brx_asset_import_file_input_stream_factory::uninit()
{
}

brx_asset_import_file_input_stream_factory::~brx_asset_import_file_input_stream_factory()
{
}

brx_asset_import_input_stream *brx_asset_import_file_input_stream_factory::create_instance(char const *file_name)
{
	// NOTE: we can not share the "fd" between different instances, since the results of "lseek" can NOT be shared.

	mcrt_wstring file_name_utf16;
	{
		assert(file_name_utf16.empty());

		mcrt_string src_utf8 = file_name;
		mcrt_wstring &dst_utf16 = file_name_utf16;

		assert(dst_utf16.empty());

		if (!src_utf8.empty())
		{
			dst_utf16.resize(src_utf8.size() + 1U);

			size_t in_bytes_left = sizeof(src_utf8[0]) * src_utf8.size();
			size_t out_bytes_left = sizeof(dst_utf16[0]) * dst_utf16.size();
			char *in_buf = src_utf8.data();
			char *out_buf = reinterpret_cast<char *>(dst_utf16.data());

			iconv_t conversion_descriptor = iconv_open("UTF-16LE", "UTF-8");
			assert(((iconv_t)(-1)) != conversion_descriptor);

			size_t conversion_result = iconv(conversion_descriptor, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
			assert(((size_t)(-1)) != conversion_result);

			int result = iconv_close(conversion_descriptor);
			assert(-1 != result);

			dst_utf16.resize(reinterpret_cast<decltype(&dst_utf16[0])>(out_buf) - dst_utf16.data());
		}
	}

	HANDLE file = CreateFileW(file_name_utf16.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE != file)
	{
		void *new_unwrapped_input_stream_base = mcrt_malloc(sizeof(brx_asset_import_file_input_stream), alignof(brx_asset_import_file_input_stream));
		assert(NULL != new_unwrapped_input_stream_base);

		brx_asset_import_file_input_stream *new_unwrapped_input_stream = new (new_unwrapped_input_stream_base) brx_asset_import_file_input_stream{};
		new_unwrapped_input_stream->init(file);
		return new_unwrapped_input_stream;
	}
	else
	{
		return NULL;
	}
}

void brx_asset_import_file_input_stream_factory::destroy_instance(brx_asset_import_input_stream *wrapped_input_stream)
{
	assert(NULL != wrapped_input_stream);
	brx_asset_import_file_input_stream *delete_unwrapped_input_stream = static_cast<brx_asset_import_file_input_stream *>(wrapped_input_stream);

	delete_unwrapped_input_stream->uninit();

	delete_unwrapped_input_stream->~brx_asset_import_file_input_stream();
	mcrt_free(delete_unwrapped_input_stream);
}

brx_asset_import_file_input_stream::brx_asset_import_file_input_stream() : m_file(INVALID_HANDLE_VALUE)
{
}

void brx_asset_import_file_input_stream::init(HANDLE file)
{
	assert(INVALID_HANDLE_VALUE == this->m_file);

	this->m_file = file;
}

void brx_asset_import_file_input_stream::uninit()
{
	assert(INVALID_HANDLE_VALUE != this->m_file);

	BOOL res_close_handle = CloseHandle(this->m_file);
	assert(FALSE != res_close_handle);

	this->m_file = INVALID_HANDLE_VALUE;
}

brx_asset_import_file_input_stream::~brx_asset_import_file_input_stream()
{
	assert(INVALID_HANDLE_VALUE == this->m_file);
}

int brx_asset_import_file_input_stream::stat_size(int64_t *size)
{
	assert(INVALID_HANDLE_VALUE != this->m_file);

	LARGE_INTEGER length;
	BOOL res_get_file_size_ex = GetFileSizeEx(this->m_file, &length);

	(*size) = static_cast<int64_t>(length.QuadPart);

	return ((FALSE != res_get_file_size_ex) ? (0) : (-1));
}

intptr_t brx_asset_import_file_input_stream::read(void *data, size_t size)
{
	assert(INVALID_HANDLE_VALUE != this->m_file);

	DWORD read_size;
	BOOL res_read_file = ReadFile(this->m_file, data, static_cast<DWORD>(size), &read_size, NULL);

	return (FALSE != res_read_file) ? (static_cast<intptr_t>(read_size)) : (-1);
}

int64_t brx_asset_import_file_input_stream::seek(int64_t offset, int whence)
{
	assert(INVALID_HANDLE_VALUE != this->m_file);

	DWORD move_method;
	switch (whence)
	{
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_SET:
	{
		move_method = FILE_BEGIN;
	}
	break;
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_CUR:
	{
		move_method = FILE_CURRENT;
	}
	break;
	case BRX_ASSET_IMPORT_INPUT_STREAM_SEEK_END:
	{
		move_method = FILE_END;
	}
	break;
	default:
		assert(false);
		move_method = -1;
	}

	LARGE_INTEGER distance_to_move;
	distance_to_move.QuadPart = static_cast<LONGLONG>(offset);

	LARGE_INTEGER new_file_pointer;
	BOOL res_set_file_pointer_ex = SetFilePointerEx(this->m_file, distance_to_move, &new_file_pointer, move_method);
	return (FALSE != res_set_file_pointer_ex) ? (static_cast<int64_t>(new_file_pointer.QuadPart)) : (-1);
}

#else
#error Unknown Compiler
#endif