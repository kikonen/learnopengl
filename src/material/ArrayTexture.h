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
        bool is16Bit,
        int width,
        int height,
        int maxLayers,
        bool hdri,
        const material::TextureSpec& spec);

    virtual ~ArrayTexture();

    virtual std::string str() const noexcept override;

    void release() override;
    void prepareSingle() override;

    void prepareArray(
        const util::Ref<ArrayTexture>& arr,
        uint32_t layer) override;

    void prepareMipMaps();

    // @return layer index
    uint32_t allocateLayer();

    int getChannels() const noexcept
    {
        return m_channels;
    }

    bool is16Bit() const noexcept
    {
        return m_is16Bit;
    }

private:
    const int m_channels;
    const bool m_is16Bit;

    const int m_maxLayers;

    const bool m_hdri;

    std::vector<util::Ref<Texture>> m_textures;

    int m_layerCount{ 0 };

private:
    bool m_valid{ false };
};
