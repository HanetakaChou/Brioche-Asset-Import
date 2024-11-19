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

#include "internal_import_exr_image.h"
#include "internal_import_image.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include "../../McRT-Malloc/include/mcrt_malloc.h"
#include "../../McRT-Malloc/include/mcrt_parallel_reduce.h"
#include "../thirdparty/OpenEXR/src/lib/OpenEXRCore/openexr.h"

static void _internal_exr_error_handler(exr_const_context_t context, exr_result_t code, const char *message);

static void *_internal_exr_alloc(size_t bytes);

static void _internal_exr_free(void *ptr);

static int64_t _internal_exr_read_func(exr_const_context_t context, void *user_data, void *buffer, uint64_t size, uint64_t offset, exr_stream_error_func_ptr_t error_callback);

static int64_t _internal_exr_query_size_func(exr_const_context_t ctxt, void *user_data);

static int64_t _internal_exr_write_func(exr_const_context_t context, void *user_data, void const *buffer, uint64_t size, uint64_t offset, exr_stream_error_func_ptr_t error_callback);

static void _internal_exr_shutdown(exr_const_context_t context, void *user_data, int failed);

struct _internal_exr_file_handle
{
    void const *m_data_base;
    size_t m_data_size;
};

extern bool internal_import_exr_image(void const *data_base, size_t data_size, mcrt_vector<uint64_t> &out_pixel_data, uint32_t *out_width, uint32_t *out_height)
{
    // https://github.com/ImageMagick/ImageMagick/blob/main/coders/exr.c

    _internal_exr_file_handle const user_data = {data_base, data_size};

    exr_context_t context = NULL;
    {
        exr_context_initializer_t context_initializer = EXR_DEFAULT_CONTEXT_INITIALIZER;
        context_initializer.error_handler_fn = _internal_exr_error_handler;
        context_initializer.alloc_fn = _internal_exr_alloc;
        context_initializer.free_fn = _internal_exr_free;
        context_initializer.user_data = const_cast<_internal_exr_file_handle *>(&user_data);
        context_initializer.read_fn = _internal_exr_read_func;
        context_initializer.size_fn = _internal_exr_query_size_func;
        context_initializer.write_fn = _internal_exr_write_func;
        context_initializer.destroy_fn = _internal_exr_shutdown;
        exr_result_t const result_exr_start_read = exr_start_read(&context, "", &context_initializer);
        if (EXR_ERR_SUCCESS != result_exr_start_read)
        {
            return false;
        }
    }
    assert(NULL != context);

    bool has_error = false;
    try
    {
        constexpr int const k_default_part_index = 0;

        bool from_yca = false;
        {
            // RgbaInputFile::RgbaInputFile
            exr_attr_chlist_t const *channel_list = NULL;
            exr_result_t const result_exr_get_channels = exr_get_channels(context, k_default_part_index, &channel_list);
            if (EXR_ERR_SUCCESS != result_exr_get_channels)
            {
                throw std::runtime_error{"exr get channels"};
            }

            // ImfRgbaFile.cpp: rgbaChannels
            for (int channel_index = 0; channel_index < channel_list->num_channels; ++channel_index)
            {
                exr_attr_chlist_entry_t const *channel = channel_list->entries + channel_index;
                if (((2 == channel->name.length) && ('R' == channel->name.str[0]) && ('Y' == channel->name.str[0])) || ((2 == channel->name.length) && ('B' == channel->name.str[0]) && ('Y' == channel->name.str[0])))
                {
                    from_yca = true;
                    break;
                }
            }
        }

        if (from_yca)
        {
            throw std::runtime_error{"TODO From YCA"};
        }

        exr_attr_box2i_t data_window;
        exr_result_t const result_exr_get_data_window = exr_get_data_window(context, k_default_part_index, &data_window);
        if (EXR_ERR_SUCCESS != result_exr_get_data_window)
        {
            throw std::runtime_error{"exr get data window"};
        }

        int32_t const width = (data_window.max.x - data_window.min.x) + 1;
        int32_t const height = (data_window.max.y - data_window.min.y) + 1;

        if ((width > k_max_image_width_or_height) || (height > k_max_image_width_or_height))
        {
            throw std::runtime_error("Size Overflow");
        }

        if ((width < 1) || (height < 1))
        {
            throw std::runtime_error("Size Zero");
        }

        constexpr uint64_t const _uint64_x_stride = static_cast<uint64_t>(k_illumiant_image_channel_size) * static_cast<uint64_t>(k_illumiant_image_num_channels);
        uint64_t const _uint64_y_stride = static_cast<uint64_t>(k_illumiant_image_channel_size) * static_cast<uint64_t>(k_illumiant_image_num_channels) * static_cast<uint64_t>(width);

        constexpr size_t const x_stride = static_cast<uintptr_t>(_uint64_x_stride);
        size_t const y_stride = static_cast<size_t>(_uint64_y_stride);

        uint64_t const _uint64_num_pixels = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
        size_t const num_pixels = static_cast<size_t>(_uint64_num_pixels);

        static_assert(sizeof(out_pixel_data[0]) == x_stride, "");

        if (!((x_stride == _uint64_x_stride) && (y_stride == _uint64_y_stride) && (num_pixels == _uint64_num_pixels)))
        {
            throw std::runtime_error("Size Overflow");
        }

        out_pixel_data.resize(num_pixels, static_cast<uint64_t>(0U));

        // ImfInputFile.cpp: InputFile::initialize
        exr_storage_t storage = EXR_STORAGE_LAST_TYPE;
        exr_result_t const result_exr_get_storage = exr_get_storage(context, k_default_part_index, &storage);
        if (EXR_ERR_SUCCESS != result_exr_get_storage)
        {
            throw std::runtime_error{"exr get storage"};
        }

        if (EXR_STORAGE_SCANLINE == storage)
        {
            // ImfScanLineInputFile.cpp: ScanLineInputFile::Data::readPixels

            struct _internal_exr_line_buffer_task
            {
                exr_chunk_info_t m_chunk;
                int32_t m_y;
                int32_t m_last_y;
            };

            mcrt_vector<_internal_exr_line_buffer_task> tasks;
            {
                int32_t scanlines_per_chunk = 1;
                exr_result_t const result_exr_get_scanlines_per_chunk = exr_get_scanlines_per_chunk(context, k_default_part_index, &scanlines_per_chunk);
                if (EXR_ERR_SUCCESS != result_exr_get_scanlines_per_chunk)
                {
                    throw std::runtime_error{"exr get scanlines per chunk"};
                }

                int y = data_window.min.y;
                while (y <= data_window.max.y)
                {
                    exr_chunk_info_t chunk;
                    exr_result_t const result_exr_read_scanline_chunk_info = exr_read_scanline_chunk_info(context, k_default_part_index, y, &chunk);
                    if (EXR_ERR_SUCCESS != result_exr_read_scanline_chunk_info)
                    {
                        throw std::runtime_error{"exr read scanline chunk info"};
                    }

                    tasks.push_back(_internal_exr_line_buffer_task{chunk, y, data_window.max.y});

                    assert(chunk.start_y == y);
                    y += (scanlines_per_chunk - (y - chunk.start_y));
                }

                assert(tasks.size() == (((data_window.max.y - data_window.min.y) / scanlines_per_chunk) + 1));
            }
            assert(tasks.size() <= static_cast<size_t>(UINT32_MAX));
            uint32_t const num_task = static_cast<uint32_t>(tasks.size());

            struct _internal_exr_line_buffer_task_context
            {
                mcrt_vector<_internal_exr_line_buffer_task> m_tasks;
                exr_context_t m_context;
                exr_attr_box2i_t m_data_window;
                int32_t m_width;
                size_t m_x_stride;
                size_t m_y_stride;
                mcrt_vector<uint64_t> *m_out_pixel_data;
            };

            _internal_exr_line_buffer_task_context const task_context{std::move(tasks), context, data_window, width, x_stride, y_stride, &out_pixel_data};
            assert(task_context.m_tasks.size() == num_task);

            int32_t const result_exr_decode = mcrt_parallel_reduce_int(
                0U,
                num_task,
                1U,
                [](uint32_t begin, uint32_t end, void *user_data) -> int32_t
                {
                    _internal_exr_line_buffer_task_context const *const task_context = static_cast<_internal_exr_line_buffer_task_context *>(user_data);
                    assert(NULL != task_context);
                    assert((begin + 1U) == end);
                    assert(begin < task_context->m_tasks.size());
                    _internal_exr_line_buffer_task const &task = task_context->m_tasks[begin];
                    exr_context_t const context = task_context->m_context;
                    exr_attr_box2i_t const &data_window = task_context->m_data_window;
                    int32_t const width = task_context->m_width;
                    size_t const x_stride = task_context->m_x_stride;
                    size_t const y_stride = task_context->m_y_stride;
                    assert(NULL != task_context->m_out_pixel_data);
                    mcrt_vector<uint64_t> &out_pixel_data = (*task_context->m_out_pixel_data);

                    exr_decode_pipeline_t decoder;
                    exr_result_t const result_exr_decoding_initialize = exr_decoding_initialize(context, k_default_part_index, &task.m_chunk, &decoder);
                    if (EXR_ERR_SUCCESS != result_exr_decoding_initialize)
                    {
                        return 1;
                    }

                    bool has_decode_error = false;
                    try
                    {
                        {
                            decoder.user_line_begin_skip = task.m_y - task.m_chunk.start_y;
                            int64_t const end_y = static_cast<int64_t>(task.m_chunk.start_y) + static_cast<int64_t>(task.m_chunk.height) - 1;
                            if (static_cast<int64_t>(task.m_last_y) < end_y)
                            {
                                decoder.user_line_end_ignore = static_cast<int32_t>(end_y - task.m_last_y);
                            }
                            else
                            {
                                decoder.user_line_end_ignore = 0;
                            }

                            for (int16_t channel_index = 0; channel_index < decoder.channel_count; ++channel_index)
                            {
                                exr_coding_channel_info_t *channel = decoder.channels + channel_index;

                                if (channel->height > 0)
                                {
                                    void *slice_base;
                                    if (('R' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = out_pixel_data.data() - width * data_window.min.y - data_window.min.x;
                                    }
                                    else if (('G' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out_pixel_data.data() - width * data_window.min.y - data_window.min.x) + k_illumiant_image_channel_size * 1U);
                                    }
                                    else if (('B' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out_pixel_data.data() - width * data_window.min.y - data_window.min.x) + k_illumiant_image_channel_size * 2U);
                                    }
                                    else if (('A' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out_pixel_data.data() - width * data_window.min.y - data_window.min.x) + k_illumiant_image_channel_size * 3U);
                                    }
                                    else
                                    {
                                        assert(false);
                                        slice_base = NULL;
                                    }

                                    if (NULL != slice_base)
                                    {
                                        channel->user_bytes_per_element = k_illumiant_image_channel_size;
                                        channel->user_data_type = EXR_PIXEL_HALF;
                                        channel->user_pixel_stride = x_stride;
                                        channel->user_line_stride = y_stride;

                                        assert(task.m_chunk.start_x >= 0);

                                        uint64_t const _uint64_ptr = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(slice_base)) + (static_cast<uint64_t>(y_stride) * static_cast<uint64_t>(task.m_y) + (static_cast<uint64_t>(x_stride) * static_cast<uint64_t>(task.m_chunk.start_x)));
                                        uintptr_t const ptr = static_cast<uintptr_t>(_uint64_ptr);
                                        if (!(_uint64_ptr == ptr))
                                        {
                                            throw std::runtime_error("Size Overflow");
                                        }
                                        channel->decode_to_ptr = reinterpret_cast<uint8_t *>(ptr);
                                    }
                                    else
                                    {
                                        assert(false);
                                        channel->user_pixel_stride = 0;
                                        channel->user_line_stride = 0;
                                        channel->decode_to_ptr = NULL;
                                    }
                                }
                                else
                                {
                                    assert(false);
                                    channel->user_pixel_stride = 0;
                                    channel->user_line_stride = 0;
                                    channel->decode_to_ptr = NULL;
                                }
                            }
                        }

                        exr_result_t const result_exr_decoding_choose_default_routines = exr_decoding_choose_default_routines(context, k_default_part_index, &decoder);
                        if (EXR_ERR_SUCCESS != result_exr_decoding_choose_default_routines)
                        {
                            throw std::runtime_error{"exr decoding choose default routines"};
                        }

                        exr_result_t const result_exr_decoding_run = exr_decoding_run(context, k_default_part_index, &decoder);
                        if (EXR_ERR_SUCCESS != result_exr_decoding_run)
                        {
                            throw std::runtime_error{"exr decoding run"};
                        }
                    }
                    catch (std::runtime_error exception)
                    {
                        std::cout << exception.what() << std::endl;
                        has_decode_error = true;
                    }

                    exr_result_t const result_exr_decoding_destroy = exr_decoding_destroy(context, &decoder);
                    assert(EXR_ERR_SUCCESS == result_exr_decoding_destroy);

                    return ((!has_decode_error) ? 0 : 1);
                },
                const_cast<_internal_exr_line_buffer_task_context *>(&task_context));

            if (0 != result_exr_decode)
            {
                throw std::runtime_error{"exr decode"};
            }

            (*out_width) = width;
            (*out_height) = height;
        }
        else if (EXR_STORAGE_DEEP_SCANLINE == storage)
        {
            // ImfCompositeDeepScanLine.cpp: CompositeDeepScanLine::readPixels
            // ImfDeepScanLineInputFile.cpp: DeepScanLineInputFile::Data::readData

            throw std::runtime_error{"TODO Deep Scanline"};
        }
        else if (EXR_STORAGE_TILED == storage)
        {
            // ImfTiledInputFile.cpp: TiledInputFile::Data::readTiles

            constexpr int const k_default_level = 0;

            struct _internal_exr_tile_buffer_task
            {
                exr_chunk_info_t m_chunk;
            };

            mcrt_vector<_internal_exr_tile_buffer_task> tasks;
            {
                int32_t tile_count_x;
                int32_t tile_count_y;
                exr_result_t const result_exr_get_tile_counts = exr_get_tile_counts(context, k_default_part_index, 0, 0, &tile_count_x, &tile_count_y);
                if (EXR_ERR_SUCCESS != result_exr_get_tile_counts)
                {
                    throw std::runtime_error{"exr get tile counts"};
                }

                for (int32_t tile_y = 0; tile_y < tile_count_y; ++tile_y)
                {
                    for (int32_t tile_x = 0; tile_x < tile_count_x; ++tile_x)
                    {
                        exr_chunk_info_t chunk;
                        exr_result_t const result_exr_read_tile_chunk_info = exr_read_tile_chunk_info(context, k_default_part_index, tile_x, tile_y, k_default_level, k_default_level, &chunk);
                        if (EXR_ERR_SUCCESS != result_exr_read_tile_chunk_info)
                        {
                            throw std::runtime_error{"exr read tile chunk info"};
                        }

                        tasks.push_back(_internal_exr_tile_buffer_task{chunk});
                    }
                }

                assert(tasks.size() == (static_cast<size_t>(tile_count_x) * static_cast<size_t>(tile_count_y)));
            }

            assert(tasks.size() <= static_cast<size_t>(UINT32_MAX));
            uint32_t const num_task = static_cast<uint32_t>(tasks.size());

            struct _internal_exr_tile_buffer_task_context
            {
                mcrt_vector<_internal_exr_tile_buffer_task> m_tasks;
                exr_context_t m_context;
                exr_attr_box2i_t m_data_window;
                int32_t m_width;
                size_t m_x_stride;
                size_t m_y_stride;
                mcrt_vector<uint64_t> *m_out_pixel_data;
            };

            _internal_exr_tile_buffer_task_context const task_context{std::move(tasks), context, data_window, width, x_stride, y_stride, &out_pixel_data};
            assert(task_context.m_tasks.size() == num_task);

            int32_t const result_exr_decode = mcrt_parallel_reduce_int(
                0U,
                num_task,
                1U,
                [](uint32_t begin, uint32_t end, void *user_data) -> int32_t
                {
                    _internal_exr_tile_buffer_task_context const *const task_context = static_cast<_internal_exr_tile_buffer_task_context *>(user_data);
                    assert(NULL != task_context);
                    assert((begin + 1U) == end);
                    assert(begin < task_context->m_tasks.size());
                    _internal_exr_tile_buffer_task const &task = task_context->m_tasks[begin];
                    exr_context_t const context = task_context->m_context;
                    exr_attr_box2i_t const &data_window = task_context->m_data_window;
                    int32_t const width = task_context->m_width;
                    size_t const x_stride = task_context->m_x_stride;
                    size_t const y_stride = task_context->m_y_stride;
                    assert(NULL != task_context->m_out_pixel_data);
                    mcrt_vector<uint64_t> &out_pixel_data = (*task_context->m_out_pixel_data);

                    int32_t tile_size_x;
                    int32_t tile_size_y;
                    assert(k_default_level == task.m_chunk.level_x);
                    assert(k_default_level == task.m_chunk.level_y);
                    exr_result_t const result_exr_get_tile_sizes = exr_get_tile_sizes(context, k_default_part_index, task.m_chunk.level_x, task.m_chunk.level_y, &tile_size_x, &tile_size_y);
                    if (EXR_ERR_SUCCESS != result_exr_get_tile_sizes)
                    {
                        assert(false);
                        return 1;
                    }

                    int32_t const tile_absolute_x = data_window.min.x + tile_size_x * task.m_chunk.start_x;
                    int32_t const tile_absolute_y = data_window.min.y + tile_size_y * task.m_chunk.start_y;

                    exr_decode_pipeline_t decoder;
                    exr_result_t const result_exr_decoding_initialize = exr_decoding_initialize(context, k_default_part_index, &task.m_chunk, &decoder);
                    if (EXR_ERR_SUCCESS != result_exr_decoding_initialize)
                    {
                        assert(false);
                        return 1;
                    }

                    bool has_decode_error = false;
                    try
                    {
                        {
                            decoder.user_line_begin_skip = 0;
                            decoder.user_line_end_ignore = 0;

                            for (int16_t channel_index = 0; channel_index < decoder.channel_count; ++channel_index)
                            {
                                exr_coding_channel_info_t *channel = decoder.channels + channel_index;

                                if (channel->height > 0)
                                {
                                    void *slice_base;
                                    if (('R' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = out_pixel_data.data() - width * data_window.min.y - data_window.min.x;
                                    }
                                    else if (('G' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out_pixel_data.data() - width * data_window.min.y - data_window.min.x) + k_illumiant_image_channel_size * 1U);
                                    }
                                    else if (('B' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out_pixel_data.data() - width * data_window.min.y - data_window.min.x) + k_illumiant_image_channel_size * 2U);
                                    }
                                    else if (('A' == channel->channel_name[0]) && ('\0' == channel->channel_name[1]))
                                    {
                                        slice_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out_pixel_data.data() - width * data_window.min.y - data_window.min.x) + k_illumiant_image_channel_size * 3U);
                                    }
                                    else
                                    {
                                        assert(false);
                                        slice_base = NULL;
                                    }

                                    if (NULL != slice_base)
                                    {
                                        channel->user_bytes_per_element = k_illumiant_image_channel_size;
                                        channel->user_data_type = EXR_PIXEL_HALF;
                                        channel->user_pixel_stride = x_stride;
                                        channel->user_line_stride = y_stride;

                                        assert(tile_absolute_x >= 0);
                                        assert(tile_absolute_y >= 0);

                                        uint64_t const _uint64_ptr = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(slice_base)) + (static_cast<uint64_t>(y_stride) * static_cast<uint64_t>(tile_absolute_y) + (static_cast<uint64_t>(x_stride) * static_cast<uint64_t>(tile_absolute_x)));
                                        uintptr_t const ptr = static_cast<uintptr_t>(_uint64_ptr);
                                        if (!(_uint64_ptr == ptr))
                                        {
                                            throw std::runtime_error("Size Overflow");
                                        }
                                        channel->decode_to_ptr = reinterpret_cast<uint8_t *>(ptr);
                                    }
                                    else
                                    {
                                        assert(false);
                                        channel->user_pixel_stride = 0;
                                        channel->user_line_stride = 0;
                                        channel->decode_to_ptr = NULL;
                                    }
                                }
                                else
                                {
                                    assert(false);
                                    channel->user_pixel_stride = 0;
                                    channel->user_line_stride = 0;
                                    channel->decode_to_ptr = NULL;
                                }
                            }
                        }

                        exr_result_t const result_exr_decoding_choose_default_routines = exr_decoding_choose_default_routines(context, k_default_part_index, &decoder);
                        if (EXR_ERR_SUCCESS != result_exr_decoding_choose_default_routines)
                        {
                            throw std::runtime_error{"exr decoding choose default routines"};
                        }

                        exr_result_t const result_exr_decoding_run = exr_decoding_run(context, k_default_part_index, &decoder);
                        if (EXR_ERR_SUCCESS != result_exr_decoding_run)
                        {
                            throw std::runtime_error{"exr decoding run"};
                        }
                    }
                    catch (std::runtime_error exception)
                    {
                        std::cout << exception.what() << std::endl;
                        has_decode_error = true;
                    }

                    exr_result_t const result_exr_decoding_destroy = exr_decoding_destroy(context, &decoder);
                    assert(EXR_ERR_SUCCESS == result_exr_decoding_destroy);

                    return ((!has_decode_error) ? 0 : 1);
                },
                const_cast<_internal_exr_tile_buffer_task_context *>(&task_context));

            if (0 != result_exr_decode)
            {
                throw std::runtime_error{"exr decode"};
            }

            (*out_width) = width;
            (*out_height) = height;
        }
        else if (EXR_STORAGE_DEEP_TILED == storage)
        {
            throw std::runtime_error{"TODO Deep Tiled"};
        }
        else
        {
            throw std::runtime_error{"invalid storage"};
        }
    }
    catch (std::runtime_error exception)
    {
        std::cout << exception.what() << std::endl;
        has_error = true;
    }

    exr_result_t const result_exr_finish = exr_finish(&context);
    assert(EXR_ERR_SUCCESS == result_exr_finish);

    return (!has_error);
}

