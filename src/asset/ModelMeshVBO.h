#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "VertexEntry.h"
#include "IndexEntry.h"

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
        ModelMesh& mesh);

    void prepareIndex(
        ModelMesh& mesh);

public:
    GLBuffer* m_vbo{ nullptr };

    // NOTE KI absolute offset into vbo
    int m_vertexOffset = 0;
    // NOTE KI absolute offset into vbo
    int m_indexOffset = 0;

    std::vector<VertexEntry> m_vertexEntries;
    std::vector<IndexEntry> m_indexEntries;

private:
    bool m_prepared = false;
};
