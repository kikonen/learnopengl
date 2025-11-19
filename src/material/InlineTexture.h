#pragma once

#pragma once

#include <vector>
#include <string>
#include <future>

#include "Texture.h"

class Image;

class InlineTexture final : public Texture
{
public:
    InlineTexture(
        std::string_view name,
        std::vector<unsigned char> data,
        int width,
        int height,
        int channels,
        bool is16Bbit,
        bool hasAlpha,
        bool gammaCorrect,
        const TextureSpec& spec);

    virtual ~InlineTexture();

    virtual std::string str() const noexcept override;

    void release() override;
    void prepare() override;

    void prepareNormal();

    bool isValid() const noexcept { return true; }

    bool hasAlpha() const noexcept { return m_hasAlpha; }

public:
    const std::string m_name;
    const bool m_flipY{ false };

    std::vector<unsigned char> m_data;

    bool m_hdri{ false };

    const int m_width;
    const int m_height;
    const int m_channels;
    const bool m_is16Bbit;
    const bool m_hasAlpha;

    bool m_valid{ true };
};
