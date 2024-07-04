#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/LodMesh.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "mesh/vao/PositionEntry.h"

#include "engine/PrepareContext.h"
#include "registry/ModelRegistry.h"

#include "animation/RigContainer.h"

namespace mesh {
    ModelMesh::ModelMesh(
        std::string_view name)
        : Mesh{ name }
    {
    }

    ModelMesh::~ModelMesh()
    {
        KI_INFO(fmt::format("MODEL_MESH: delete - {}", str()));
        m_vertices.clear();
    }

    std::string ModelMesh::str() const noexcept
    {
        return fmt::format(
            "<MODEL: id={}, name={}, indeces={}, vertices={}, bones={}>",
            m_id,
            m_name,
            m_indeces.size(),
            m_vertices.size(),
            m_vertexBones.size());
    }

    AABB ModelMesh::calculateAABB() const noexcept {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            const auto& pos = m_rigTransform * glm::vec4(vertex.pos, 1.f);
            aabb.minmax(pos);
        }

        return aabb;
    }

    const kigl::GLVertexArray* ModelMesh::prepareRT(
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

    void ModelMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = lodMesh.m_flags.tessellation
            ? backend::DrawOptions::Mode::patches
            : backend::DrawOptions::Mode::triangles;
    }
}
