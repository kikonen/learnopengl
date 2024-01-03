#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class Batch;

class QuadVAO {
public:
    kigl::GLVertexArray* prepare();

private:
    void prepareVAO(
        kigl::GLVertexArray& vao,
        kigl::GLBuffer& vbo);

    void prepareVBO(
        kigl::GLBuffer& vbo);

private:
    bool m_prepared = false;

    std::unique_ptr<kigl::GLVertexArray> m_vao;
    kigl::GLBuffer m_vbo{ "quadVBO" };

};
