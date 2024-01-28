#include "imageRegistry.h"

#include "asset/Image.h"


namespace {
}

Image* ImageRegistry::getImage(std::string_view path)
{
    const std::string cacheKey{ path };

    std::lock_guard lock(m_lock);

    auto e = m_images.find(cacheKey);
    if (e == m_images.end()) {
        m_images[cacheKey] = std::make_unique<Image>(path, true);
        e = m_images.find(cacheKey);
    }
    return e->second.get();
}
