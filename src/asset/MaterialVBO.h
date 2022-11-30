#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class ModelMesh;

class MaterialVBO {
public:
    void prepare(
        ModelMesh& mesh);

    void prepareVAO(
        GLVertexArray& vao);

private:
    void prepareVBO(
        ModelMesh& mesh);

private:
    bool m_prepared = false;

    GLBuffer m_vbo;
};