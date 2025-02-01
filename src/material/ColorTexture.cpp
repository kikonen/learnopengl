#include "ColorTexture.h"

#include <glm/ext.hpp>

namespace {
    TextureSpec getTextureSpec() {
        TextureSpec spec;
        spec.minFilter = GL_NEAREST;
        spec.magFilter = GL_NEAREST;
        spec.mipMapLevels = 1;
        return spec;
    }
}

const ColorTexture& ColorTexture::getWhiteRGBA()
{
    static ColorTexture s_whitePixel{
        "WHITE_RGBA",
        glm::vec4{ 1.f, 1.f, 1.f, 1.f },
        GL_RGBA8,
        true };
    return s_whitePixel;
}

const ColorTexture& ColorTexture::getWhiteRGB()
{
    static ColorTexture s_whitePixel{
        "WHITE_RGB",
        glm::vec4{ 1.f, 1.f, 1.f, 1.f },
        GL_RGB8,
        true };
    return s_whitePixel;
}

const ColorTexture& ColorTexture::getWhiteR()
{
    static ColorTexture s_whitePixel{
        "WHITE_R",
        glm::vec4{ 1.f, 1.f, 1.f, 1.f },
        GL_R8,
        true };
    return s_whitePixel;
}

const ColorTexture& ColorTexture::getBlackRGBA()
{
    static ColorTexture s_blackPixel{
        "BLACK_RGBA",
        glm::vec4{ 0.f },
        GL_RGBA8,
        true };
    return s_blackPixel;
}

ColorTexture::ColorTexture(
    std::string_view name,
    glm::vec4 color,
    GLenum internalFormat,
    bool usePrepare)
    : Texture(name, false, false, getTextureSpec()),
    m_color{ color }
{
    m_internalFormat = internalFormat;
    m_format = GL_RGBA;
    m_pixelFormat = GL_UNSIGNED_BYTE;
    if (usePrepare) {
        prepare();
    }
}

ColorTexture::~ColorTexture()
{}

void ColorTexture::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    kigl::setLabel(GL_TEXTURE, m_textureID, m_name);

    {
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, m_spec.wrapS);
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, m_spec.wrapT);

        // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
        // https://stackoverflow.com/questions/12363463/when-should-i-set-gl-texture-min-filter-and-gl-texture-mag-filter
        glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_spec.minFilter);
        glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, m_spec.magFilter);

        const int mipMapLevels = m_spec.mipMapLevels;

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

        m_handle = glGetTextureHandleARB(m_textureID);
        glMakeTextureHandleResidentARB(m_handle);
    }
}
