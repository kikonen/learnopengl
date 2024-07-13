#pragma once

#include <string>

#include "mesh/Mesh.h"

namespace mesh {
    class QuadMesh final : public Mesh
    {
        friend class QuadMaterialInit;

    public:
        QuadMesh();
        virtual ~QuadMesh();

        virtual std::string str() const noexcept override;

        virtual AABB calculateAABB(const glm::mat4& transform) const noexcept override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) override;

    protected:
        std::vector<Material> m_material;

    };
}
