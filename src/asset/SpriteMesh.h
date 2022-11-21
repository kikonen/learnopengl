#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

class SpriteMesh final : public Mesh
{
public:
    SpriteMesh(const std::string& name);
    virtual ~SpriteMesh();

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
        Shader* shader,
        bool bindMaterials) noexcept override;

    void drawInstanced(const RenderContext& ctx, int instanceCount) noexcept override;

private:
    void prepareMaterials(
        const Assets& assets,
        NodeRegistry& registry);

    void prepareBuffers(MeshBuffers& curr);
    void prepareVBO(MeshBuffers& curr);

public:
    Material m_material;

private:
    bool m_prepared = false;

    unsigned int m_materialsUboSize = 0;
    unsigned int m_materialsUboId = 0;
};
