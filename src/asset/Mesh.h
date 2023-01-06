#pragma once

#include <vector>
#include <functional>

#include "ki/GL.h"

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"

#include "Assets.h"
#include "Material.h"
#include "MaterialEntry.h"
#include "Volume.h"
#include "AABB.h"

#include "MaterialVBO.h"

#include "registry/NodeRegistry.h"

class BatchRegistry;
class RenderContext;
class ModelRegistry;

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    virtual const std::string str() const;

    virtual void prepareVolume();
    virtual const AABB& calculateAABB() const = 0;

    virtual const std::vector<Material>& getMaterials() const = 0;

    // @return VAO for mesh
    virtual GLVertexArray* prepare(
        const Assets& assets,
        BatchRegistry& batchRegistry,
        ModelRegistry& modelRegistry) = 0;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) = 0;

    virtual void prepareVAO(
        GLVertexArray& vao,
        backend::DrawOptions& drawOptions) = 0;

    void setVolume(std::unique_ptr<Volume> volume);
    const Volume* getVolume() const;

    void setAABB(const AABB& aabb);
    const AABB& getAABB() const;

public:
    const int m_objectID;

protected:
    bool m_prepared = false;

    GLVertexArray* m_vao{ nullptr };

private:
    AABB m_aabb{};
    std::unique_ptr<Volume> m_volume;
};
