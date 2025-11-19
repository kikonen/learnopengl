#include "Image.h"

#include <map>
#include <fstream>

#include <fmt/format.h>

#include <codeanalysis/warnings.h>

#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#pragma warning(disable : 6308)
#pragma warning(disable : 26451)

#include <stb_image.h>

#pragma warning(pop)

#include "util/Log.h"

namespace {
    const std::string KTX_EXT = ".ktx";
}

Image::Image(Image&& o) noexcept
    : m_path{ o.m_path },
    m_flipped{ o.m_flipped },
    m_hdri{ o.m_hdri },
    m_ktx{ o.m_ktx },
    m_width{ o.m_width },
    m_height{ o.m_height },
    m_channels{ o.m_channels },
    m_is16Bbit{ o.m_is16Bbit },
    m_data{ o.m_data },
    m_loaded{ o.m_loaded },
    m_res{ o.m_res }
{
    // NOTE KI o is moved now
    o.m_data = nullptr;
}

Image::Image(bool flipped)
    : Image("", flipped)
{
}

Image::Image(std::string_view path, bool flipped)
    :Image(path, flipped, false)
{
}

Image::Image(
    std::string_view path,
    bool flipped,
    bool hdri)
    : m_path{ path },
    m_flipped{ flipped },
    m_hdri{ hdri },
    m_ktx{ path.ends_with(KTX_EXT) }
{
}

Image::~Image()
{
    stbi_image_free(m_data);
    m_data = nullptr;
}

// NOTE KI *NOT* thread safe
// https://github.com/nothings/stb/issues/309
int Image::load() {
    if (m_loaded) {
        return m_res;
    }
    m_loaded = true;

    if (m_ktx) return loadKtx();
    return loadNormal();
}

int Image::loadNormal()
{
    stbi_set_flip_vertically_on_load_thread(m_flipped);
    //stbi_set_flip_vertically_on_load(flip);

    m_is16Bbit = stbi_is_16_bit(m_path.c_str());

    if (m_hdri) {
        m_data = (unsigned char*)stbi_loadf(
            m_path.c_str(),
            &m_width,
            &m_height,
            &m_channels,
            STBI_default);
    } else if (m_is16Bbit) {
        m_data = (unsigned char*)stbi_load_16(
            m_path.c_str(),
            &m_width,
            &m_height,
            &m_channels,
            STBI_default);
    }
    else {
        m_data = (unsigned char*)stbi_load(
            m_path.c_str(),
            &m_width,
            &m_height,
            &m_channels,
            STBI_default);
    }

    if (m_data) {
        KI_INFO(fmt::format(
            "IMAGE::LOADED {}, flipped={}, channels={}, width={}, height={}",
            m_path, m_flipped, m_channels, m_width, m_height));
    }
    else {
        KI_ERROR(fmt::format("IMAGE::LOAD_FAILED {}", m_path));
    }

    checkAlpha();

    m_res = m_data ? 0 : -1;
    return m_res;
}

int Image::loadFromMememory(
    const std::vector<unsigned char>& data)
{
    m_is16Bbit = stbi_is_16_bit_from_memory(data.data(), data.size());

    stbi_set_flip_vertically_on_load_thread(m_flipped);

    m_data = (unsigned char*)stbi_load_from_memory(
        data.data(),
        data.size(),
        &m_width,
        &m_height,
        &m_channels,
        STBI_default);

    checkAlpha();

    return m_data ? 0 : -1;
}

int Image::loadKtx()
{
    //std::ifstream f{ m_path, std::ios::in | std::ios::binary | std::ios::ate };

    //if (!f.is_open()) {
    //    return -1;
    //}

    //if (f.is_open())
    //{
    //    std::streampos size = f.tellg();
    //    m_data = new unsigned char[size];
    //    f.seekg(0, std::ios::beg);
    //    f.read((char*)m_data, size);
    //    f.close();
    //}

    return 0;
}

void Image::checkAlpha()
{
    m_hasAlpha = false;

    if (!m_data) return;
    if (m_channels != 4) return;

    unsigned char* srcByteData{ nullptr };
    unsigned short* srcShortData{ nullptr };

    if (m_is16Bbit) {
        srcShortData = (unsigned short*)m_data;
    }
    else {
        srcByteData = m_data;
    }

    const auto alphaIndex = 3;
    bool opaque = true;
    for (size_t x = 0; x < m_height; x++) {
        for (size_t y = 0; y < m_width; y++) {
            int srcIndex = y * (m_width * m_channels) + x * m_channels + alphaIndex;
            if (srcByteData) {
                if (srcByteData[srcIndex] < 255) {
                    opaque = false;
                    break;
                }
            }
            else if (srcShortData) {
                if (srcShortData[srcIndex] < 65535) {
                    opaque = false;
                    break;
                }
            }
        }
        if (!opaque) break;
    }
    m_hasAlpha = !opaque;
}
