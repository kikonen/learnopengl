#pragma once

#include "Texture.h"

class PlainTexture final : public Texture
{
public:
    PlainTexture(
        std::string_view name,
        bool grayScale,
        bool gammaCorrect,
        TextureType type,
        const TextureSpec& spec,
        int width,
        int height);

    virtual ~PlainTexture();

    void release() override;
    void prepare() override;
    void setData(void* data, int size);
};
