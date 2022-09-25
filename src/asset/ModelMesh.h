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
        const std::string& modelName);

    ModelMesh(
        const std::string& modelName,
        const std::string& modelPath);

    virtual ~ModelMesh();

    bool hasReflection() override;
    bool hasRefraction() override;

    Material* findMaterial(std::function<bool(const Material&)> fn) override;
    void modifyMaterials(std::function<void(Material&)> fn) override;

    void prepare(const Assets& assets) override;
    void prepareBuffers(MeshBuffers& curr) override;
    void bind(const RenderContext& ctx, Shader* shader) override;
    void draw(const RenderContext& ctx) override;
    void drawInstanced(const RenderContext& ctx, int instanceCount) override;

public:
    int triCount = 0;
    std::vector<glm::uvec3> tris;
    std::vector<Vertex> vertices;

    std::vector<Material> materials;
    std::vector<GLuint> textureIDs;

    const std::string modelPath;

private:
    bool refraction = false;
    bool reflection = false;

    unsigned int  materialsUboId = 0;
    unsigned int materialsUboSize = 0;
};
