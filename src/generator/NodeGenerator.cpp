#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/Batch.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"

void NodeGenerator::snapshotWT(bool force)
{
    const auto diff = m_transforms.size() - m_snapshotsWT.size();
    if (diff != 0) {
        force |= true;
        m_snapshotsWT.reserve(m_transforms.size());
        for (int i = 0; i < diff; i++) {
            m_snapshotsWT.emplace_back();
        }
    }

    for (size_t i = 0; i < m_transforms.size(); i++) {
        auto& transform = m_transforms[i];

        assert(!transform.m_dirty);
        if (!force && !transform.m_dirtySnapshot) continue;

        auto& snapshot = m_snapshotsWT[i];
        snapshot = transform;
        snapshot.m_dirty = true;

        transform.m_dirtySnapshot = false;
    }
}

void NodeGenerator::snapshotRT(bool force)
{
    const auto diff = m_snapshotsWT.size() - m_snapshotsRT.size();
    if (diff != 0) {
        force |= true;
        m_snapshotsRT.reserve(m_snapshotsWT.size());
        for (int i = 0; i < diff; i++) {
            m_snapshotsRT.emplace_back();
        }
    }

    for (size_t i = 0; i < m_snapshotsWT.size(); i++) {
        auto& snapshotWT = m_snapshotsWT[i];

        if (!force && !snapshotWT.m_dirty) continue;

        auto& snapshotRT = m_snapshotsRT[i];
        snapshotRT = snapshotWT;
        snapshotRT.m_dirty = true;

        snapshotWT.m_dirty = false;
    }
}

void NodeGenerator::prepareEntities(
    const PrepareContext& ctx)
{
    if (m_reservedFirst >= 0) return;
    if (m_reservedCount == 0) return;

    m_reservedFirst = ctx.m_registry->m_entityRegistry->registerEntityRange(m_reservedCount);

    uint32_t snapshotIndex = 0;
    for (auto& snapshot : m_snapshotsRT) {
        snapshot.m_entityIndex = static_cast<int>(m_reservedFirst + snapshotIndex);
        prepareEntity(ctx, snapshot, snapshotIndex);
        snapshotIndex++;
    }
}

void NodeGenerator::updateEntity(
    const UpdateContext& ctx,
    Node& container,
    EntityRegistry* entityRegistry,
    bool force)
{
    if (m_activeCount == 0) return;

    if (m_reservedFirst == -1) {
        prepareEntities(ctx.toPrepareContext());
    }

    const auto flags = container.getSnapshot().m_flags;
    const auto highlightIndex = container.getHighlightIndex(ctx.m_assets);

    for (auto& snapshot : m_snapshotsRT) {
        if (!force && !snapshot.m_dirty) continue;
        if (snapshot.m_entityIndex == -1) continue;

        auto* entity = entityRegistry->modifyEntity(snapshot.m_entityIndex, true);

        entity->u_objectID = container.m_id;
        entity->u_flags = flags;
        entity->u_highlightIndex = highlightIndex;

        snapshot.updateEntity(ctx, entity);

        snapshot.m_dirty = false;
    }
}

void NodeGenerator::bindBatch(
    const RenderContext& ctx,
    Node& container,
    render::Batch& batch)
{
    if (m_activeCount == 0) return;

    // NOTE KI instanced node may not be ready, or currently not generating visible entities
    //batch.addInstanced(ctx, container.getEntityIndex(), m_activeFirst, m_activeCount);
    batch.addSnapshotsInstanced(ctx, m_snapshotsRT);
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
