#include "TerrainMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

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

    std::string TerrainMesh::str() const noexcept
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
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vao = terrainVAO.prepare();
        return m_vao;
    }

    void TerrainMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::arrays;
        drawOptions.m_mode = GL_PATCHES;

        auto& lod = drawOptions.m_lod;
        lod.m_indexCount = 1;
    }
}
