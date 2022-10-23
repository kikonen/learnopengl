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

    virtual const std::string str() const;

    virtual bool hasReflection() const = 0;
    virtual bool hasRefraction() const = 0;

    virtual Material* findMaterial(std::function<bool(const Material&)> fn) = 0;
    virtual void modifyMaterials(std::function<void(Material&)> fn) = 0;

    virtual void calculateVolume() = 0;

    virtual void prepare(const Assets& assets) = 0;
    virtual void prepareBuffers(MeshBuffers& curr) = 0;
    virtual void bind(
        const RenderContext& ctx,
        Shader* shader,
        bool bindMaterials) = 0;
    virtual void drawInstanced(const RenderContext& ctx, int instanceCount) = 0;

    void setVolume(std::unique_ptr<Volume> volume);
    const Volume* getVolume() const;

public:
    std::string m_name;

    MeshBuffers m_buffers;

private:
    std::unique_ptr<Volume> m_volume;
};
