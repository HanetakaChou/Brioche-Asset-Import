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

#if defined(__GNUC__)
#elif defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS 1
#else
#error Unknown Compiler
#endif
#define CGLTF_IMPLEMENTATION
#include "../thirdparty/cgltf/cgltf.h"
#include "../include/brx_asset_import_input_stream.h"
#include "../../McRT-Malloc/include/mcrt_malloc.h"

extern cgltf_result cgltf_custom_read_file(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *file_options, const char *path, cgltf_size *size, void **data)
{
    void *(*const memory_alloc)(void *, cgltf_size) = memory_options->alloc_func;
    assert(NULL != memory_alloc);

    void (*const memory_free)(void *, void *) = memory_options->free_func;
    assert(NULL != memory_free);

    brx_asset_import_input_stream_factory *const input_stream_factory = static_cast<brx_asset_import_input_stream_factory *>(file_options->user_data);

    brx_asset_import_input_stream *file;
    if (NULL == (file = input_stream_factory->create_instance(path)))
    {
        return cgltf_result_file_not_found;
    }

    cgltf_size file_size = (NULL != size) ? (*size) : 0;
    if (file_size == 0)
    {
        int64_t length;
        if (-1 == file->stat_size(&length))
        {
            input_stream_factory->destroy_instance(file);
            return cgltf_result_io_error;
        }

        file_size = length;
    }

    void *file_data = memory_alloc(memory_options->user_data, file_size);
    if (NULL == file_data)
    {
        input_stream_factory->destroy_instance(file);
        return cgltf_result_out_of_memory;
    }

    intptr_t read_size = file->read(file_data, file_size);

    input_stream_factory->destroy_instance(file);

    if (-1 == read_size || read_size != file_size)
    {
        memory_free(memory_options->user_data, file_data);
        return cgltf_result_io_error;
    }

    if (NULL != size)
    {
        *size = file_size;
    }
    if (NULL != data)
    {
        *data = file_data;
    }

    return cgltf_result_success;
}

extern void cgltf_custom_file_release(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *, void *data)
{
    void (*const memory_free)(void *, void *) = memory_options->free_func;
    assert(NULL != memory_free);

    memory_free(memory_options->user_data, data);
}

extern void *cgltf_custom_alloc(void *, cgltf_size size)
{
    return mcrt_malloc(size, alignof(size_t));
}

extern void cgltf_custom_free(void *, void *ptr)
{
    mcrt_free(ptr);
}
