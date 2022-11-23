#pragma once

#include <functional>

#include "ki/GL.h"

#include "MeshBuffers.h"
#include "Assets.h"
#include "Material.h"
#include "Volume.h"
#include "AABB.h"

#include "registry/NodeRegistry.h"

class RenderContext;

class Mesh
{
public:
    Mesh();
    Mesh(const std::string& name);
    virtual ~Mesh();

    virtual const std::string str() const;

    virtual Material* findMaterial(std::function<bool(const Material&)> fn) = 0;
    virtual void modifyMaterials(std::function<void(Material&)> fn) = 0;

    virtual void prepareVolume() = 0;
    virtual const AABB& calculateAABB() const = 0;

    virtual void prepare(
        const Assets& assets,
        NodeRegistry& registry) = 0;

    virtual void bind(
        const RenderContext& ctx,
        Shader* shader) noexcept  = 0;

    virtual void drawInstanced(const RenderContext& ctx, int instanceCount) noexcept  = 0;

    void setVolume(std::unique_ptr<Volume> volume);
    const Volume* getVolume() const;

    void setAABB(const AABB& aabb);
    const AABB& getAABB() const;

public:
    std::string m_name;

    MeshBuffers m_buffers;

private:
    AABB m_aabb;
    std::unique_ptr<Volume> m_volume;

};
