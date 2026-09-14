#include "RawTexture.h"

#include <mutex>

#include "kigl/kigl.h"

#include "material/ArrayTexture.h"

RawTexture::RawTexture(
    std::string_view name,
    bool grayScale,
    bool gammaCorrect,
    material::TextureType type,
    const material::TextureSpec& spec,
    int width,
    int height)
    : Texture(name, grayScale, gammaCorrect, type, spec)
{
    m_width = width;
    m_height = height;

    m_format = GL_RGBA;
    m_internalFormat = m_gammaCorrect ? GL_SRGB8_ALPHA8 : GL_RGBA8;
}

RawTexture::~RawTexture()
{
}

void RawTexture::release()
{
    if (!m_prepared) return;
}

void RawTexture::setData(
    void* data,
    int size,
    GLenum pixelFormat)
{
    m_data = data;
    m_size = data != nullptr ? size : 0;
    m_pixelFormat = pixelFormat;
}

void RawTexture::prepareSingle()
{
    if (m_prepared) return;
    m_prepared = true;

    // TODO KI these need setters or such
    m_internalFormat = GL_R8;
    m_format = GL_RED;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());

    m_mipMapLevels = resolveMixMapLevels();

    glTextureParameteri(m_textureID, GL_TEXTURE_MAX_LEVEL, m_mipMapLevels - 1);
    glTextureStorage2D(m_textureID, m_mipMapLevels, m_internalFormat, m_width, m_height);
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, m_pixelFormat, m_data);

    if (m_mipMapLevels > 1) {
        glGenerateTextureMipmap(m_textureID);
    }
}

void RawTexture::prepareArray(
    ArrayTexture& arr,
    uint32_t layer)
{
    if (m_prepared) return;
    m_prepared = true;

    // 1. sizes from array texture
    int targetWidth = arr.getWidth();
    int targetHeight = arr.getHeight();
    int targetChannels = arr.getChannels();
    bool is16Bit = arr.is16Bit();
    // gammaCorrect; only for SRGB case
    bool isSRGB = arr.isGammaCorrect();

    auto textureID = arr.getTextureID();
    m_handle = static_cast<GLuint64>(layer);

    // 2. generate image buffer
    if (is16Bit) {
        // stream to GPU
        glTextureSubImage3D(
            textureID, 0,
            0, 0, static_cast<GLint>(layer),
            targetWidth, targetHeight, 1,
            // format (ex. GL_RED, GL_RGB, GL_RGBA)
            arr.getFormat(),
            m_pixelFormat,
            m_data
        );
    }
    else {
        // stream to GPU
        glTextureSubImage3D(
            textureID, 0,
            0, 0, static_cast<GLint>(layer),
            targetWidth, targetHeight, 1,
            // format (ex. GL_RED, GL_RGB, GL_RGBA)
            arr.getFormat(),
            m_pixelFormat,
            m_data
        );
    }

    KI_INFO(fmt::format(
        "TEX::COLOR::GENERATED: Pure color filled into array id={}, layer={}, format={}, channels={}, sRGB={}",
        textureID, layer, kigl::formatEnum(arr.getInternalFormat()), targetChannels, isSRGB
    ));
}

void RawTexture::updateSingle()
{
    glTextureSubImage2D(
        m_textureID,
        0,
        0, 0, m_width, m_height,
        m_format,
        m_pixelFormat,
        m_data);

    // newly rasterized glyphs changed level 0 -> refresh the mip chain
    if (m_mipMapLevels > 1) {
        glGenerateTextureMipmap(m_textureID);
    }
}

void RawTexture::updateArray(
    ArrayTexture& arr,
    uint32_t layer)
{
    glTextureSubImage3D(
        arr.getTextureID(),
        0,
        0, 0, layer,
        m_width, m_height, 1,
        arr.getFormat(),
        m_pixelFormat,
        m_data
    );
}
