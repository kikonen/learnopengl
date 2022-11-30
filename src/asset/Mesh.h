#pragma once

#include <vector>
#include <functional>

#include "ki/GL.h"

#include "kigl/GLVertexArray.h"

#include "MeshBuffers.h"
#include "Assets.h"
#include "Material.h"
#include "Volume.h"
#include "AABB.h"
#include "MaterialVBO.h"

#include "registry/NodeRegistry.h"

class RenderContext;

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    virtual const std::string str() const;

    virtual void prepareVolume() = 0;
    virtual const AABB& calculateAABB() const = 0;

    virtual const std::vector<Material>& getMaterials() const = 0;

    virtual void prepare(
        const Assets& assets) = 0;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) = 0;

    virtual void prepareVAO(
        GLVertexArray& vao,
        MaterialVBO& materialVBO) = 0;

    virtual void drawInstanced(const RenderContext& ctx, int instanceCount) const  = 0;

    void setVolume(std::unique_ptr<Volume> volume);
    const Volume* getVolume() const;

    void setAABB(const AABB& aabb);
    const AABB& getAABB() const;

public:
    const int m_objectID;

protected:
    bool m_prepared = false;

private:
    AABB m_aabb{};
    std::unique_ptr<Volume> m_volume;

};
