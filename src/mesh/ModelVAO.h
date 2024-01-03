#pragma once

#include <mutex>
#include <atomic>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "mesh/VertexEntry.h"
#include "mesh/IndexEntry.h"


class Batch;
class UpdateContext;

namespace mesh {
    class ModelMeshVBO;

    class ModelVAO {
    public:
        ModelVAO() {};
        ~ModelVAO() = default;

        kigl::GLVertexArray* prepare();

        // @return VAO for mesh
        kigl::GLVertexArray* registerModel(ModelMeshVBO& meshVBO);

        void updateRT(const UpdateContext& ctx);

    private:
        void prepareVAO(
            kigl::GLVertexArray& vao,
            kigl::GLBuffer& positionVbo,
            kigl::GLBuffer& vertexVbo,
            kigl::GLBuffer& ebo);

        void updatePositionBuffer();
        void updateVertexBuffer();
        void updateIndexBuffer();

    private:
        bool m_prepared = false;

        std::atomic<bool> m_dirty;
        std::mutex m_lock{};

        std::unique_ptr<kigl::GLVertexArray> m_vao;

        kigl::GLBuffer m_positionVbo{ "positionVBO" };
        kigl::GLBuffer m_vertexVbo{ "vertexVBO" };

        kigl::GLBuffer m_ebo{ "modelEBO" };

        std::vector<PositionEntry> m_positionEntries;
        std::vector<VertexEntry> m_vertexEntries;
        std::vector<IndexEntry> m_indexEntries;

        size_t m_lastPositionSize = 0;
        size_t m_lastVertexSize = 0;
        size_t m_lastIndexSize = 0;
    };
}
