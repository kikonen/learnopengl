#include "PlainTexture.h"

#include <mutex>

#include "kigl/kigl.h"


PlainTexture::PlainTexture(
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

PlainTexture::~PlainTexture()
{
}

void PlainTexture::release()
{
    if (!m_prepared) return;
}

void PlainTexture::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    const int mipMapLevels = resolveMixMapLevels();

    //KI_GL_CALL(glad_glTextureStorage2D(m_textureID, 1, m_internalFormat, m_width, m_height));
    glTextureStorage2D(m_textureID, mipMapLevels, m_internalFormat, m_width, m_height);

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.asWrapS());
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.asWrapT());

    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.asMinFilter());
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.asMagFilter());

    glGenerateTextureMipmap(m_textureID);
}

void PlainTexture::setData(void* data, int size)
{
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, GL_UNSIGNED_BYTE, data);
}

void PlainTexture::prepareArray()
{
}
