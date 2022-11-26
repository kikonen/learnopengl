#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"


class QuadVBO {
public:
    void prepare(GLVertexArray& vao);

private:
    void prepareVBO(GLVertexArray& vao);

private:
    bool m_prepared = false;

    GLBuffer m_vbo;

};

