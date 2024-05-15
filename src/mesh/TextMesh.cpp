#include "TextMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "text/TextDraw.h"

namespace {
}

namespace mesh {
    TextMesh::TextMesh()
        : Mesh("text")
    {
    }

    TextMesh::~TextMesh()
    {
    }

    std::string TextMesh::str() const noexcept
    {
        return fmt::format("<TEXT: id={}>", m_id);
    }

    void TextMesh::clear() {
        m_positions.clear();
        m_normals.clear();
        m_texCoords.clear();
        m_atlasCoords.clear();
        m_indeces.clear();
    }

    const AABB TextMesh::calculateAABB() const
    {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

        for (auto&& vertex : m_positions)
        {
            minAABB.x = std::min(minAABB.x, vertex.x);
            minAABB.y = std::min(minAABB.y, vertex.y);
            minAABB.z = std::min(minAABB.z, vertex.z);

            maxAABB.x = std::max(maxAABB.x, vertex.x);
            maxAABB.y = std::max(maxAABB.y, vertex.y);
            maxAABB.z = std::max(maxAABB.z, vertex.z);
        }

        return { minAABB, maxAABB, false };
    }

    const kigl::GLVertexArray* TextMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        return m_vao;
    }

    void TextMesh::prepareLod(
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_lod.m_baseVertex = 0;
        lodMesh.m_lod.m_baseIndex = 0;
        lodMesh.m_lod.m_indexCount = 0;
    }

    void TextMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = GL_TRIANGLES;
    }
}
