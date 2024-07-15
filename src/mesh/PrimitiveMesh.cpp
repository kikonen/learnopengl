#include "PrimitiveMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/LodMesh.h"

#include "animation/RigContainer.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "mesh/vao/PositionEntry.h"

#include "engine/PrepareContext.h"
#include "registry/ModelRegistry.h"


namespace mesh {
    PrimitiveMesh::PrimitiveMesh()
        : Mesh("primitive"),
        m_type{ PrimitiveType::none }
    {
    }

    PrimitiveMesh::~PrimitiveMesh()
    {
    }

    std::string PrimitiveMesh::str() const noexcept
    {
        return fmt::format("<LINE: id={}>", m_id);
    }

    void PrimitiveMesh::clear() {
        m_vertices.clear();
        m_indeces.clear();
    }

    AABB PrimitiveMesh::calculateAABB(const glm::mat4& transform) const noexcept {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            const auto& pos = transform * glm::vec4(vertex.pos, 1.f);
            aabb.minmax(pos);
        }

        aabb.updateVolume();

        return aabb;
    }

    const kigl::GLVertexArray* PrimitiveMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        TexturedVAO* vao;
        SkinnedVAO* skinnedVao = nullptr;

        if (m_rig && !m_vertexBones.empty()) {
            skinnedVao = ModelRegistry::get().getSkinnedVao();
            vao = skinnedVao;
        }
        else {
            vao = ModelRegistry::get().getTexturedVao();
        }

        m_vboIndex = vao->reserveVertices(m_vertices.size());
        m_eboIndex = vao->reserveIndeces(m_indeces.size());
        if (skinnedVao) {
            skinnedVao->reserveVertexBones(m_vertexBones.size());
        }

        vao->updateVertices(
            m_vboIndex,
            m_vertices);

        vao->updateIndeces(
            m_eboIndex,
            m_indeces);

        if (skinnedVao) {
            skinnedVao->updateVertexBones(
                m_vboIndex,
                m_vertexBones);
        }

        m_vao = vao->getVAO();

        return m_vao;
    }

    void PrimitiveMesh::prepareLodMesh(
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
