#include "TextMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "mesh/LodMesh.h"
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

    std::string TextMesh::str() const noexcept
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
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        return m_vao;
    }

    void TextMesh::prepareLod(
        mesh::LodMesh& lodMesh)
    {
    }

    void TextMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = GL_TRIANGLES;
    }
}
