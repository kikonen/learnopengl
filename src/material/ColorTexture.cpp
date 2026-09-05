#include "ColorTexture.h"

#include <iostream>

#include <glm/ext.hpp>

#include "material/ArrayTexture.h"
#include "material/TextureRegistry.h"

#include "material/material_util.h"

namespace {
    material::TextureSpec getTextureSpec()
    {
        material::TextureSpec spec;
        spec.minFilter = material::TextureFilter::nearest;
        spec.magFilter = material::TextureFilter::nearest;
        spec.maxMipMapLevels = 1;
        return spec;
    }

    util::Ref<ColorTexture> s_whitePixelRGBA;
    util::Ref<ColorTexture> s_whitePixelRGB;
    util::Ref<ColorTexture> s_whitePixelR;

    util::Ref<ColorTexture> s_blackPixelRGBA;
    util::Ref<ColorTexture> s_blackPixelRGB;
    util::Ref<ColorTexture> s_blackPixelR;

    util::Ref<ColorTexture> s_flatNormalPixelRGBA;
}

util::Ref<ColorTexture> ColorTexture::getWhiteRGBA(bool prepare)
{
    if (!s_whitePixelRGBA) {
        s_whitePixelRGBA = util::Ref<ColorTexture>::create(
            "WHITE_RGBA",
            glm::vec4{ 1.f, 1.f, 1.f, 1.f },
            GL_RGBA8);
        if (prepare) {
            TextureRegistry::get().registerTexture(s_whitePixelRGBA);
        }
    }
    return s_whitePixelRGBA;
}

util::Ref<ColorTexture> ColorTexture::getWhiteRGB(bool prepare)
{
    if (!s_whitePixelRGB) {
        s_whitePixelRGB = util::Ref<ColorTexture>::create(
            "WHITE_RGB",
            glm::vec4{ 1.f, 1.f, 1.f, 1.f },
            GL_RGB8);
        if (prepare) {
            TextureRegistry::get().registerTexture(s_whitePixelRGB);
        }
    };
    return s_whitePixelRGB;
}

util::Ref<ColorTexture> ColorTexture::getWhiteR(bool prepare)
{
    if (!s_whitePixelR) {
        s_whitePixelR = util::Ref<ColorTexture>::create(
            "WHITE_R",
            glm::vec4{ 1.f, 1.f, 1.f, 1.f },
            GL_R8);
        if (prepare) {
            TextureRegistry::get().registerTexture(s_whitePixelR);
        }
    }
    return s_whitePixelR;
}

util::Ref<ColorTexture> ColorTexture::getBlackRGBA(bool prepare)
{
    if (!s_blackPixelRGBA) {
        s_blackPixelRGBA = util::Ref<ColorTexture>::create(
            "BLACK_RGBA",
            glm::vec4{ 0.f },
            GL_RGBA8);
        if (prepare) {
            TextureRegistry::get().registerTexture(s_blackPixelRGBA);
        }
    }
    return s_blackPixelRGBA;
}

util::Ref<ColorTexture> ColorTexture::getFlatNormalRGBA(bool prepare)
{
    if (!s_flatNormalPixelRGBA) {
        s_flatNormalPixelRGBA = util::Ref<ColorTexture>::create(
            "FLAT_NORMAL_RGBA",
            glm::vec4{ 0.5f, 0.5f, 1.f, 1.f },
            GL_RGBA8);
        if (prepare) {
            TextureRegistry::get().registerTexture(s_flatNormalPixelRGBA);
        }
    }
    return s_flatNormalPixelRGBA;
}

ColorTexture::ColorTexture(
    std::string_view name,
    glm::vec4 color,
    GLenum internalFormat)
    : Texture(name, false, false, material::TextureType::diffuse, getTextureSpec()),
    m_color{ color }
{
    m_internalFormat = internalFormat;
    m_format = GL_RGBA;
    m_pixelFormat = GL_UNSIGNED_BYTE;
}

ColorTexture::~ColorTexture()
{}

void ColorTexture::release()
{
    if (!m_prepared) return;
}

void ColorTexture::prepareSingle()
{
    if (m_prepared) return;
    m_prepared = true;

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

    {
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());

        const int mipMapLevels = m_spec.maxMipMapLevels;

        glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, 1, 1);

        unsigned char data[] {
            static_cast<unsigned char>(m_color[0] * 255),
            static_cast<unsigned char>(m_color[1] * 255),
            static_cast<unsigned char>(m_color[2] * 255),
            static_cast<unsigned char>(m_color[3] * 255),
        };

        glTextureSubImage2D(m_textureID, 0, 0, 0, 1, 1, m_format, m_pixelFormat, data);
        glGenerateTextureMipmap(m_textureID);

        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
    }
}

