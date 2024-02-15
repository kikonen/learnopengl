#pragma once

#include <string>

#include "asset/Material.h"

#include "mesh/Mesh.h"

namespace mesh {
    class QuadMesh final : public Mesh
    {
        friend class QuadMaterialInit;

    public:
        QuadMesh();
        virtual ~QuadMesh();

        virtual std::string str() const noexcept override;

        virtual const AABB calculateAABB() const override;

        const std::vector<Material>& getMaterials() const override;

        virtual kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLod(
            mesh::LodMesh& lodMesh) override;

        virtual void prepareDrawOptions(
            backend::DrawOptions& drawOptions) override;

    protected:
        std::vector<Material> m_material;

    };
}
