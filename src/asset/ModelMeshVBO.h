#pragma once

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

private:
    void prepareBuffers(
        ModelMesh& mesh);

    void prepareVertex(
        ModelMesh& mesh);

    void prepareIndex(
        ModelMesh& mesh);

public:
    // NOTE KI absolute offset into vbo
    int m_vertexOffset = 0;
    // NOTE KI absolute offset into vbo
    int m_indexOffset = 0;

    std::vector<VertexEntry> m_vertexEntries;
    std::vector<IndexEntry> m_indexEntries;

    bool m_single = false;

private:
    bool m_prepared = false;
};
