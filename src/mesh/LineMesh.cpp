#include "LineMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/LodMesh.h"

#include "mesh/vao/TexturedVAO.h"

#include "mesh/vao/PositionEntry.h"

#include "engine/PrepareContext.h"
#include "registry/ModelRegistry.h"


namespace {
    constexpr uint32_t DEF_SIZE = 100;
}

namespace mesh {
    LineMesh::LineMesh()
        : Mesh("line")
    {
    }

    LineMesh::~LineMesh()
    {
    }

    std::string LineMesh::str() const noexcept
    {
        return fmt::format("<LINE: id={}>", m_id);
    }

    void LineMesh::clear() {
        m_vertices.clear();
        m_indeces.clear();
    }

    AABB LineMesh::calculateAABB(const glm::mat4& transform) const noexcept {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            const auto& pos = transform * glm::vec4(vertex.pos, 1.f);
            aabb.minmax(pos);
        }

        aabb.updateVolume();

        return aabb;
    }

    const kigl::GLVertexArray* LineMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        TexturedVAO* vao = ModelRegistry::get().getTexturedVao();

        m_vboIndex = vao->reserveVertices(m_vertices.size());
        m_eboIndex = vao->reserveIndeces(m_indeces.size());

        vao->updateVertices(
            m_vboIndex,
            m_vertices);

        vao->updateIndeces(
            m_eboIndex,
            m_indeces);

        m_vao = vao->getVAO();

        return m_vao;
    }

    void LineMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::elements;

        switch (m_type) {
        case PrimitiveType::lines:
            drawOptions.m_mode = backend::DrawOptions::Mode::lines;
            break;
        case PrimitiveType::points:
            drawOptions.m_mode = backend::DrawOptions::Mode::points;
            break;
        }
    }
}
