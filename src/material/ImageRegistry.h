#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <future>

#include "TextureSpec.h"

class ImageTexture;

class ImageRegistry final
{
public:
    static void init() noexcept;
    static void release() noexcept;
    static ImageRegistry& get() noexcept;

    ImageRegistry();
    ImageRegistry& operator=(const ImageRegistry&) = delete;

    ~ImageRegistry();

    void clear();

    std::shared_future<std::shared_ptr<ImageTexture>> getTexture(
        std::string_view name,
        std::string_view path,
        bool shared,
        bool grayScale,
        bool gammaCorrect,
        bool normalMap,
        bool flipY,
        const TextureSpec& spec);

private:
    std::shared_future<std::shared_ptr<ImageTexture>> startLoad(
        std::shared_ptr<ImageTexture> texture);

private:
    std::unordered_map<std::string, std::shared_future<std::shared_ptr<ImageTexture>>> m_textures;

    std::mutex m_lock{};
};

