#pragma once

#include <vector>
#include <string>
#include <future>

#include "Texture.h"

class Image;

class ImageTexture final : public Texture
{
public:
    static std::shared_future<ImageTexture*> getTexture(
        std::string_view name,
        std::string_view path,
        bool gammaCorrect,
        const TextureSpec& spec);

    static const std::pair<int, const std::vector<const ImageTexture*>&> getPreparedTextures();

    ImageTexture(
        std::string_view name,
        std::string_view path,
        bool gammaCorrect,
        const TextureSpec& spec);

    virtual ~ImageTexture();

    void prepare() override;

    bool isValid() { return m_valid; }

    void load();

public:
    const std::string m_path;

    std::unique_ptr<Image> m_image;

    bool m_hdri{ false };

private:
    bool m_valid = false;
};

