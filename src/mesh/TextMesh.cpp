#include "TextMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "mesh/MeshType.h"

#include "text/TextDraw.h"

namespace {
    // NOTE KI plane, only xy
    const AABB QUAD_AABB = {
        glm::vec3{ -1.f, -1.f, 0.f },
        glm::vec3{ 1.f, 1.f, 0.f },
        true };
}

namespace mesh {
    TextMesh::TextMesh()
        : Mesh()
    {
    }

    TextMesh::~TextMesh()
    {
    }

    const std::string TextMesh::str() const noexcept
    {
        return fmt::format("<TEXT: id={}>", m_id);
    }

    const AABB TextMesh::calculateAABB() const
    {
        return QUAD_AABB;
    }

    const std::vector<Material>& TextMesh::getMaterials() const
    {
        return m_material;
    }

    kigl::GLVertexArray* TextMesh::prepareRT(
        const Assets& assets,
        Registry* registry)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        return m_vao;
    }

    void TextMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.type = backend::DrawOptions::Type::elements;
        drawOptions.mode = GL_TRIANGLES;
        drawOptions.indexFirst = 0;
        drawOptions.indexCount = 0;
    }
}
