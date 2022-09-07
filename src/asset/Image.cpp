#include "Image.h"

#include <iostream>
#include <map>
#include <mutex>

#include <stb_image.h>

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
    :path(path)
{
}

Image::~Image()
{
    stbi_image_free(data);
    data = nullptr;
}

// NOTE KI *NOT* thread safe
// https://github.com/nothings/stb/issues/309
int Image::load(bool flip) {
    std::lock_guard<std::mutex> lock(load_lock);

    if (loaded) {
        return res;
    }
    loaded = true;

    flipped = flip;
    stbi_set_flip_vertically_on_load(flip);

    data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        STBI_default);

    if (data) {
        KI_INFO_SB("IMAGE::LOADED " << path
            + " flipped=" << std::to_string(flipped)
            + " channels=" << std::to_string(channels)
            + " width=" << std::to_string(width)
            + " height=" << std::to_string(height));
    }
    else {
        KI_ERROR_SB("IMAGE::LOAD_FAILED " << path);
    }

    res = data ? 0 : -1;
    return res;
}
