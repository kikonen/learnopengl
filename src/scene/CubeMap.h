#pragma once

#include <vector>
#include <string>

#include "ki/GL.h"

class RenderContext;

class CubeMap
{
public:
    CubeMap(bool empty);

    ~CubeMap();

    void create();

    void bindTexture(const RenderContext& ctx, int unitIndex);

private:
    GLuint createEmpty();

    GLuint createFaces();

public:
    const bool m_empty;

    std::vector<std::string> m_faces;
    int m_size = 0;

    GLuint m_textureID = 0;

    GLenum m_format = GL_RGB;
    GLenum m_internalFormat = GL_RGB8;
};
