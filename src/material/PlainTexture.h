#pragma once

#include "Texture.h"

class PlainTexture final : public Texture
{
public:
    PlainTexture(
        std::string_view name,
        bool gammaCorrect,
        const TextureSpec& spec,
        int width,
        int height);

    virtual ~PlainTexture();

    void prepare() override;
    void setData(void* data, int size);
};
