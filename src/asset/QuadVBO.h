#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"


class QuadVBO {
public:
    void prepare();

    void prepareMesh(GLVertexArray& vao);

private:
    void prepareVAO(GLVertexArray& vao);
    void prepareVBO();

private:
    bool m_prepared = false;

    GLBuffer m_vbo;

};

