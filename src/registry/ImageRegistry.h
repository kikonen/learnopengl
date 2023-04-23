#pragma once

#include <string>
#include <mutex>
#include <unordered_map>


class Image;

class ImageRegistry final
{
public:
    ImageRegistry() = default;
    ~ImageRegistry() = default;

    Image* getImage(const std::string& path);
public:

private:
    std::unordered_map<std::string, std::unique_ptr<Image>> m_images;

    std::mutex m_lock{};
};

