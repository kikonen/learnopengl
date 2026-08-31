#pragma once

#include <string>

#include "mesh/VaoMesh.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace mesh {
    class TextMesh final : public VaoMesh
    {
    public:
        TextMesh();
        virtual ~TextMesh();

        void clear();

        virtual const kigl::GLVertexArray* prepareVAO() override;
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) override;

        const std::vector<glm::vec2>& getAtlasCoords() const noexcept
        {
            return m_atlasCoords;
        }

        std::vector<glm::vec2>& modifyAtlasCoords() noexcept
        {
            return m_atlasCoords;
        }

        uint32_t getMaxSize() const noexcept
        {
            return m_maxSize;
        }

        void setMaxSize(uint32_t maxSize) noexcept
        {
            m_maxSize = maxSize;
        }

    private:
        std::vector<glm::vec2> m_atlasCoords;

        uint32_t m_maxSize{ 0 };
    };
}
