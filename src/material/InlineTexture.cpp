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
    bool is16Bbit,
    bool gammaCorrect,
    const TextureSpec& spec)
    : Texture{ name, false, gammaCorrect, spec },
    m_name{ name },
    m_data{ data },
    m_width{ width },
    m_height{ height },
    m_channels{ channels },
    m_is16Bbit{ is16Bbit }
{
}

InlineTexture::~InlineTexture()
{
}

std::string InlineTexture::str() const noexcept
{
    return fmt::format(
        "<IMG: {} {}bit {}ch {}x{} {}{} ({}), [{}, {}], [{}, {}]>",
        m_name,
        m_is16Bbit ? "16" : "8",
        m_channels,
        m_width,
        m_height,
        m_grayScale ? "GRAY " : "",
        kigl::formatEnum(m_internalFormat),
        kigl::formatEnum(m_format),
        kigl::formatEnum(m_spec.wrapS),
        kigl::formatEnum(m_spec.wrapT),
        kigl::formatEnum(m_spec.minFilter),
        kigl::formatEnum(m_spec.magFilter)
    );
}

void InlineTexture::release()
{
    if (!m_prepared) return;
    Texture::release();
}

void InlineTexture::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    prepareNormal();
}

void InlineTexture::prepareNormal()
{
    m_pixelFormat = GL_UNSIGNED_BYTE;

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) format
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_channels == 1) {
        if (m_is16Bbit) {
            m_format = GL_RED;
            m_internalFormat = m_grayScale ? GL_RGB16 : GL_R16;
            m_pixelFormat = GL_UNSIGNED_SHORT;
        }
        else {
            m_format = GL_RED;
            m_internalFormat = m_grayScale ? GL_RGB8 : GL_R8;
        }
        //m_specialTexture = true;
    }
    else if (m_channels == 2) {
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
        else if (m_is16Bbit) {
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
        if (m_is16Bbit) {
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

    if (m_specialTexture) {
        glTextureStorage2D(m_textureID, 1, m_internalFormat, m_width, m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_data.data());

        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);
    }
    else {
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

        const uint8_t mipMapLevels = std::min(
            m_spec.mipMapLevels,
            static_cast<uint8_t>(log2(std::max(m_width, m_height))));

        glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_data.data());
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO_OUT(fmt::format(
            "TEX_UPLOAD: {}, compressed={}\n{}",
            m_name,
            compFlag,
            str()));

        m_handle = glGetTextureHandleARB(m_textureID);
        glMakeTextureHandleResidentARB(m_handle);
    }

    //m_texIndex = Texture::nextIndex();

    m_data.resize(0);
}
