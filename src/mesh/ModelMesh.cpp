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

    uint32_t ModelMesh::getBaseVertex() const noexcept {
        return static_cast<uint32_t>(m_positionVboOffset / sizeof(mesh::PositionEntry));
    }

    AABB ModelMesh::calculateAABB() const noexcept {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            aabb.minmax(vertex.pos);
        }

        return aabb;
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
        drawOptions.m_mode = drawOptions.m_tessellation
            ? backend::DrawOptions::Mode::patches
            : backend::DrawOptions::Mode::triangles;
    }
}
