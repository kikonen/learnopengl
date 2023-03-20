#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "asset/VertexEntry.h"
#include "asset/IndexEntry.h"


class Batch;
class ModelMeshVBO;
class UpdateContext;


class ModelVAO {
public:
    ModelVAO(bool singleMaterial);

    GLVertexArray* prepare();

    // @return VAO for mesh
    GLVertexArray* registerModel(ModelMeshVBO& meshVBO);

    void update(const UpdateContext& ctx);

private:
    void prepareVAO(
        GLVertexArray& vao,
        GLBuffer& vbo,
        GLBuffer& ebo);

    void updateVertexBuffer();
    void updateIndexBuffer();

private:
    const bool m_singleMaterial;

    bool m_prepared = false;

    std::unique_ptr<GLVertexArray> m_vao;
    GLBuffer m_vbo{ "modelVBO" };
    GLBuffer m_ebo{ "modelEBO" };

    std::vector<VertexEntry> m_vertexEntries;
    std::vector<IndexEntry> m_indexEntries;

    size_t m_lastVertexSize = 0;
    size_t m_lastIndexSize = 0;
};
