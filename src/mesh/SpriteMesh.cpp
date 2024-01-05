#include "SpriteMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/MeshType.h"
#include "mesh/SpriteVAO.h"


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

    const std::string SpriteMesh::str() const noexcept
    {
        return fmt::format("<SPRITE: id={}>", m_id);
    }

    const AABB SpriteMesh::calculateAABB() const
    {
        return QUAD_AABB;
    }

    const std::vector<Material>& SpriteMesh::getMaterials() const
    {
        return m_material;
    }

    kigl::GLVertexArray* SpriteMesh::prepareRT(
        const Assets& assets,
        Registry* registry)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vao = spriteVAO.prepare();
        return m_vao;
    }

    void SpriteMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.type = backend::DrawOptions::Type::arrays;
        drawOptions.mode = GL_POINTS;
        drawOptions.indexFirst = 0;
        drawOptions.indexCount = 1;
    }
}