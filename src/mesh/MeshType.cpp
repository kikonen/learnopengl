#include "MeshType.h"

#include <fmt/format.h>

#include "material/CustomMaterial.h"
#include "material/Material.h"

#include "backend/DrawOptions.h"

#include "pool/TypeHandle.h"

#include "mesh/MeshSet.h"
#include "mesh/LodMesh.h"
#include "mesh/Mesh.h"

#include "engine/PrepareContext.h"

#include "model/Snapshot.h"
#include "model/EntityFlags.h"

#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/MeshTypeRegistry.h"

namespace {
}

namespace mesh {
    MeshType::MeshType()
        : m_lodMeshes{ std::make_unique<std::vector<LodMesh>>()}
    {}

    MeshType::MeshType(MeshType&& o) noexcept
        : m_handle{ o.m_handle },
        m_name{ o.m_name },
        m_nodeType{ o.m_nodeType },
        m_flags{ o.m_flags },
        m_layer{ o.m_layer },
        m_preparedWT{ o.m_preparedWT },
        m_preparedRT{ o.m_preparedRT },
        m_lodMeshes{ std::move(o.m_lodMeshes) },
        m_customMaterial{ std::move(o.m_customMaterial) }
    {
    }

    MeshType::~MeshType()
    {
        KI_INFO(fmt::format("NODE_TYPE: delete - id={}", m_handle.str()));
    }

    std::string MeshType::str() const noexcept
    {
        auto* lodMesh = getLodMesh(0);

        return fmt::format(
            "<NODE_TYPE: id={}, name={}, lod={}>",
            m_handle.str(), m_name, lodMesh ? lodMesh->str() : "N/A");
    }

    uint16_t MeshType::addMeshSet(
        const mesh::MeshSet& meshSet)
    {
        uint16_t count = 0;

        for (auto& mesh : meshSet.getMeshes()) {
            auto* lodMesh = addLodMesh({ mesh });
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
            lodMesh.registerMaterial();

            const auto& opt = lodMesh.m_drawOptions;
            m_flags.anySolid |= opt.isSolid();
            m_flags.anyAlpha |= opt.isAlpha();
            m_flags.anyBlend |= opt.isBlend();
            m_flags.anyAnimation |= lodMesh.m_flags.useAnimation;
        }
    }

    void MeshType::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_preparedRT) return;
        m_preparedRT = true;

        for (auto& lodMesh : *m_lodMeshes) {
            lodMesh.prepareRT(ctx);
        }

        if (m_customMaterial) {
            m_customMaterial->prepareRT(ctx);
            MeshTypeRegistry::get().registerCustomMaterial(m_handle);
        }
    }

    void MeshType::bind(const RenderContext& ctx)
    {
        assert(isReady());
    }

    void MeshType::setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept
    {
        m_customMaterial = std::move(customMaterial);
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
        if (m_lodMeshes->empty()) return {};

        AABB aabb{ true };

        for (auto& lodMesh : *m_lodMeshes) {
            if (lodMesh.m_flags.noVolume) continue;
            aabb.minmax(lodMesh.calculateAABB());
        }

        aabb.updateVolume();

        return aabb;
    }
}
