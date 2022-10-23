#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Assets.h"
#include "Shader.h"
#include "scene/RenderContext.h"

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

    bool hasReflection() const override;
    bool hasRefraction() const override;

    Material* findMaterial(std::function<bool(const Material&)> fn) override;
    void modifyMaterials(std::function<void(Material&)> fn) override;

    virtual void calculateVolume() override;

    void prepare(const Assets& assets) override;
    void prepareBuffers(MeshBuffers& curr) override;
    void bind(
        const RenderContext& ctx,
        Shader* shader,
        bool bindMaterials) override;
    void drawInstanced(const RenderContext& ctx, int instanceCount) override;

public:
    int m_triCount = 0;
    std::vector<glm::uvec3> m_tris;
    std::vector<Vertex> m_vertices;

    std::vector<Material> m_materials;
    std::vector<GLuint> m_textureIDs;
    unsigned int m_unitIndexFirst = 0;

    const std::string m_meshName;
    const std::string m_meshPath;

private:
    bool m_prepared = false;

    bool m_refraction = false;
    bool m_reflection = false;

    unsigned int m_materialsUboId = 0;
    unsigned int m_materialsUboSize = 0;
};
