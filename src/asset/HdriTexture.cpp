#include "Hdritexture.h"

#include "kigl/GLTextureHandle.h"
#include "asset/Image.h"
#include "registry/Registry.h"

void HdriTexture::prepareRT(
    const Assets& assets,
    Registry* registry)
{
    Image image{ m_path, true, true };

    if (image.load()) {
        return;
    }

    const unsigned int width = image.m_width;
    const unsigned int height = image.m_height;

    {
        m_texture.create("hdri", GL_TEXTURE_2D, width, height);

        const GLenum internalFormat = GL_RGB16F;
        const GLenum pixelFormat = GL_FLOAT;
        const GLenum format = GL_RGB;

        glTextureStorage2D(m_texture, 1, internalFormat, width, height);
        glTextureSubImage2D(m_texture, 0, 0, 0, width, height, format, pixelFormat, image.m_data);

        glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}
