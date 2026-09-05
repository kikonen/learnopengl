#pragma once

#include <string>

#include "kigl/kigl.h"

#include "util/Ref.h"

#include "TextureSpec.h"
#include "TextureType.h"

class ArrayTexture;

//
// https://learnopengl.com/Getting-started/Textures
//
class Texture : public util::RefCounted<>
{
public:
    Texture(
        std::string_view name,
        bool grayScale,
        bool gammaCorrect,
        material::TextureType type,
        const material::TextureSpec& spec);

    virtual ~Texture();

    virtual std::string str() const noexcept;

    virtual void release();

    virtual void prepareSingle() = 0;
    virtual void prepareHandle();

    virtual void prepareArray(
        const util::Ref<ArrayTexture>& arr,
        uint32_t layer) = 0;

    int resolveMixMapLevels();

    int getWidth() const noexcept
    {
        return m_width;
    }

    int getHeight() const noexcept
    {
        return m_height;
    }

    bool isGammaCorrect() const noexcept
    {
        return m_gammaCorrect;
    }

    GLuint getTextureID() const noexcept
    {
        return m_textureID;
    }

    GLuint64 getHandle() const noexcept
    {
        return m_handle;
    }

    int getFormat() const noexcept
    {
        return m_format;
    }

    int getInternalFormat() const noexcept
    {
        return m_internalFormat;
    }

public:
    const std::string m_name;
    const bool m_grayScale : 1;
    const bool m_gammaCorrect : 1;
    const material::TextureType m_type;
    const material::TextureSpec m_spec;

    GLuint m_textureID{ 0 };
    GLuint64 m_handle{ 0 };
    bool m_boundBindless{ false };

    mutable bool m_sent : 1 { false };

protected:
    bool m_prepared : 1 { false };

    int m_width{ 0 };
    int m_height{ 0 };
    int m_format{ 0 };
    int m_internalFormat{ 0 };
};
