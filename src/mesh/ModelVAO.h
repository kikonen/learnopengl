#pragma once

#include <mutex>
#include <atomic>
#include <string>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "mesh/PositionEntry.h"
#include "mesh/NormalEntry.h"
#include "mesh/TextureEntry.h"
#include "mesh/IndexEntry.h"

struct UpdateContext;

namespace mesh {
    class ModelVBO;

    class ModelVAO {
    public:
        ModelVAO(std::string_view name);
        ~ModelVAO();

        void prepare(std::string_view name);
        void clear();

        void bind();
        void unbind();

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
            kigl::GLBuffer& normalVbo,
            kigl::GLBuffer& textureVbo,
            kigl::GLBuffer& ebo);

        void updatePositionBuffer();
        void updateNormalBuffer();
        void updateTextureBuffer();
        void updateIndexBuffer();

    private:
        bool m_prepared = false;
        std::string m_name;

        std::unique_ptr<kigl::GLVertexArray> m_vao;

        kigl::GLBuffer m_positionVbo;
        kigl::GLBuffer m_normalVbo;
        kigl::GLBuffer m_textureVbo;

        kigl::GLBuffer m_ebo;

        std::vector<PositionEntry> m_positionEntries;
        std::vector<NormalEntry> m_normalEntries;
        std::vector<TextureEntry> m_textureEntries;
        std::vector<IndexEntry> m_indexEntries;

        size_t m_lastPositionSize = 0;
        size_t m_lastNormalSize = 0;
        size_t m_lastTextureSize = 0;
        size_t m_lastIndexSize = 0;
    };
}
