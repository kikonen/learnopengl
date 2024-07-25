#include "TextMesh.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "text/TextDraw.h"

#include "text/vao/TextVAO.h"
#include "text/TextSystem.h"


namespace {
    constexpr uint32_t DEF_SIZE = 100;
}

namespace mesh {
    TextMesh::TextMesh()
        : Mesh("text")
    {
    }

    TextMesh::~TextMesh()
    {
    }

    void TextMesh::clear() {
        m_vertices.clear();
        m_atlasCoords.clear();
        m_indeces.clear();
    }

    const kigl::GLVertexArray* TextMesh::prepareVAO()
    {
        if (m_preparedVAO) return m_vao;
        m_preparedVAO = true;

        text::TextVAO* vao = text::TextSystem::get().getTextVAO();

        return setupVAO(vao);
    }

    const kigl::GLVertexArray* TextMesh::setupVAO(mesh::TexturedVAO* vao)
    {
        if (m_maxSize == 0) {
            m_maxSize = DEF_SIZE;
        }

        m_vboIndex = vao->reserveVertices(m_maxSize * 4);

        text::TextVAO* textVao = dynamic_cast<text::TextVAO*>(vao);
        if (textVao) {
            textVao->reserveAtlasCoords(m_maxSize * 4);
        }

        m_eboIndex = vao->reserveIndeces(m_maxSize * 4);

        m_vao = vao->getVAO();
        return m_vao;
    }

    void TextMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = backend::DrawOptions::Mode::triangles;
    }
}
