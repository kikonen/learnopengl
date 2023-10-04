#pragma once

#include <vector>
#include <functional>

#include "kigl/kigl.h"

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"

#include "Assets.h"
#include "Material.h"
#include "Volume.h"
#include "AABB.h"

#include "MaterialVBO.h"

#include "registry/Registry.h"

class Registry;


class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    virtual const std::string str() const noexcept;

    virtual bool isValid() const noexcept { return true; }
    virtual void prepareVolume();
    virtual const AABB calculateAABB() const = 0;

    virtual const std::vector<Material>& getMaterials() const = 0;

    // @return VAO for mesh
    virtual GLVertexArray* prepare(
        const Assets& assets,
        Registry* registry) = 0;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) = 0;

    virtual void prepareDrawOptions(
        backend::DrawOptions& drawOptions) = 0;

    void setAABB(const AABB& aabb) {
        m_aabb = aabb;
    }

    const AABB& getAABB() const {
        return m_aabb;
    }

public:
    const int m_objectID;

protected:
    bool m_prepared = false;

    GLVertexArray* m_vao{ nullptr };

private:
    AABB m_aabb{};
    std::unique_ptr<Volume> m_volume;
};
