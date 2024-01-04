#pragma once

#include <mutex>
#include <atomic>
#include <string>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "mesh/PositionEntry.h"
#include "mesh/VertexEntry.h"
#include "mesh/IndexEntry.h"

namespace kigl {
    class GLState;
}

class Batch;
class UpdateContext;

namespace mesh {
    class ModelVBO;

    class ModelVAO {
    public:
        ModelVAO();
        ~ModelVAO() = default;

        void prepare(std::string_view name);
        void clear();

        void bind(kigl::GLState& state);
        void unbind(kigl::GLState& state);

        // @return VBO for model mesh
        kigl::GLVertexArray* registerModel(ModelVBO& modelVBO);

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
