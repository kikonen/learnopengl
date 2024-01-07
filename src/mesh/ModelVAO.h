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

struct UpdateContext;

namespace mesh {
    class ModelVBO;

    class ModelVAO {
    public:
        ModelVAO(std::string_view name);
        ~ModelVAO() = default;

        void prepare(std::string_view name);
        void clear();

        void bind(kigl::GLState& state);
        void unbind(kigl::GLState& state);

        // @return VBO for model mesh
        kigl::GLVertexArray* registerModel(ModelVBO& modelVBO);

        const kigl::GLVertexArray* getVAO() const
        {
            return m_vao.get();
        }

        void updateRT();

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
        std::string m_name;

        std::atomic<bool> m_dirty;
        std::mutex m_lock{};

        std::unique_ptr<kigl::GLVertexArray> m_vao;

        kigl::GLBuffer m_positionVbo;
        kigl::GLBuffer m_vertexVbo;

        kigl::GLBuffer m_ebo;

        std::vector<PositionEntry> m_positionEntries;
        std::vector<VertexEntry> m_vertexEntries;
        std::vector<IndexEntry> m_indexEntries;

        size_t m_lastPositionSize = 0;
        size_t m_lastVertexSize = 0;
        size_t m_lastIndexSize = 0;
    };
}
