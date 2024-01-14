#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/Batch.h"
#include "render/RenderContext.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"
#include "registry/SnapshotRegistry.h"


void NodeGenerator::prepareSnapshots(
    SnapshotRegistry& snapshotRegistry)
{
    const auto count = m_transforms.size();
    m_snapshotBase = snapshotRegistry.registerSnapshotRange(count);
}

void NodeGenerator::snapshotWT(
    SnapshotRegistry& snapshotRegistry)
{
    const auto count = m_transforms.size();

    auto snapshots = snapshotRegistry.modifySnapshotRange(m_snapshotBase, count);

    for (size_t i = 0; i < count; i++) {
        auto& transform = m_transforms[i];

        assert(!transform.m_dirty);
        if (!transform.m_dirtySnapshot) continue;

        auto& snapshot = snapshots[i];
        snapshot = transform;
        snapshot.m_dirty = true;

        transform.m_dirtySnapshot = false;
    }
}

void NodeGenerator::prepareEntities(
    SnapshotRegistry& snapshotRegistry,
    EntityRegistry& entityRegistry)
{
    if (m_entityBase) return;
    if (m_reservedCount == 0) return;

    auto snapshots = snapshotRegistry.getActiveSnapshotRange(
        m_snapshotBase,
        m_reservedCount);

    m_entityBase = entityRegistry.registerEntityRange(m_reservedCount);

    auto entities = entityRegistry.getEntityRange(
        m_entityBase,
        m_reservedCount);

    for (uint32_t i = 0; i < m_reservedCount; i++) {
        prepareEntity(snapshots[i], entities[i], i);
    }
}

void NodeGenerator::updateEntity(
    const Assets& assets,
    SnapshotRegistry& snapshotRegistry,
    EntityRegistry& entityRegistry,
    Node& container)
{
    if (m_activeCount == 0) return;

    if (!m_entityBase) {
        prepareEntities(snapshotRegistry, entityRegistry);
    }

    auto snapshots = snapshotRegistry.modifyActiveSnapshotRange(
        m_snapshotBase,
        m_reservedCount);
    const auto& containerSnapshot = snapshotRegistry.getActiveSnapshot(container.m_snapshotIndex);

    auto entities = entityRegistry.modifyEntityRange(
        m_entityBase,
        m_reservedCount);

    const auto flags = containerSnapshot.m_flags;
    const auto highlightIndex = container.getHighlightIndex(assets);

    for (uint32_t i = 0; i < m_reservedCount; i++) {
        auto& snapshot = snapshots[i];
        if (!snapshot.m_dirty) continue;

        auto& entity = entities[i];

        entity.u_objectID = container.m_id;
        entity.u_flags = flags;
        entity.u_highlightIndex = highlightIndex;

        snapshot.updateEntity(entity);

        entityRegistry.markDirty(m_entityBase + i);
        snapshot.m_dirty = false;
    }
}

void NodeGenerator::bindBatch(
    const RenderContext& ctx,
    Node& container,
    render::Batch& batch)
{
    if (m_activeCount == 0) return;

    const auto snapshots = ctx.m_registry->m_snapshotRegistry->getActiveSnapshotRange(
        m_snapshotBase,
        m_reservedCount);

    // NOTE KI instanced node may not be ready, or currently not generating visible entities
    //batch.addInstanced(ctx, container.getEntityIndex(), m_activeFirst, m_activeCount);
    batch.addSnapshotsInstanced(ctx, snapshots, m_entityBase);
}

const kigl::GLVertexArray* NodeGenerator::getVAO(
    const Node& container) const noexcept
{
    return container.m_type->getVAO();
}

const backend::DrawOptions& NodeGenerator::getDrawOptions(
    const Node& container) const noexcept
{
    return container.m_type->getDrawOptions();
}

const glm::vec4 NodeGenerator::calculateVolume()
{
    AABB minmax{ true };

    for (auto& transform : m_transforms)
    {
        minmax.minmax(transform.getPosition());
    }

    return minmax.getVolume();
}
