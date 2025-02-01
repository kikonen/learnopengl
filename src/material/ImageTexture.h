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
        bool grayScale,
        bool gammaCorrect,
        bool flipY,
        const TextureSpec& spec);

    ImageTexture(
        std::string_view name,
        std::string_view path,
        bool grayScale,
        bool gammaCorrect,
        bool flipY,
        const TextureSpec& spec);

    virtual ~ImageTexture();

    virtual std::string str() const noexcept override;

    void prepare() override;

    void prepareNormal();
    void prepareKtx();

    bool isValid() { return m_valid; }

    void load();

public:
    const std::string m_path;
    const bool m_flipY{ false };

    std::unique_ptr<Image> m_image;

    bool m_hdri{ false };

    int m_width{ 0 };
    int m_height{ 0 };
    int m_channels{ 0 };
    bool m_is16Bbit{ false };

private:
    bool m_valid{ false };
};

