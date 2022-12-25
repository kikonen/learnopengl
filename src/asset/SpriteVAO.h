#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class Batch;

class SpriteVAO {
public:
    GLVertexArray* prepare(Batch& batch);

private:
    void prepareVAO(
        GLVertexArray& vao,
        GLBuffer& vbo);

    void prepareVBO(
        GLBuffer& vbo);

private:
    bool m_prepared = false;

    std::unique_ptr<GLVertexArray> m_vao;
    GLBuffer m_vbo;
};
