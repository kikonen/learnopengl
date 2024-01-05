#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "render/Batch.h"

#include "engine/UpdateContext.h"

#include "registry/EntityRegistry.h"

void NodeGenerator::snapshot(bool force)
{
    const auto diff = m_transforms.size() - m_snapshots.size();
    if (diff != 0) {
        force |= true;
        m_snapshots.reserve(m_transforms.size());
        for (int i = 0; i < diff; i++) {
            m_snapshots.emplace_back();
        }
    }

    for (size_t i = 0; i < m_transforms.size(); i++) {
        auto& transform = m_transforms[i];
        assert(!transform.m_dirty);
        if (!force && !transform.m_dirtySnapshot) continue;
        m_snapshots[i] = transform;
        transform.m_dirtySnapshot = false;
        transform.m_dirtyNormal = false;
    }
}

void NodeGenerator::updateEntity(
    const UpdateContext& ctx,
    Node& container,
    EntityRegistry* entityRegistry,
    bool force)
{
    if (m_activeCount == 0) return;

    int entityIndex = m_reservedFirst;

    for (auto& snapshot : m_snapshots) {
        if (!force && !snapshot.m_dirtyEntity) continue;
        if (snapshot.m_entityIndex == -1) continue;

        auto* entity = entityRegistry->modifyEntity(snapshot.m_entityIndex, true);

        entity->u_objectID = container.m_id;
        entity->u_flags = container.getEntityFlags();
        entity->u_highlightIndex = container.getHighlightIndex(ctx.m_assets);

        snapshot.updateEntity(ctx, entity);

        entityIndex++;
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
    batch.addInstanced(ctx, -1, m_activeFirst, m_activeCount);
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
