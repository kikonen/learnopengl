#include "SpriteMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/vao/SpriteVAO.h"


namespace {

    // NOTE KI plane, only xy
    const AABB QUAD_AABB = {
        glm::vec3{ -1.f, -1.f, 0.f },
        glm::vec3{ 1.f, 1.f, 0.f },
        true };

    mesh::SpriteVAO spriteVAO;
}

namespace mesh {
    SpriteMesh::SpriteMesh()
        : Mesh()
    {
    }

    SpriteMesh::~SpriteMesh()
    {
    }

    std::string SpriteMesh::str() const noexcept
    {
        return fmt::format("<SPRITE: id={}>", m_id);
    }

    const AABB SpriteMesh::calculateAABB() const
    {
        return QUAD_AABB;
    }

    const kigl::GLVertexArray* SpriteMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vao = spriteVAO.prepare();
        return m_vao;
    }

    void SpriteMesh::prepareLod(
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_lod.m_indexCount = 1;
    }

    void SpriteMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::arrays;
        drawOptions.m_mode = GL_POINTS;
    }
}
