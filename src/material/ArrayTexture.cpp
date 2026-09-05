#include "ArrayTexture.h"

#include <unordered_map>
#include <mutex>
#include <regex>

#include <iostream>

#include <regex>
#include <fmt/format.h>

// https://stackoverflow.com/questions/5159353/how-can-i-get-rid-of-the-imp-prefix-in-the-linker-in-vc
#define KHRONOS_STATIC
#include <ktx.h>

#include <fmt/format.h>

#include "material/Image.h"

#include "util/util.h"
#include "util/Log.h"
#include "util/file.h"

#include "kigl/kigl.h"

namespace
{
    const std::vector<std::regex> hdrMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

ArrayTexture::ArrayTexture(
    const std::string& name,
    bool grayScale,
    bool gammaCorrect,
    int channels,
    bool is16Bit,
    int width,
    int height,
    int maxLayers,
    bool hdri,
    const material::TextureSpec& spec)
    : Texture{ name, grayScale, gammaCorrect, material::TextureType::array, spec },
    m_channels{ channels },
    m_is16Bit{ is16Bit },
    m_maxLayers{ maxLayers },
    m_hdri{ hdri }
{
    m_width = width;
    m_height = height;
}

ArrayTexture::~ArrayTexture()
{
}

std::string ArrayTexture::str() const noexcept
{
    return fmt::format(
        "<IMG: {}[{}] {}bit {}ch {}x{} {}{} ({}), [{}], [{}, {}]>",
        m_name,
        m_textures.size(),
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

void ArrayTexture::release()
{
    if (!m_prepared) return;
    Texture::release();
}

void ArrayTexture::prepareSingle()
{
    if (m_prepared) return;
    m_prepared = true;

    // NOTE KI 1 & 2 channels have issues
    // => need to convert manually to RGB(A) s
    // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    if (m_channels == 1) {
        if (m_is16Bit) {
            m_format = GL_RED;
            m_internalFormat = GL_R16;
        }
        else {
            m_format = GL_RED;
            m_internalFormat = GL_R8;
        }
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
        }
        else if (m_is16Bit) {
            m_format = GL_RGB;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB16;
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
        }
        else {
            m_format = GL_RGBA;
            m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            //m_internalFormat = assets.glPreferredTextureFormatRGBA;
        }
    }
    else {
        KI_WARN(fmt::format(
            "TEX::ARRAY::PREPARE: unsupported channels {}, image={}",
            m_channels, str()));
        m_valid = false;
        return;
    }

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_textureID);

    //kigl::setLabel(GL_TEXTURE_2D_ARRAY, m_textureID, m_name);

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

        const int layerCount = static_cast<int>(m_maxLayers);
        const uint8_t mipMapLevels = std::min(
            m_spec.maxMipMapLevels,
            static_cast<uint8_t>(log2(std::max(m_width, m_height))));

        glTextureStorage3D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height, layerCount);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

        GLint compFlag;
        glGetTextureLevelParameteriv(m_textureID, 0, GL_TEXTURE_COMPRESSED, &compFlag);
        KI_INFO(fmt::format(
            "TEX::ARRAY::UPLOAD::PLAIN: name={}, compressed={}\n{}",
            m_name,
            compFlag,
            str()));
    }
}

void ArrayTexture::prepareArray(
    const util::Ref<ArrayTexture>& arr,
    uint32_t layer)
{
    // NOTE KI array cannot be in array
}

void ArrayTexture::prepareMipMaps()
{
    if (m_spec.maxMipMapLevels > 1) {
        glGenerateTextureMipmap(m_textureID);
    }
}

uint32_t ArrayTexture::allocateLayer()
{
    m_layerCount++;
    return static_cast<uint32_t>(m_layerCount);
}
