#include "InlineTexture.h"

#include <unordered_map>
#include <mutex>
#include <regex>

#include <iostream>

#include <regex>
#include <fmt/format.h>

#include <fmt/format.h>

#include "util/util.h"
#include "util/Log.h"

#include "kigl/kigl.h"

#include "material/ArrayTexture.h"

namespace {
    const std::vector<std::regex> hdrMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

InlineTexture::InlineTexture(
    std::string_view name,
    std::vector<unsigned char> data,
    int width,
    int height,
    int channels,
    bool is16Bit,
    bool hasAlpha,
    bool gammaCorrect,
    material::TextureType type,
    const material::TextureSpec& spec)
    : Texture{ name, false, gammaCorrect, type, spec },
    m_name{ name },
    m_data{ data },
    m_width{ width },
    m_height{ height },
    m_channels{ channels },
    m_is16Bit{ is16Bit },
    m_hasAlpha{ hasAlpha }
{
}

InlineTexture::~InlineTexture()
{
}

std::string InlineTexture::str() const noexcept
{
    return fmt::format(
        "<IMG: {} {}bit {}ch {}x{} {}{} ({}), [{}], [{}, {}]>",
        m_name,
        m_is16Bit ? "16" : "8",
        m_channels,
        m_width,
        m_height,
        m_grayScale ? "GRAY " : "",
        kigl::formatEnum(m_internalFormat),
        kigl::formatEnum(m_format),
        util::as_integer(m_spec.wrap),
        util::as_integer(m_spec.minFilter),
        util::as_integer(m_spec.magFilter)
    );
}

void InlineTexture::release()
{
    if (!m_prepared) return;
    Texture::release();
}

void InlineTexture::prepareSingle()
{
    if (m_prepared) return;
    m_prepared = true;

    m_pixelFormat = GL_UNSIGNED_BYTE;

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) format
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_channels == 1) {
        if (m_is16Bit) {
            m_format = GL_RED;
            m_internalFormat = GL_R16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RED;
            m_internalFormat = GL_R8;
        }
        //m_specialTexture = true;
    }
    else if (m_channels == 2) {
        // NOTE KI in PNG channel 2 is alpha
        m_format = GL_RG;
        m_internalFormat = GL_TEXTURE_SWIZZLE_RGBA;
    }
    else if (m_channels == 3) {
        if (m_hdri) {
            // NOTE KI hdri is *linear* (no gamma)
            m_format = GL_RGB;
            m_internalFormat = GL_RGB16F;
            m_pixelFormat = GL_FLOAT;
        }
        else if (m_is16Bit) {
            m_format = GL_RGB;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RGB;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
            //m_internalFormat = assets.glPreferredTextureFormatRGB;
        }
    }
    else if (m_channels == 4) {
        if (m_is16Bit) {
            m_format = GL_RGBA;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RGBA;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            //m_internalFormat = assets.glPreferredTextureFormatRGBA;
        }
    }
    else {
        KI_WARN(fmt::format(
            "IMAGE: unsupported channels {}, image={}",
            m_channels, str()));
        m_valid = false;
        m_data.resize(0);
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

    {
        if (m_grayScale && m_channels == 1) {
            GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            glTextureParameteriv(m_textureID, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());

        const int mipMapLevels = resolveMixMapLevels();

        glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_data.data());
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO(fmt::format(
            "TEX_UPLOAD: {}, compressed={}\n{}",
            m_name,
            compFlag,
            str()));
    }

    //m_texIndex = Texture::nextIndex();

    m_data.resize(0);
}

void InlineTexture::prepareArray(
    const util::Ref<ArrayTexture>& arr,
    uint32_t layer)
{
}
