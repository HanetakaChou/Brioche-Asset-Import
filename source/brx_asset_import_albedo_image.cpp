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

#include "brx_asset_import_albedo_image.h"
#include "internal_import_image.h"

extern "C" brx_asset_import_image *brx_asset_import_create_image_from_input_stream(brx_asset_import_input_stream_factory *input_stream_factory, char const *input_stream_name)
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
            input_stream_factory->destory_instance(input_stream);
            return NULL;
        }

        if ((length <= 0) || (length >= static_cast<int64_t>(static_cast<int64_t>(1) << static_cast<int64_t>(24))))
        {
            input_stream_factory->destory_instance(input_stream);
            return NULL;
        }

        size_t input_stream_size = static_cast<uint32_t>(length);
        assert(static_cast<int64_t>(input_stream_size) == length);

        input_stream_data.resize(input_stream_size);

        intptr_t read_size = input_stream->read(input_stream_data.data(), input_stream_data.size());

        input_stream_factory->destory_instance(input_stream);

        if (-1 == read_size || read_size != input_stream_size)
        {
            input_stream_data.clear();
            return NULL;
        }
    }
    assert(!input_stream_data.empty());

    return brx_asset_import_create_image_from_memory(input_stream_data.data(), input_stream_data.size());
}

extern "C" brx_asset_import_image *brx_asset_import_create_image_from_memory(void const *data_base, size_t data_size)
{
    switch (internal_import_image_type(data_base, data_size))
    {
    case BRX_IMAGE_TYPE_ALBEDO:
    {
        void *new_unwrapped_albedo_image_base = mcrt_malloc(sizeof(brx_asset_import_albedo_image), alignof(brx_asset_import_albedo_image));
        assert(NULL != new_unwrapped_albedo_image_base);

        brx_asset_import_albedo_image *new_unwrapped_albedo_image = new (new_unwrapped_albedo_image_base) brx_asset_import_albedo_image{};
        if (new_unwrapped_albedo_image->init(data_base, data_size))
        {
            return new_unwrapped_albedo_image;
        }
        else
        {
            new_unwrapped_albedo_image->~brx_asset_import_albedo_image();
            mcrt_free(new_unwrapped_albedo_image);
            return NULL;
        }
    }
    break;
    case BRX_IMAGE_TYPE_UNKNOWN:
    default:
    {
        return NULL;
    }
    }
}

extern "C" void brx_asset_import_destory_image(brx_asset_import_image *wrapped_image)
{
    assert(NULL != wrapped_image);
    brx_asset_import_albedo_image *delete_unwrapped_image = static_cast<brx_asset_import_albedo_image *>(wrapped_image);

    delete_unwrapped_image->uninit();

    delete_unwrapped_image->~brx_asset_import_albedo_image();
    mcrt_free(delete_unwrapped_image);
}

brx_asset_import_albedo_image::brx_asset_import_albedo_image() : m_width(0U), m_height(0U)
{
}

brx_asset_import_albedo_image::~brx_asset_import_albedo_image()
{
    assert(0U == this->m_width);
    assert(0U == this->m_height);
}

bool brx_asset_import_albedo_image::init(void const *data_base, size_t data_size)
{
    assert(this->m_pixel_data.empty());
    assert(0U == this->m_width);
    assert(0U == this->m_height);

    return internal_import_albedo_image(data_base, data_size, this->m_pixel_data, &this->m_width, &this->m_height);
}

void brx_asset_import_albedo_image::uninit()
{
    assert(!this->m_pixel_data.empty());
    assert(this->m_width > 0U);
    assert(this->m_height > 0U);
    assert(this->m_width <= k_max_image_width_or_height);
    assert(this->m_height <= k_max_image_width_or_height);
#ifndef NDEBUG
    uint32_t const mip_level_count = this->get_mip_level_count();
    for (uint32_t mip_level_index = 0U; mip_level_index < mip_level_count; ++mip_level_index)
    {
        assert((static_cast<size_t>(this->get_width(mip_level_index)) * static_cast<size_t>(this->get_height(mip_level_index))) == this->m_pixel_data[mip_level_index].size());
    }
#endif

    this->m_pixel_data.clear();
    this->m_width = 0U;
    this->m_height = 0U;
}

BRX_SAMPLED_ASSET_IMAGE_FORMAT brx_asset_import_albedo_image::get_format(bool force_srgb) const
{
    return ((force_srgb) ? BRX_SAMPLED_ASSET_IMAGE_FORMAT_R8G8B8A8_SRGB : BRX_SAMPLED_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM);
}

uint32_t brx_asset_import_albedo_image::get_mip_level_count() const
{
    return this->m_pixel_data.size();
}

uint32_t brx_asset_import_albedo_image::get_row_pitch(uint32_t mip_level_index) const
{
    return this->get_row_size(mip_level_index);
}

uint32_t brx_asset_import_albedo_image::get_row_size(uint32_t mip_level_index) const
{
    return (static_cast<uint32_t>(k_albedo_image_channel_size) * static_cast<uint32_t>(k_albedo_image_num_channels) * static_cast<uint32_t>(this->get_width(mip_level_index)));
}

uint32_t brx_asset_import_albedo_image::get_row_count(uint32_t mip_level_index) const
{
    return this->get_width(mip_level_index);
}

uint32_t brx_asset_import_albedo_image::get_width(uint32_t mip_level_index) const
{
    return std::max(1U, this->m_width >> mip_level_index);
}

uint32_t brx_asset_import_albedo_image::get_height(uint32_t mip_level_index) const
{
    return std::max(1U, this->m_height >> mip_level_index);
}

void const *brx_asset_import_albedo_image::get_pixel_data(uint32_t mip_level_index) const
{
    return this->m_pixel_data[mip_level_index].data();
}
