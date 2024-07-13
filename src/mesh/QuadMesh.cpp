#include "QuadMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/vao/QuadVAO.h"


namespace {
    constexpr int INDEX_COUNT = 4;

    glm::vec3 AABB_VERTICES[] = {
        glm::vec3{ -1.f, -1.f, 0.f },
        glm::vec3{ 1.f, 1.f, 0.f },
    };

    // NOTE KI plane, only xy
    const AABB QUAD_AABB = {
        glm::vec3{ -1.f, -1.f, 0.f },
        glm::vec3{ 1.f, 1.f, 0.f },
        true };

    mesh::QuadVAO quadVAO;
}

namespace mesh {
    QuadMesh::QuadMesh()
        : Mesh("quad")
    {
    }

    QuadMesh::~QuadMesh()
    {
    }

    std::string QuadMesh::str() const noexcept
    {
        return fmt::format("<QUAD: id={}>", m_id);
    }

    AABB QuadMesh::calculateAABB(const glm::mat4& transform) const noexcept
    {
        AABB aabb{ true };

        for (auto&& vertex : AABB_VERTICES)
        {
            const auto& pos = transform * glm::vec4(vertex, 1.f);
            aabb.minmax(pos);
        }

        aabb.updateVolume();

        return aabb;
    }

    const kigl::GLVertexArray* QuadMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vao = quadVAO.prepare();
        return m_vao;
    }

    void QuadMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_lod.m_indexCount = INDEX_COUNT;

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::arrays;
        drawOptions.m_mode = backend::DrawOptions::Mode::triangle_strip;
    }
}
