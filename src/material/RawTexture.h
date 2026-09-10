#pragma once

#include "Texture.h"

class RawTexture final : public Texture
{
public:
    RawTexture(
        std::string_view name,
        bool grayScale,
        bool gammaCorrect,
        material::TextureType type,
        const material::TextureSpec& spec,
        int width,
        int height);

    virtual ~RawTexture();

    void release() override;

    void setData(
        void* data,
        int size,
        GLenum pixelFormat);

    void prepareSingle() override;

    void prepareArray(
        ArrayTexture& arr,
        uint32_t layer) override;

    void updateSingle() override;

    void updateArray(
        ArrayTexture& arr,
        uint32_t layer) override;

private:
    void* m_data{ nullptr };
    size_t m_size{ 0 };
    GLenum m_pixelFormat{ 0 };

    // number of mip levels allocated for the atlas texture (>1 == mipmapped)
    int m_mipMapLevels{ 1 };
};