static void _internal_exr_error_handler(exr_const_context_t context, exr_result_t code, const char *message)
{
    std::cout << message << std::endl;
}

static void *_internal_exr_alloc(size_t bytes)
{
    return mcrt_malloc(bytes, 16U);
}

static void _internal_exr_free(void *ptr)
{
    return mcrt_free(ptr);
}

static int64_t _internal_exr_read_func(exr_const_context_t context, void *user_data, void *buffer, uint64_t size, uint64_t offset, exr_stream_error_func_ptr_t error_callback)
{
    _internal_exr_file_handle const *const file_handle = static_cast<_internal_exr_file_handle *>(user_data);

    if (NULL != file_handle)
    {
        uint64_t const read_source = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(file_handle->m_data_base)) + offset;
        uint64_t const read_size = ((offset + size) <= static_cast<uint64_t>(file_handle->m_data_size)) ? size : ((offset < static_cast<uint64_t>(file_handle->m_data_size)) ? (static_cast<uint64_t>(file_handle->m_data_size) - offset) : 0U);

        if ((static_cast<uint64_t>(static_cast<size_t>(read_source)) == read_source) && (static_cast<uint64_t>(static_cast<size_t>(read_size)) == read_size))
        {
            if (read_size > 0U)
            {
                std::memcpy(buffer, reinterpret_cast<void const *>(static_cast<uintptr_t>(read_source)), static_cast<size_t>(read_size));
            }
            else
            {
                assert(false);
            }

            int64_t const return_size = read_size;

            return return_size;
        }
        else
        {
            if (error_callback)
            {
                error_callback(context, EXR_ERR_INVALID_ARGUMENT, "read request size too large for architecture");
            }

            assert(false);
            return -1;
        }
    }
    else
    {
        if (error_callback)
        {
            error_callback(context, EXR_ERR_INVALID_ARGUMENT, "invalid file handle pointer");
        }

        assert(false);
        return -1;
    }
}

static int64_t _internal_exr_query_size_func(exr_const_context_t context, void *user_data)
{
    _internal_exr_file_handle const *const file_handle = static_cast<_internal_exr_file_handle *>(user_data);

    if (NULL != file_handle)
    {
        int64_t const size = file_handle->m_data_size;

        return size;
    }
    else
    {

        assert(false);
        return -1;
    }
}

static int64_t _internal_exr_write_func(exr_const_context_t context, void *user_data, void const *buffer, uint64_t size, uint64_t offset, exr_stream_error_func_ptr_t error_callback)
{
    if (error_callback)
    {
        error_callback(context, EXR_ERR_INVALID_ARGUMENT, "attempt to write the read only stream");
    }

    assert(false);
    return -1;
}

static void _internal_exr_shutdown(exr_const_context_t context, void *user_data, int failed)
{
}
