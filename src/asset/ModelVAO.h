#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "asset/VertexEntry.h"
#include "asset/IndexEntry.h"


class Batch;
class ModelMeshVBO;

class ModelVAO {
public:
    ModelVAO(bool singleMaterial);

    GLVertexArray* prepare(Batch& batch);

    // @return VAO for mesh
    GLVertexArray* registerModel(ModelMeshVBO& meshVBO);

private:
    void prepareVAO(
        GLVertexArray& vao,
        GLBuffer& vbo,
        GLBuffer& ebo);

private:
    const bool m_singleMaterial;

    bool m_prepared = false;

    std::unique_ptr<GLVertexArray> m_vao;
    GLBuffer m_vbo;
    GLBuffer m_ebo;

    std::vector<VertexEntry> m_vertexEntries;
    std::vector<IndexEntry> m_indexEntries;
};
