#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

class QuadMesh final : public Mesh
{
public:
    QuadMesh();
    virtual ~QuadMesh();

    virtual const std::string str() const override;

    virtual void prepareVolume() override;
    virtual const AABB& calculateAABB() const override;

    const std::vector<Material>& getMaterials() const override;

    virtual void prepare(
        const Assets& assets) override;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) override;

    virtual void prepareVAO(
        GLVertexArray& vao,
        MaterialVBO& materialVBO) override;

    void drawInstanced(const RenderContext& ctx, int instanceCount) const override;

private:
    void prepareMaterialVBO(
        MaterialVBO& materialVBO);

protected:
    Material m_material;

};
