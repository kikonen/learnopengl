#include "TerrainMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/TerrainMaterialInit.h"
#include "mesh/TerrainVAO.h"


namespace {

    const AABB TERRAIN_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };

    mesh::TerrainVAO terrainVAO;
}


namespace mesh {
    TerrainMesh::TerrainMesh()
        : Mesh()
    {
    }

    TerrainMesh::~TerrainMesh()
    {
    }

    const std::string TerrainMesh::str() const noexcept
    {
        return fmt::format("<TERRAIN: id={}>", m_id);
    }

    const AABB TerrainMesh::calculateAABB() const
    {
        return TERRAIN_AABB;
    }

    const std::vector<Material>& TerrainMesh::getMaterials() const
    {
        return m_material;
    }

    kigl::GLVertexArray* TerrainMesh::prepareRT(
        const Assets& assets,
        Registry* registry)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vao = terrainVAO.prepare();
        return m_vao;
    }

    void TerrainMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.type = backend::DrawOptions::Type::arrays;
        drawOptions.mode = GL_PATCHES;
        drawOptions.indexFirst = 0;
        drawOptions.indexCount = 1;
    }
}