void ColorTexture::prepareArray(
    const util::Ref<ArrayTexture>& arr,
    uint32_t layer)
{
    if (m_prepared) return;
    m_prepared = true;

    // 1. sizes from array texture
    int targetWidth = arr->getWidth();
    int targetHeight = arr->getHeight();
    int targetChannels = arr->getChannels();
    bool is16Bit = arr->is16Bit();
    // gammaCorrect; only for SRGB case
    bool isSRGB = arr->isGammaCorrect();

    auto textureID = arr->getTextureID();
    m_handle = static_cast<GLuint64>(layer);

    // 2. generate image buffer
    if (is16Bit) {
        // --- 16 image
        std::vector<uint16_t> pixelData(targetWidth * targetHeight * targetChannels);

        // SRGB conversion
        float r = isSRGB ? material::linearToSRGB(m_color.r) : m_color.r;
        float g = isSRGB ? material::linearToSRGB(m_color.g) : m_color.g;
        float b = isSRGB ? material::linearToSRGB(m_color.b) : m_color.b;
        // Alpha is ALWAYS linear
        float a = m_color.a;

        uint16_t valR = static_cast<uint16_t>(glm::clamp(r * 65535.0f, 0.0f, 65535.0f));
        uint16_t valG = static_cast<uint16_t>(glm::clamp(g * 65535.0f, 0.0f, 65535.0f));
        uint16_t valB = static_cast<uint16_t>(glm::clamp(b * 65535.0f, 0.0f, 65535.0f));
        uint16_t valA = static_cast<uint16_t>(glm::clamp(a * 65535.0f, 0.0f, 65535.0f));

        for (int i = 0; i < targetWidth * targetHeight; ++i) {
            int idx = i * targetChannels;
            if (targetChannels >= 1) pixelData[idx + 0] = valR;
            // Ex. DuDv variation
            if (targetChannels >= 2) pixelData[idx + 1] = valG;
            // Ex. Normal/Plain color 16-bit
            if (targetChannels >= 3) pixelData[idx + 2] = valB;
            if (targetChannels >= 4) pixelData[idx + 3] = valA;
        }

        // stream to GPU
        glTextureSubImage3D(
            textureID, 0,
            0, 0, static_cast<GLint>(layer),
            targetWidth, targetHeight, 1,
            // format (ex. GL_RED, GL_RGB, GL_RGBA)
            arr->getFormat(),
            GL_UNSIGNED_SHORT,
            pixelData.data()
        );
    }
    else {
        // --- 8-bit image
        std::vector<uint8_t> pixelData(targetWidth * targetHeight * targetChannels);

        // SRGB conversion
        float r = isSRGB ? material::linearToSRGB(m_color.r) : m_color.r;
        float g = isSRGB ? material::linearToSRGB(m_color.g) : m_color.g;
        float b = isSRGB ? material::linearToSRGB(m_color.b) : m_color.b;
        float a = m_color.a;

        uint8_t valR = static_cast<uint8_t>(glm::clamp(r * 255.0f, 0.0f, 255.0f));
        uint8_t valG = static_cast<uint8_t>(glm::clamp(g * 255.0f, 0.0f, 255.0f));
        uint8_t valB = static_cast<uint8_t>(glm::clamp(b * 255.0f, 0.0f, 255.0f));
        uint8_t valA = static_cast<uint8_t>(glm::clamp(a * 255.0f, 0.0f, 255.0f));

        for (int i = 0; i < targetWidth * targetHeight; ++i) {
            int idx = i * targetChannels;
            if (targetChannels >= 1) pixelData[idx + 0] = valR;
            if (targetChannels >= 2) pixelData[idx + 1] = valG;
            if (targetChannels >= 3) pixelData[idx + 2] = valB;
            if (targetChannels >= 4) pixelData[idx + 3] = valA;
        }

        // stream to GPU
        glTextureSubImage3D(
            textureID, 0,
            0, 0, static_cast<GLint>(layer),
            targetWidth, targetHeight, 1,
            // format (ex. GL_RED, GL_RGB, GL_RGBA)
            arr->getFormat(),
            GL_UNSIGNED_BYTE,
            pixelData.data()
        );
    }

    KI_INFO(fmt::format(
        "TEX::COLOR::GENERATED: Pure color filled into array id={}, layer={}, format={}, channels={}, sRGB={}",
        textureID, layer, kigl::formatEnum(arr->getInternalFormat()), targetChannels, isSRGB
    ));
}
