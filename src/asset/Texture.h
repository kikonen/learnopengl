#pragma once

#include <string>

#include "asset/Assets.h"

#include "ki/GL.h"

constexpr int MIP_MAP_LEVELS = 3;

struct TextureSpec {
    int clamp = GL_CLAMP_TO_EDGE;

    int minFilter = GL_NEAREST_MIPMAP_LINEAR;
    int magFilter = GL_LINEAR;

    int mipMapLevels = MIP_MAP_LEVELS;
};

/*
* https://learnopengl.com/Getting-started/Textures
*/
class Texture
{
public:
    Texture(const std::string& name, const TextureSpec& spec);
    virtual ~Texture();

    virtual void prepare(const Assets& assets) = 0;

    static GLuint nextIndex();

    // Reserve textureCount indexes which are consequetive from baseIndex
    // (possibly overlapping with old assignments)
    // @return Base index
    static GLuint getUnitIndexBase(int textureCount);

    static GLuint nextUnitIndex();

public:
    const std::string m_name;
    const TextureSpec m_spec;

    GLuint m_textureID = 0;
    int m_texIndex = -1;
    GLuint64 m_handle = 0;

    mutable bool m_sent = false;

protected:
    bool m_prepared = false;

    int m_width = 0;
    int m_height = 0;
    int m_format = 0;
    int m_internalFormat = 0;
};
