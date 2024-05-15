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
            "<MODEL: id={}, name={}>",
            m_id, m_name);
    }

    uint32_t ModelMesh::getBaseVertex() const noexcept {
        return static_cast<uint32_t>(m_positionVboOffset / sizeof(mesh::PositionEntry));
    }

    const AABB ModelMesh::calculateAABB() const {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

        for (auto&& vertex : m_vertices)
        {
            minAABB.x = std::min(minAABB.x, vertex.pos.x);
            minAABB.y = std::min(minAABB.y, vertex.pos.y);
            minAABB.z = std::min(minAABB.z, vertex.pos.z);

            maxAABB.x = std::max(maxAABB.x, vertex.pos.x);
            maxAABB.y = std::max(maxAABB.y, vertex.pos.y);
            maxAABB.z = std::max(maxAABB.z, vertex.pos.z);
        }

        return { minAABB, maxAABB, false };
    }

    const kigl::GLVertexArray* ModelMesh::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        if (m_rig && m_rig->hasBones()) {
            m_vao = ModelRegistry::get().getSkinnedVao()->registerModel(this);
        }
        else {
            m_vao = ModelRegistry::get().getTexturedVao()->registerModel(this);
        }

        return m_vao;
    }

    void ModelMesh::prepareLod(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();
    }

    void ModelMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = drawOptions.m_tessellation ? GL_PATCHES : GL_TRIANGLES;
    }
}
