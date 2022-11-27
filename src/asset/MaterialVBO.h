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
    void prepareListVBO(
        ModelMesh& mesh);

    void prepareSingleVBO(
        ModelMesh& mesh);

private:
    bool m_prepared = false;
    bool m_single = false;
    int m_vertexCount = 0;

    GLBuffer m_vbo;
};
