#pragma once

#include <string>

#include "animation/VertexBone.h"

#include "mesh/Mesh.h"
#include "mesh/PrimitiveType.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace animation {
    struct RigContainer;
    struct VertexBone;
}

namespace mesh {
    //
    // **References**
    // https://www.khronos.org/opengl/wiki/Primitive
    // https://stackoverflow.com/questions/39588937/when-drawing-lines-with-vbo-how-do-i-specify-indices
    //
    class PrimitiveMesh final : public Mesh
    {
    public:
        PrimitiveMesh();
        PrimitiveMesh(std::string_view name);
        virtual ~PrimitiveMesh();

        virtual std::string str() const noexcept override;

        void clear();

        virtual AABB calculateAABB(const glm::mat4& transform) const noexcept override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual std::shared_ptr<animation::RigContainer> getRigContainer() override
        {
            return m_rig;
        }

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
        PrimitiveType m_type;

        std::vector<Vertex> m_vertices;
        std::vector<Index32> m_indeces;

        std::vector<animation::VertexBone> m_vertexBones;

        std::shared_ptr<animation::RigContainer> m_rig;

        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };
    };
}
