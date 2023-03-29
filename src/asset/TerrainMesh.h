#pragma once

#include <string>

#include "Assets.h"
#include "Material.h"
#include "Mesh.h"

class TerrainMesh final : public Mesh
{
    friend class TerrainMaterialInit;

public:
    TerrainMesh();
    virtual ~TerrainMesh();

    virtual const std::string str() const noexcept override;

    virtual const AABB calculateAABB() const override;

    const std::vector<Material>& getMaterials() const override;

    virtual GLVertexArray* prepare(
        const Assets& assets,
        Registry* registry) override;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) override;

    virtual void prepareDrawOptions(
        backend::DrawOptions& drawOptions) override;

protected:
    std::vector<Material> m_material;

};
