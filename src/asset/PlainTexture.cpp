#include "PlainTexture.h"

#include <mutex>
#include <glad/glad.h>

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

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    KI_GL_CALL(glad_glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, spec.mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, spec.mode);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void PlainTexture::setData(void* data, int size)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

