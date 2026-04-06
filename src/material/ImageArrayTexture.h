#pragma once

#include <vector>
#include <string>
#include <future>

#include "Texture.h"

class Image;

class ImageArrayTexture final : public Texture
{
public:
    ImageArrayTexture(
        std::string_view name,
        std::vector<std::string> paths,
        bool shared,
        bool grayScale,
        bool gammaCorrect,
        bool flipY,
        TextureType type,
        const TextureSpec& spec);

    virtual ~ImageArrayTexture();

    virtual std::string str() const noexcept override;

    bool validateLayers();

    void release() override;
    void prepare() override;

    void prepareNormal();
    void prepareKtx();

    bool isValid() const noexcept { return m_valid; }

    void load();

    const Image* getImage(int index) const
    {
        return m_images[index].get();
    }

public:
    const std::vector<std::string> m_paths;

    const bool m_flipY{ false };
    // NOTE KI shared == image data may be reused
    // => thus cannot automatically release after uploading to GPU
    const bool m_shared{ false };

private:
    std::vector<std::unique_ptr<Image>> m_images;

    bool m_hdri{ false };

    int m_width{ 0 };
    int m_height{ 0 };
    int m_channels{ 0 };
    bool m_is16Bbit{ false };

private:
    bool m_valid{ false };
};
