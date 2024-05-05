#pragma once

#include <string>

#include "asset/Material.h"
#include "asset/Sprite.h"

#include "mesh/Mesh.h"

namespace mesh {
    class SpriteMesh final : public Mesh
    {
    public:
        SpriteMesh();
        virtual ~SpriteMesh();

        virtual std::string str() const noexcept override;

        virtual const AABB calculateAABB() const override;

        const std::vector<Material>& getMaterials() const override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLod(
            mesh::LodMesh& lodMesh) override;

        virtual void prepareDrawOptions(
            backend::DrawOptions& drawOptions) override;

    protected:
        std::vector<Material> m_material;

    };
}
