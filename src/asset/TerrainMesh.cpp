#include "TerrainMesh.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"
#include "asset/TerrainMaterialInit.h"
#include "asset/TerrainVAO.h"

#include "scene/RenderContext.h"


namespace {

    const AABB TERRAIN_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };

    TerrainVAO terrainVAO;
}


TerrainMesh::TerrainMesh()
    : Mesh()
{
}

TerrainMesh::~TerrainMesh()
{
}

const std::string TerrainMesh::str() const noexcept
{
    return fmt::format("<TERRAIN: {}>", m_objectID);
}

const AABB TerrainMesh::calculateAABB() const
{
    return TERRAIN_AABB;
}

const std::vector<Material>& TerrainMesh::getMaterials() const
{
    return m_material;
}

GLVertexArray* TerrainMesh::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return m_vao;
    m_prepared = true;

    m_vao = terrainVAO.prepare();
    return m_vao;
}

void TerrainMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    TerrainMaterialInit init;
    init.prepare(*this, materialVBO);
}

void TerrainMesh::prepareVAO(
    GLVertexArray& vao,
    backend::DrawOptions& drawOptions)
{
    drawOptions.type = backend::DrawOptions::Type::arrays;
    drawOptions.mode = GL_PATCHES;
    drawOptions.indexFirst = 0;
    drawOptions.indexCount = 1;
}
