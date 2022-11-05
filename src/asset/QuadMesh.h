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

    virtual const std::string str() const override;

    bool hasReflection() const override;
    bool hasRefraction() const override;

    Material* findMaterial(std::function<bool(const Material&)> fn) override;
    void modifyMaterials(std::function<void(Material&)> fn) override;

    virtual void calculateVolume() override;

    void prepare(const Assets& assets) override;
    void bind(
        const RenderContext& ctx,
        Shader* shader,
        bool bindMaterials) noexcept override;
    void drawInstanced(const RenderContext& ctx, int instanceCount) noexcept override;

private:
    void prepareMaterials(const Assets& assets);
    void prepareBuffers(MeshBuffers& curr);
    void prepareVBO(MeshBuffers& curr);

public:
    Material m_material;
    std::vector<GLuint> m_textureIDs;
    unsigned int m_unitIndexFirst = 0;

private:
    bool m_prepared = false;

    unsigned int m_materialsUboSize = 0;
    unsigned int m_materialsUboId = 0;
};
