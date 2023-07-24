#pragma once

#include "ki/GL.h"

class RenderContext;

class TextureQuad {
public:
    void prepare();

    void draw(const RenderContext& ctx);

private:
    GLuint m_vao{ 0 };
    GLuint m_vbo{ 0 };
};
