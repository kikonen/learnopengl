#include "Image.h"

#include <iostream>
#include <map>
#include <mutex>

#include "fmt/format.h"

#include <codeanalysis/warnings.h>

#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#pragma warning(disable : 6308)
#pragma warning(disable : 26451)

#include <stb_image.h>

#pragma warning(pop)

#include "util/Log.h"

namespace {
    std::mutex load_lock;
}

Image::Image(const std::string& path)
    :m_path(path)
{
}

Image::~Image()
{
    stbi_image_free(m_data);
    m_data = nullptr;
}

// NOTE KI *NOT* thread safe
// https://github.com/nothings/stb/issues/309
int Image::load(bool flip) {
    //std::lock_guard<std::mutex> lock(load_lock);

    if (m_loaded) {
        return m_res;
    }
    m_loaded = true;

    m_flipped = flip;
    stbi_set_flip_vertically_on_load_thread(flip);
    //stbi_set_flip_vertically_on_load(flip);

    m_data = stbi_load(
        m_path.c_str(),
        &m_width,
        &m_height,
        &m_channels,
        STBI_default);

    if (m_data) {
        KI_INFO(fmt::format(
            "IMAGE::LOADED {}, flipped={}, channels={}, width={}, height={}",
            m_path, m_flipped, m_channels, m_width, m_height));
    }
    else {
        KI_ERROR(fmt::format("IMAGE::LOAD_FAILED {}", m_path));
    }

    m_res = m_data ? 0 : -1;
    return m_res;
}
