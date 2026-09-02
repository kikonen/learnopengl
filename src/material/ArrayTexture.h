#pragma once

#include <vector>
#include <string>
#include <future>

#include "Texture.h"

class ArrayTexture final : public Texture
{
public:
    ArrayTexture(
        const std::string& name,
        bool grayScale,
        bool gammaCorrect,
        int channels,
        bool is16Bbit,
        int width,
        int height,
        int maxLayers,
        bool hdri,
        const material::TextureSpec& spec);

    virtual ~ArrayTexture();

    virtual std::string str() const noexcept override;

    bool validateLayers();

    void release() override;
    void prepare() override;

    // @return layer index
    int32_t allocateLayer(const util::Ref<Texture>& texture);

private:
    void preparePlain();

private:
    const int m_channels;
    const bool m_is16Bbit;

    const int m_maxLayers;

    const bool m_hdri;

    std::vector<util::Ref<Texture>> m_textures;

    int m_format{ 0 };
    int m_internalFormat{ 0 };

private:
    bool m_valid{ false };
};
