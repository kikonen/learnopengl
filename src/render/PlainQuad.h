#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class GLState;

class PlainQuad {
public:
    PlainQuad() = default;
    ~PlainQuad() = default;

    void prepare();

    void draw(GLState& state);

private:
    GLVertexArray m_vao;
    GLBuffer m_vbo{ "plain_quad" };
};
