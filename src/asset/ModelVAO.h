#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "asset/VertexEntry.h"
#include "asset/IndexEntry.h"


class Batch;
class ModelMeshVBO;
class UpdateContext;
class UpdateViewContext;


class ModelVAO {
public:
    ModelVAO() {};
    ~ModelVAO() = default;

    GLVertexArray* prepare();

    // @return VAO for mesh
    GLVertexArray* registerModel(ModelMeshVBO& meshVBO);

    void updateView(const UpdateViewContext& ctx);

private:
    void prepareVAO(
        GLVertexArray& vao,
        GLBuffer& positionVbo,
        GLBuffer& vertexVbo,
        GLBuffer& ebo);

    void updatePositionBuffer();
    void updateVertexBuffer();
    void updateIndexBuffer();

private:
    bool m_prepared = false;

    std::unique_ptr<GLVertexArray> m_vao;

    GLBuffer m_positionVbo{ "positionVBO" };
    GLBuffer m_vertexVbo{ "vertexVBO" };

    GLBuffer m_ebo{ "modelEBO" };

    std::vector<PositionEntry> m_positionEntries;
    std::vector<VertexEntry> m_vertexEntries;
    std::vector<IndexEntry> m_indexEntries;

    size_t m_lastPositionSize = 0;
    size_t m_lastVertexSize = 0;
    size_t m_lastIndexSize = 0;
};
