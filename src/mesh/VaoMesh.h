#pragma once

#include <string_view>

#include "Mesh.h"

namespace mesh {
    // Mesh which uses VAO/VBO to store indeces
    class VaoMesh : public Mesh
    {
    public:
        VaoMesh(std::string_view name);
        virtual ~VaoMesh();

        virtual const kigl::GLVertexArray* getVAO() const noexcept override
        {
            return m_vao;
        }

        virtual AABB calculateAABB(const glm::mat4& transform) const override;

        void setupVertexCounts()
        {
        }

        uint32_t getVertexCount() const noexcept override
        {
            return static_cast<uint32_t>(m_vertices.size());
        }

        uint32_t getIndexCount() const noexcept override
        {
            return static_cast<uint32_t>(m_indeces.size());
        }

        const std::vector<mesh::Vertex>& getVertices() const noexcept
        {
            return m_vertices;
        }

        const std::vector<mesh::Index32>& getIndeces() const noexcept
        {
            return m_indeces;
        }

        std::vector<mesh::Vertex>& modifyVertices() noexcept
        {
            return m_vertices;
        }

        std::vector<mesh::Index32>& modifyIndeces() noexcept
        {
            return m_indeces;
        }

        int32_t getRigNodeIndex() const noexcept
        {
            return m_rigNodeIndex;
        }

        void setRigNodeIndex(int32_t rigNodeIndex) noexcept
        {
            m_rigNodeIndex = rigNodeIndex;
        }

    protected:
        std::vector<mesh::Vertex> m_vertices;
        std::vector<mesh::Index32> m_indeces;

        // NOTE KI debug/troubleshoot only
        int32_t m_rigNodeIndex{ -1 };

    protected:
        const kigl::GLVertexArray* m_vao{ nullptr };
    };
}
