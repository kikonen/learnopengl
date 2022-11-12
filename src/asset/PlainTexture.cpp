#include "PlainTexture.h"

#include <mutex>

#include "ki/GL.h"


PlainTexture::PlainTexture(const std::string& name, const TextureSpec& spec, int width, int height)
    : Texture(name, spec)
{
    this->width = width;
    this->height = height;

    format = GL_RGBA;
    internalFormat = GL_RGBA8;
}

PlainTexture::~PlainTexture()
{
}

void PlainTexture::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

    KI_GL_CALL(glad_glTextureStorage2D(textureID, 1, internalFormat, width, height));

    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, spec.mode);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, spec.mode);

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateTextureMipmap(textureID);
}

void PlainTexture::setData(void* data, int size)
{
    glTextureSubImage2D(textureID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
}

