#include "Image.h"

#include <iostream>
#include <map>
#include <mutex>

#include <codeanalysis/warnings.h>

#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#pragma warning(disable : 6308)
#pragma warning(disable : 26451)

#include <stb_image.h>

#pragma warning(pop)

#include "util/Log.h"

namespace {
    std::map<const std::string, std::unique_ptr<Image>> images;

    std::mutex images_lock;
    std::mutex load_lock;
}

Image* Image::getImage(const std::string& path)
{
    std::lock_guard<std::mutex> lock(images_lock);

    const std::string cacheKey = path;

    auto e = images.find(cacheKey);
    if (e == images.end()) {
        images[cacheKey] = std::make_unique<Image>(path);
        e = images.find(cacheKey);
    }
    return e->second.get();
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
    std::lock_guard<std::mutex> lock(load_lock);

    if (m_loaded) {
        return m_res;
    }
    m_loaded = true;

    m_flipped = flip;
    stbi_set_flip_vertically_on_load(flip);

    m_data = stbi_load(
        m_path.c_str(),
        &m_width,
        &m_height,
        &m_channels,
        STBI_default);

    if (m_data) {
        KI_INFO_SB("IMAGE::LOADED " << m_path
            + " flipped=" << std::to_string(m_flipped)
            + " channels=" << std::to_string(m_channels)
            + " width=" << std::to_string(m_width)
            + " height=" << std::to_string(m_height));
    }
    else {
        KI_ERROR_SB("IMAGE::LOAD_FAILED " << m_path);
    }

    m_res = m_data ? 0 : -1;
    return m_res;
}
