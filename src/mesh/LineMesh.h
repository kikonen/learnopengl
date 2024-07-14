#pragma once

#include <string>

#include "mesh/Mesh.h"
#include "mesh/PrimitiveType.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace mesh {
    //
    // https://stackoverflow.com/questions/39588937/when-drawing-lines-with-vbo-how-do-i-specify-indices
    //
    class LineMesh final : public Mesh
    {
    public:
        LineMesh();
        virtual ~LineMesh();

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
            return m_eboIndex * 3;
        }

        inline uint32_t getIndexCount() const noexcept {
            return static_cast<uint32_t>(m_indeces.size() * 3);
        }

    public:
        mesh::PrimitiveType m_type;

        std::vector<Vertex> m_vertices;
        std::vector<mesh::Index> m_indeces;

        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };
    };
}
