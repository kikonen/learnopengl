#include "imageRegistry.h"

#include "asset/Image.h"


namespace {
}

Image* ImageRegistry::getImage(const std::string& path)
{
    std::lock_guard<std::mutex> lock(m_lock);

    const std::string cacheKey = path;

    auto e = m_images.find(cacheKey);
    if (e == m_images.end()) {
        m_images[cacheKey] = std::make_unique<Image>(path);
        e = m_images.find(cacheKey);
    }
    return e->second.get();
}
