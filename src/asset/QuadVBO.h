#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"


class QuadVBO {
public:
    void prepare();

    void prepareVAO(GLVertexArray& vao);

private:
    void prepareVBO();

private:
    bool m_prepared = false;

    GLBuffer m_vbo;

};
