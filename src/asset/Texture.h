#pragma once

#include <string>

#include "asset/Assets.h"
#include "TextureSpec.h"

#include "kigl/kigl.h"

/*
* https://learnopengl.com/Getting-started/Textures
*/
class Texture
{
public:
    Texture(
        std::string_view name,
        bool gammaCorrect,
        const TextureSpec& spec);

    virtual ~Texture();

    virtual void prepare(
        const Assets& assets) = 0;

    static GLuint nextIndex();

    // Reserve textureCount indexes which are consequetive from baseIndex
    // (possibly overlapping with old assignments)
    // @return Base index
    static GLuint getUnitIndexBase(int textureCount);

    static GLuint nextUnitIndex();

public:
    const std::string m_name;
    const bool m_gammaCorrect : 1;
    const TextureSpec m_spec;

    GLuint m_textureID{ 0 };
    //int m_texIndex{ -1 };
    GLuint64 m_handle{ 0 };

    mutable bool m_sent : 1 { false };

    bool m_specialTexture : 1 { false };

protected:
    bool m_prepared : 1 { false };

    int m_width{ 0 };
    int m_height{ 0 };
    int m_format{ 0 };
    int m_internalFormat{ 0 };

    GLenum m_pixelFormat{ 0 };
};
