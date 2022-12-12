#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class ModelMesh;

//
// https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
//
class ModelMeshVBO {
public:
    ModelMeshVBO();
    ~ModelMeshVBO();

    void prepare(
        ModelMesh& mesh);

    void prepareVAO(
        GLVertexArray& vao);

private:
    void prepareBuffers(
        ModelMesh& mesh);

    void prepareVertex(
        ModelMesh& mesh,
        unsigned char* data,
        int offset);

    void prepareIndex(
        ModelMesh& mesh,
        unsigned char* data,
        int offset);

public:
    int m_vertexOffset = 0;
    int m_indexOffset = 0;

    int m_baseVertex = 0;

private:
    bool m_prepared = false;

    GLBuffer m_buffer;
};
