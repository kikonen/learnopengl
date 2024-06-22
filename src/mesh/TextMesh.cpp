#include "TextMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "text/TextDraw.h"

#include "text/vao/TextVAO.h"
#include "text/TextSystem.h"


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
        m_vertices.clear();
        m_atlasCoords.clear();
        m_indeces.clear();
    }

    AABB TextMesh::calculateAABB() const noexcept {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            aabb.minmax(vertex.pos);
        }

        return aabb;
    }

    const kigl::GLVertexArray* TextMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        text::TextVAO* vao = text::TextSystem::get().getTextVAO();

        if (m_reserveVertexCount <= 0) {
            m_reserveVertexCount = 100 * 4;
        }

        if (m_reserveIndexCount <= 0) {
            m_reserveIndexCount = 100 * 2;
        }

        m_vboIndex = vao->reserveVertices(m_reserveVertexCount);
        vao->reserveAtlasCoords(m_reserveVertexCount);

        m_eboIndex = vao->reserveIndeces(m_reserveIndexCount);

        m_vao = vao->getVAO();

        return m_vao;
    }

    void TextMesh::prepareLod(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();
    }

    void TextMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = backend::DrawOptions::Mode::triangles;
    }
}
