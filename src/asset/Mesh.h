#pragma once

#include <functional>

#include "ki/GL.h"

#include "MeshBuffers.h"
#include "Assets.h"
#include "Material.h"
#include "Volume.h"

#include "scene/RenderContext.h"

class Mesh
{
public:
    Mesh();
    Mesh(const std::string& name);
    virtual ~Mesh();

    virtual std::string str();

    virtual bool hasReflection() = 0;
    virtual bool hasRefraction() = 0;

    virtual Material* findMaterial(std::function<bool(const Material&)> fn) = 0;
    virtual void modifyMaterials(std::function<void(Material&)> fn) = 0;

    virtual void prepare(const Assets& assets) = 0;
    virtual void prepareBuffers(MeshBuffers& curr) = 0;
    virtual void bind(const RenderContext& ctx, Shader* shader) = 0;
    virtual void draw(const RenderContext& ctx) = 0;
    virtual void drawInstanced(const RenderContext& ctx, int instanceCount) = 0;


public:
    std::string m_name;

    MeshBuffers m_buffers;

    std::unique_ptr<Volume> m_volume;
};

