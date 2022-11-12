#include "PlainTexture.h"

#include <mutex>

#include "ki/GL.h"


PlainTexture::PlainTexture(const std::string& name, const TextureSpec& spec, int width, int height)
    : Texture(name, spec)
{
    m_width = width;
    m_height = height;

    m_format = GL_RGBA;
    m_internalFormat = GL_RGBA8;
}

PlainTexture::~PlainTexture()
{
}

void PlainTexture::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    KI_GL_CALL(glad_glTextureStorage2D(m_textureID, 1, m_internalFormat, m_width, m_height));

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.mode);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.mode);

    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateTextureMipmap(m_textureID);
}

void PlainTexture::setData(void* data, int size)
{
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, m_format, GL_UNSIGNED_BYTE, data);
}

