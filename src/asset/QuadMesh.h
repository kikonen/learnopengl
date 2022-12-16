#pragma once

#include <string>

#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

class QuadMesh final : public Mesh
{
    friend class QuadMaterialInit;

public:
    QuadMesh();
    virtual ~QuadMesh();

    virtual const std::string str() const override;

    virtual const AABB& calculateAABB() const override;

    const std::vector<Material>& getMaterials() const override;

    virtual void prepare(
        const Assets& assets,
        MeshRegistry& meshRegistry) override;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) override;

    virtual void prepareVAO(
        GLVertexArray& vao) override;

    void drawInstanced(const RenderContext& ctx, int instanceCount) const override;

protected:
    Material m_material;

};
