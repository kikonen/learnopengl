#include "MeshType.h"

#include <fmt/format.h>

#include "asset/Program.h"
#include "asset/CustomMaterial.h"
#include "asset/Material.h"
#include "asset/Sprite.h"

#include "backend/DrawOptions.h"

#include "pool/TypeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/Mesh.h"

#include "engine/PrepareContext.h"

#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/SpriteRegistry.h"


namespace {
}

namespace mesh {
    MeshType::MeshType()
        : m_lodMeshes{ std::make_unique<std::vector<LodMesh>>()}
    {}

    MeshType::MeshType(MeshType&& o) noexcept
        : m_id{ o.m_id },
        m_name{ o.m_name },
        m_entityType{ o.m_entityType },
        m_flags{ o.m_flags },
        m_priority{ o.m_priority },
        m_program{ o.m_program },
        m_shadowProgram{ o.m_shadowProgram },
        m_preDepthProgram{ o.m_preDepthProgram },
        m_sprite{ std::move(o.m_sprite) },
        m_drawOptions{ o.m_drawOptions },
        m_vao{ o.m_vao },
        m_preparedWT{ o.m_preparedWT },
        m_preparedRT{ o.m_preparedRT },
        m_lodMeshes{ std::move(o.m_lodMeshes) },
        m_customMaterial{ std::move(o.m_customMaterial) }
    {
    }

    MeshType::~MeshType()
    {
        KI_INFO(fmt::format("NODE_TYPE: delete iD={}", m_id));
    }

    std::string MeshType::str() const noexcept
    {
        auto* lod = getLod(0);

        return fmt::format(
            "<NODE_TYPE: id={}, name={}, vao={}, lod={}>",
            m_id, m_name, m_vao ? *m_vao : -1, lod ? lod->str() : "N/A");
    }

    pool::TypeHandle MeshType::toHandle() const noexcept
    {
        return { m_handleIndex, m_id };
    }

    LodMesh* MeshType::addLod(
        LodMesh&& lod)
    {
        m_lodMeshes->push_back(std::move(lod));
        return &(*m_lodMeshes)[m_lodMeshes->size() - 1];
    }

    void MeshType::prepareWT(
        const PrepareContext& ctx)
    {
        if (m_preparedWT) return;
        m_preparedWT = true;

        if (!hasMesh()) return;

        for (auto& lodMesh : *m_lodMeshes) {
            lodMesh.registerMaterials();
        }

        if (m_entityType == EntityType::sprite && m_sprite) {
            SpriteRegistry::get().registerSprite(*m_sprite);
        }
    }

    void MeshType::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_preparedRT) return;
        m_preparedRT = true;

        if (!hasMesh()) return;

        for (auto& lodMesh : *m_lodMeshes) {
            lodMesh.prepareRT(ctx);
        }

        {
            auto& lodMesh = (*m_lodMeshes)[0];
            m_vao = lodMesh.m_vao;
            m_drawOptions.m_renderBack = m_flags.renderBack;
            m_drawOptions.m_wireframe = m_flags.wireframe;
            m_drawOptions.m_blend = m_flags.blend;
            m_drawOptions.m_blendOIT = m_flags.blendOIT;
            m_drawOptions.m_tessellation = m_flags.tessellation;

            lodMesh.m_mesh->prepareDrawOptions(m_drawOptions);
        }

        if (m_program) {
            m_program->prepareRT();
        }

        if (m_shadowProgram) {
            m_shadowProgram->prepareRT();
        }

        if (m_preDepthProgram) {
            m_preDepthProgram->prepareRT();
        }

        if (m_customMaterial) {
            m_customMaterial->prepareRT(ctx);
        }
    }

    void MeshType::bind(const RenderContext& ctx)
    {
        assert(isReady());

        if (m_customMaterial) {
            m_customMaterial->bindTextures(ctx);
        }
    }

    void MeshType::setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept
    {
        m_customMaterial = std::move(customMaterial);
    }
}
