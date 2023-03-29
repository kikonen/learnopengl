#pragma once

#include <string>
#include <mutex>
#include <map>

#include "asset/Image.h"

class ImageRegistry final
{
public:
    ImageRegistry() = default;
    ~ImageRegistry() = default;

    Image* getImage(const std::string& path);
public:

private:
    std::map<const std::string, std::unique_ptr<Image>> m_images;

    std::mutex m_lock{};
};

