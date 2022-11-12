#pragma once

#include <string>

#include "scene/RenderContext.h"

struct TextureSpec {
    int mode = GL_CLAMP_TO_EDGE;
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
    const std::string name;
    const TextureSpec spec;

    GLuint textureID = 0;
    int m_texIndex = -1;
    GLuint64 m_handle = 0;

protected:
    bool m_prepared = false;

    int width = 0;
    int height = 0;
    int format = 0;
    int internalFormat = 0;
};
