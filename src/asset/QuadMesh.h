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

    Material* findMaterial(std::function<bool(const Material&)> fn) override;
    void modifyMaterials(std::function<void(Material&)> fn) override;

    virtual void prepareVolume() override;
    virtual const AABB& calculateAABB() const override;

    void prepare(
        const Assets& assets,
        NodeRegistry& registry) override;

    void bind(
        const RenderContext& ctx,
        Shader* shader) noexcept override;


private:
    void prepareBuffers(MeshBuffers& curr);
    void prepareMaterialVBO(MeshBuffers& curr);

    void drawInstanced(const RenderContext& ctx, int instanceCount) const override;
public:
    Material m_material;

private:

};
