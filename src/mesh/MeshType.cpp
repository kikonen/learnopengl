#include "MeshType.h"

#include <fmt/format.h>

#include "asset/Program.h"
#include "asset/CustomMaterial.h"
#include "asset/Material.h"
#include "asset/Sprite.h"

#include "backend/DrawOptions.h"

#include "pool/TypeHandle.h"

#include "mesh/MeshSet.h"
#include "mesh/LodMesh.h"
#include "mesh/Mesh.h"

#include "engine/PrepareContext.h"

#include "model/Snapshot.h"
#include "model/EntityFlags.h"

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
        m_selectionProgram{ o.m_selectionProgram },
        m_idProgram{ o.m_idProgram },
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
        auto* lodMesh = getLodMesh(0);

        return fmt::format(
            "<NODE_TYPE: id={}, name={}, vao={}, lod={}>",
            m_id, m_name, m_vao ? *m_vao : -1, lodMesh ? lodMesh->str() : "N/A");
    }

    pool::TypeHandle MeshType::toHandle() const noexcept
    {
        return { m_handleIndex, m_id };
    }

    uint16_t MeshType::addMeshSet(
        mesh::MeshSet& meshSet,
        uint16_t lodLevel)
    {
        uint16_t count = 0;
        for (auto& mesh : meshSet.getMeshes()) {
            auto* lodMesh = addLodMesh({ mesh.get() });
            if (lodMesh->m_lodLevel < 0) {
                lodMesh->m_lodLevel = lodLevel;
            }
            count++;
        }
        return count;
    }

    LodMesh* MeshType::addLodMesh(
        LodMesh&& lodmesh)
    {
        m_lodMeshes->push_back(std::move(lodmesh));
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

        //if (!hasMesh()) return;

        for (auto& lodMesh : *m_lodMeshes) {
            lodMesh.prepareRT(ctx);
        }

        if (!m_lodMeshes->empty()) {
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

        if (m_selectionProgram) {
            m_selectionProgram->prepareRT();
        }

        if (m_idProgram) {
            m_idProgram->prepareRT();
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

    std::span<mesh::LodMesh> MeshType::findMeshBatch(
        const glm::vec3& cameraPos,
        const Snapshot& snapshot) const
    {
        auto& lodMeshes = *m_lodMeshes.get();
        const auto sz = lodMeshes.size();

        if (sz == 1) return { lodMeshes };

        size_t lodIndex = 0;
        size_t count = sz;
        {
            auto dist2 = glm::distance2(snapshot.getWorldPosition(), cameraPos);

            lodIndex = 0;
            for (; lodIndex < sz - 1; lodIndex++) {
                if (dist2 < lodMeshes[lodIndex].m_lod.m_distance2)
                    break;
            }

            const auto level = lodMeshes[lodIndex].m_lodLevel;

            while (lodIndex > 0) {
                if (lodMeshes[lodIndex - 1].m_lodLevel != level)
                    break;
                lodIndex--;
            }

            //auto lod = &lodMeshes[lodIndex].m_lod;
            //lod = &lodMeshes[std::min((size_t)2, lodMeshes.size() -1)].m_lod;

            count = 0;
            while (lodIndex + count < sz) {
                if (lodMeshes[lodIndex + count].m_lodLevel != level)
                    break;
                count++;
            }
        }

        return std::span{ lodMeshes }.subspan(lodIndex, count);
    }

    ki::size_t_entity_flags MeshType::resolveEntityFlags() const noexcept {
        ki::size_t_entity_flags flags = 0;

        if (m_flags.billboard) {
            flags |= ENTITY_BILLBOARD_BIT;
        }
        if (m_entityType == mesh::EntityType::sprite) {
            flags |= ENTITY_SPRITE_BIT;
        }
        if (m_flags.noFrustum) {
            flags |= ENTITY_NO_FRUSTUM_BIT;
        }
        return flags;
    }
}
