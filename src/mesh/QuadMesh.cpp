#include "QuadMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/MeshType.h"
#include "mesh/QuadVAO.h"


namespace {
    constexpr int INDEX_COUNT = 4;

    // NOTE KI plane, only xy
    const AABB QUAD_AABB = {
        glm::vec3{ -1.f, -1.f, 0.f },
        glm::vec3{ 1.f, 1.f, 0.f },
        true };

    mesh::QuadVAO quadVAO;
}

namespace mesh {
    QuadMesh::QuadMesh()
        : Mesh()
    {
    }

    QuadMesh::~QuadMesh()
    {
    }

    std::string QuadMesh::str() const noexcept
    {
        return fmt::format("<QUAD: id={}>", m_id);
    }

    const AABB QuadMesh::calculateAABB() const
    {
        return QUAD_AABB;
    }

    const std::vector<Material>& QuadMesh::getMaterials() const
    {
        return m_material;
    }

    kigl::GLVertexArray* QuadMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vao = quadVAO.prepare();
        return m_vao;
    }

    void QuadMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::arrays;
        drawOptions.m_mode = GL_TRIANGLE_STRIP;

        auto& lod = drawOptions.m_lod;
        lod.m_indexCount = INDEX_COUNT;
    }
}
