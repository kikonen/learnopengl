#pragma once

#include "Texture.h"

class PlainTexture final : public Texture
{
public:
    PlainTexture(
        const std::string& name,
        bool gammaCorrect,
        const TextureSpec& spec,
        int width,
        int height);

    virtual ~PlainTexture();

    void prepare(const Assets& assets) override;
    void setData(void* data, int size);
};
