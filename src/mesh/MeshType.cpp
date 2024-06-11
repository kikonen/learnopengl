#include "MeshType.h"

#include <fmt/format.h>

#include "asset/CustomMaterial.h"
#include "asset/Material.h"

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
            "<NODE_TYPE: id={}, name={}, lod={}>",
            m_id, m_name, lodMesh ? lodMesh->str() : "N/A");
    }

    pool::TypeHandle MeshType::toHandle() const noexcept
    {
        return { m_handleIndex, m_id };
    }

    uint16_t MeshType::addMeshSet(
        mesh::MeshSet& meshSet)
    {
        uint16_t count = 0;

        for (auto& mesh : meshSet.getMeshes()) {
            auto* lodMesh = addLodMesh({ mesh.get() });

            if (m_flags.zUp) {
                const auto rotateYUp = util::degreesToQuat(glm::vec3{ 90.f, 0.f, 0.f });
                lodMesh->getMesh<mesh::Mesh>()->m_animationBaseTransform = glm::toMat4(rotateYUp);
            }

            count++;
        }

        // NOTE KI ensure volume is containing all meshes
        prepareVolume();

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
            lodMesh.registerMaterial();

            const auto& opt = lodMesh.m_drawOptions;
            m_flags.anySolid |= opt.m_solid;
            m_flags.anyAlpha |= opt.m_alpha;
            m_flags.anyBlend |= opt.m_blend;
        }
    }

    void MeshType::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_preparedRT) return;
        m_preparedRT = true;

        for (auto& lodMesh : *m_lodMeshes) {
            lodMesh.m_drawOptions.m_tessellation = m_flags.tessellation;
            lodMesh.prepareRT(ctx);
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

    int8_t MeshType::getLodLevel(
        const glm::vec3& cameraPos,
        const glm::vec3& worldPos) const
    {
        auto& lodMeshes = *m_lodMeshes.get();
        if (lodMeshes.size() == 1) return lodMeshes[0].m_level;

        const LodMesh* last = &lodMeshes[0];
        {
            auto dist2 = glm::distance2(worldPos, cameraPos);

            for (const auto& lodMesh : lodMeshes) {
                if (lodMesh.m_level < 0) continue;
                if (dist2 < lodMesh.m_distance2)
                    return lodMesh.m_level;
                last = &lodMesh;
            }
        }

        return last->m_level;
    }

    ki::size_t_entity_flags MeshType::resolveEntityFlags() const noexcept {
        ki::size_t_entity_flags flags = 0;

        if (m_flags.noFrustum) {
            flags |= ENTITY_NO_FRUSTUM_BIT;
        }
        return flags;
    }

    void MeshType::prepareVolume() noexcept {
        m_aabb = calculateAABB();
    }

    AABB MeshType::calculateAABB() const noexcept
    {
        AABB aabb{ true };

        for (auto& lodMesh : *m_lodMeshes) {
            auto* mesh = lodMesh.getMesh<mesh::Mesh>();
            if (!mesh) continue;
            mesh->prepareVolume();
            aabb.merge(mesh->getAABB());
        }

        return aabb;
    }
}
