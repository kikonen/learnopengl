#pragma once

#include <string>

#include "mesh/Mesh.h"

namespace mesh {
    class TextMesh final : public Mesh
    {
    public:
        TextMesh();
        virtual ~TextMesh();

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
