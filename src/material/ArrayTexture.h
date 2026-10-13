#pragma once

#include <vector>
#include <string>
#include <future>

#include "Texture.h"
#include "ArrayTextureType.h"

class ArrayTexture final : public Texture
{
public:
    ArrayTexture(
        material::ArrayTextureType arrayType,
        int unitIndex,
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
        ArrayTexture& arr,
        uint16_t layer) override;

    // @return registered layer index
    uint16_t registerTexture(
        const util::Ref<Texture>& texture);

    void updateTexture(
        const util::Ref<Texture>& texture);

    void updateMipMaps();

    // @return layer index
    uint16_t allocateLayer();

    uint16_t peekNextLayerIndex()
    {
        return static_cast<uint16_t>(m_layerIndex + 1);
    }

    int getUnitIndex() const noexcept
    {
        return m_unitIndex;
    }

    int getChannels() const noexcept
    {
        return m_channels;
    }

    bool is16Bit() const noexcept
    {
        return m_is16Bit;
    }

private:
    const material::ArrayTextureType m_arrayType;
    const int m_unitIndex;
    const int m_channels;
    const bool m_is16Bit;

    const int m_maxLayers;

    const bool m_hdri;

    int m_layerIndex{ -1 };

    std::vector<util::Ref<Texture>> m_registeredTextures;

    static const std::string& typeToName(material::ArrayTextureType type);
    static material::ArrayTextureType nameToType(const std::string& name);

private:
    bool m_valid{ false };
};
