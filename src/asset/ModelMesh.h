#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Assets.h"
#include "Shader.h"

#include "Material.h"
#include "Shader.h"
#include "Vertex.h"
#include "Assets.h"
#include "Mesh.h"

class ModelMesh final : public Mesh {
public:
    ModelMesh(
        const std::string& name,
        const std::string& meshName);

    ModelMesh(
        const std::string& name,
        const std::string& meshName,
        const std::string& meshPath);

    virtual ~ModelMesh();

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
    void prepareEBO(MeshBuffers& curr);

public:
    int m_triCount = 0;
    std::vector<glm::uvec3> m_tris;
    std::vector<Vertex> m_vertices;

    std::vector<Material> m_materials;

    const std::string m_meshName;
    const std::string m_meshPath;

private:
    bool m_prepared = false;

    unsigned int m_materialsUboId = 0;
    unsigned int m_materialsUboSize = 0;
};
