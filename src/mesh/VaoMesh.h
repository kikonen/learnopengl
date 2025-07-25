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

        void setRigTransform(const glm::mat4& rigTransform) {
            m_rigTransform = rigTransform;
            m_inverseRigTransform = glm::inverse(rigTransform);
        }

        virtual const kigl::GLVertexArray* getVAO() const noexcept override
        {
            return m_vao;
        }

        virtual AABB calculateAABB(const glm::mat4& transform) const override;

        virtual size_t getDefinedVertexCount() const noexcept override
        {
            return m_vertices.size();
        }

        virtual size_t getDefinedIndexCount() const noexcept override
        {
            return m_indeces.size();
        }

        void setupVertexCounts()
        {
            m_vertexCount = static_cast<uint32_t>(m_vertices.size());
            m_indexCount = static_cast<uint32_t>(m_indeces.size());
        }

    public:
        std::vector<mesh::Vertex> m_vertices;
        std::vector<mesh::Index32> m_indeces;

        // NOTE KI for debug
        std::string m_rigJointName;

        glm::mat4 m_inverseRigTransform{ 1.f };

    protected:
        const kigl::GLVertexArray* m_vao{ nullptr };

    };
}
