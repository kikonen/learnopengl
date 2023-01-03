#pragma once

#include <vector>
#include <string>

#include "ki/GL.h"


class CubeMap
{
public:
    static unsigned int createEmpty(
        int size,
        GLenum internalFormat = GL_RGBA8);

    static unsigned int createFromImages(
        std::vector<std::string> faces,
        GLenum format = GL_RGB,
        GLenum internalFormat = GL_RGB8);
};
