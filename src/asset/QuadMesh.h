#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "QuadVBO.h"

class QuadMesh final : public Mesh
{
public:
    QuadMesh(const std::string& name);
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

    void drawInstanced(const RenderContext& ctx, int instanceCount) noexcept override;

private:
    void prepareBuffers(MeshBuffers& curr);
    void prepareVBO(MeshBuffers& curr);

public:
    Material m_material;

private:
    bool m_prepared = false;

    QuadVBO m_vertexVBO;
};
