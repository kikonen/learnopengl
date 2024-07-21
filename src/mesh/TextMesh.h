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

    public:
        std::vector<glm::vec2> m_atlasCoords;

        uint32_t m_maxSize{ 0 };
    };
}
