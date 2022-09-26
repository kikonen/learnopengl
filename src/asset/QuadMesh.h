#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

#include "scene/RenderContext.h"

class QuadMesh final : public Mesh
{
public:
    QuadMesh(const std::string& modelName);
    virtual ~QuadMesh();

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
    Material material;
    std::vector<GLuint> textureIDs;

private:
    unsigned int materialsUboSize = 0;
    unsigned int materialsUboId = 0;
};
