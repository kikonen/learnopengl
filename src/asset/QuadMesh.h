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
    QuadMesh(const std::string& name);
    virtual ~QuadMesh();

    virtual std::string str() override;

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
    Material m_material;
    std::vector<GLuint> m_textureIDs;

private:
    bool m_prepared = false;

    unsigned int m_materialsUboSize = 0;
    unsigned int m_materialsUboId = 0;
};
