#include "QuadMesh.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"
#include "asset/MaterialEntry.h"
#include "asset/QuadMaterialInit.h"

#include "scene/RenderContext.h"

namespace {
    constexpr int INDEX_COUNT = 4;

    const AABB QUAD_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };

    QuadVBO m_quad;
}


QuadMesh::QuadMesh()
    : Mesh()
{
}

QuadMesh::~QuadMesh()
{
}

const std::string QuadMesh::str() const
{
    return fmt::format("<QUAD: {}>", m_objectID);
}

const AABB& QuadMesh::calculateAABB() const
{
    return QUAD_AABB;
}

const std::vector<Material>& QuadMesh::getMaterials() const
{
    return { m_material };
}

void QuadMesh::prepare(
    const Assets& assets,
    MeshRegistry& meshRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    m_quad.prepare();
}

void QuadMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    QuadMaterialInit init;
    init.prepare(*this, materialVBO);
}

void QuadMesh::prepareVAO(
    GLVertexArray& vao,
    backend::DrawOptions& drawOptions)
{
    m_quad.prepareVAO(vao);

    drawOptions.type = backend::DrawOptions::Type::arrays;
    drawOptions.mode = GL_TRIANGLE_STRIP;
    drawOptions.indexFirst = 0;
    drawOptions.indexCount = INDEX_COUNT;
}
