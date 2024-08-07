#pragma once

#include <string>

#include "mesh/Mesh.h"

#include "mesh/Index.h"

#include "mesh/Vertex.h"

namespace mesh {
    class TextMesh final : public Mesh
    {
    public:
        TextMesh();
        virtual ~TextMesh();

        virtual std::string str() const noexcept override;

        void clear();

        virtual AABB calculateAABB(const glm::mat4& transform) const noexcept override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) override;

        uint32_t getBaseVertex() const noexcept {
            return m_vboIndex;
        }

        inline uint32_t getBaseIndex() const noexcept {
            return m_eboIndex;
        }

        inline uint32_t getIndexCount() const noexcept {
            return static_cast<uint32_t>(m_indeces.size());
        }

    public:
        std::vector<Vertex> m_vertices;
        std::vector<mesh::Index32> m_indeces;

        std::vector<glm::vec2> m_atlasCoords;

        uint32_t m_maxSize{ 0 };

        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };
    };
}
