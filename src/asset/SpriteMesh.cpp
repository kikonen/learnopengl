#include "SpriteMesh.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"
#include "asset/SpriteMaterialInit.h"
#include "MaterialEntry.h"

#include "scene/RenderContext.h"


namespace {

    const AABB SPRITE_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };

}


SpriteMesh::SpriteMesh()
    : Mesh()
{
}

SpriteMesh::~SpriteMesh()
{
}

const std::string SpriteMesh::str() const
{
    return fmt::format("<SPRITE: {}>", m_objectID);
}

const AABB& SpriteMesh::calculateAABB() const
{
    return SPRITE_AABB;
}

const std::vector<Material>& SpriteMesh::getMaterials() const
{
    return { m_material };
}

void SpriteMesh::prepare(
    const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;
}

void SpriteMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    SpriteMaterialInit init;
    init.prepare(*this, materialVBO);
}

void SpriteMesh::prepareVAO(
    GLVertexArray& vao)
{
    // nothing
}

void SpriteMesh::drawInstanced(const RenderContext& ctx, int instanceCount) const
{
    //glDrawArraysInstanced(GL_POINTS, 0, 1, instanceCount);

    int baseInstance = 0;

    glDrawArraysInstancedBaseInstance(
        GL_POINTS,
        0,
        1,
        instanceCount,
        baseInstance);
}
