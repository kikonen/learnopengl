#include "NodeGenerator.h"

#include <iostream>

#include "asset/AABB.h"

#include "backend/Lod.h"

#include "model/Node.h"

#include "component/Camera.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/Batch.h"
#include "render/RenderContext.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"
#include "registry/NodeSnapshotRegistry.h"


NodeGenerator::~NodeGenerator() = default;

std::span<const Snapshot> NodeGenerator::getSnapshots(
    NodeSnapshotRegistry& snapshotRegistry) const noexcept
{
    return snapshotRegistry.getSnapshotRange(
        m_snapshotBase,
        static_cast<uint32_t>(m_transforms.size()));
}

void NodeGenerator::prepareSnapshots(
    NodeSnapshotRegistry& snapshotRegistry)
{
    const auto count = m_transforms.size();
    m_snapshotBase = snapshotRegistry.registerSnapshotRange(count);
}

void NodeGenerator::snapshotWT(
    NodeSnapshotRegistry& snapshotRegistry)
{
    const auto count = static_cast<uint32_t>(m_transforms.size());

    auto snapshots = snapshotRegistry.modifySnapshotRange(m_snapshotBase, count);

    for (size_t i = 0; i < count; i++) {
        const auto& transform = m_transforms[i];

        assert(!transform.m_dirty);
        if (!transform.m_dirtySnapshot) continue;

        snapshots[i].applyFrom(transform);
    }
}

void NodeGenerator::prepareEntities(
    EntityRegistry& entityRegistry)
{
    if (m_entityBase) return;
    if (m_reservedCount == 0) return;

    m_entityBase = entityRegistry.registerEntityRange(m_reservedCount);

    auto entities = entityRegistry.modifyEntityRange(
        m_entityBase,
        m_reservedCount);

    for (uint32_t i = 0; i < m_reservedCount; i++) {
        prepareEntity(entities[i], i);
    }
}

void NodeGenerator::updateEntity(
    NodeSnapshotRegistry& snapshotRegistry,
    EntityRegistry& entityRegistry,
    Node& container)
{
    if (m_activeCount == 0) return;

    if (!m_entityBase) {
        prepareEntities(entityRegistry);
    }

    bool ready = snapshotRegistry.hasSnapshotRange(
        m_snapshotBase,
        m_reservedCount);

    if (!ready) {
        KI_INFO(fmt::format("GENERATOR: snapshot not_ready - container={}", container.str()));
        return;
    }

    auto snapshots = snapshotRegistry.getSnapshotRange(
        m_snapshotBase,
        m_reservedCount);
    const auto& containerSnapshot = snapshotRegistry.getSnapshot(container.m_snapshotIndex);

    auto entities = entityRegistry.modifyEntityRange(
        m_entityBase,
        m_reservedCount);

    const auto flags = containerSnapshot.m_flags;
    const auto highlightIndex = container.getHighlightIndex();

    for (uint32_t i = 0; i < m_reservedCount; i++) {
        auto& snapshot = snapshots[i];
        if (!snapshot.m_dirty) continue;

        auto& entity = entities[i];

        entity.u_objectID = container.getId();
        entity.u_flags = flags;
        entity.u_highlightIndex = highlightIndex;

        snapshot.updateEntity(entity);

        entityRegistry.markDirty(m_entityBase + i);
        snapshot.m_dirty = false;
    }
}

void NodeGenerator::bindBatch(
    const RenderContext& ctx,
    mesh::MeshType* type,
    const std::function<Program* (const mesh::LodMesh&)>& programSelector,
    uint8_t kindBits,
    render::Batch& batch,
    Node& container)
{
    if (m_activeCount == 0) return;

    auto& snapshotRegistry = *ctx.m_registry->m_activeSnapshotRegistry;

    bool ready = snapshotRegistry.hasSnapshotRange(
        m_snapshotBase,
        m_reservedCount);

    if (!ready) return;

    const auto& snapshots = snapshotRegistry.getSnapshotRange(
        m_snapshotBase,
        m_reservedCount);

    //{
    //    m_lods.reserve(m_reservedCount);
    //    m_lods.clear();

    //    if (m_lods.size() != m_transforms.size()) {
    //        m_lods.clear();

    //        const auto& cameraPos = ctx.m_camera->getWorldPosition();

    //        auto& meshLods = type->getLods();

    //        //auto* meshLod = type->getLod(0);
    //        //const auto& lod = meshLod->m_lod;

    //        for (auto& snapshot : snapshots) {
    //            auto dist2 = glm::distance2(snapshot.getWorldPosition(), cameraPos);
    //            //std::cout << "DIST: " << distance << "\n";

    //            int lodIndex = 0;
    //            for (; lodIndex < meshLods.size(); lodIndex++) {
    //                if (dist2 < meshLods[lodIndex].m_lod.m_distance2)
    //                    break;
    //            }
    //            if (lodIndex >= meshLods.size()) {
    //                lodIndex--;
    //            }

    //            // TODO KI select LOD based to distance
    //            //int lodIndex = static_cast<int>(rand() * (meshLods.size() + 1)) % meshLods.size();

    //            m_lods.push_back(&meshLods[lodIndex].m_lod);
    //        }
    //    }
    //}
    //auto lodSpan = std::span<const backend::Lod*>{ m_lods };
    //auto lodSpan = std::span<const backend::Lod*>{};

    // NOTE KI instanced node may not be ready, or currently not generating visible entities
    batch.addSnapshotsInstanced(
        ctx,
        type,
        programSelector,
        kindBits,
        snapshots,
        m_entityBase);
}

glm::vec4 NodeGenerator::calculateVolume() const noexcept
{
    AABB minmax{ true };

    for (auto& transform : m_transforms)
    {
        minmax.minmax(transform.getPosition());
    }

    return minmax.getVolume();
}
