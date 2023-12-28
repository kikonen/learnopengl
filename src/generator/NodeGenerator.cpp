#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "render/Batch.h"

#include "engine/UpdateContext.h"

#include "registry/EntityRegistry.h"

void NodeGenerator::updateEntity(
    const UpdateContext& ctx,
    Node& container,
    EntityRegistry* entityRegistry)
{
    if (m_activeCount == 0) return;

    int entityIndex = m_reservedFirst;

    for (auto& transform : m_transforms) {
        if (!transform.m_dirtyEntity) continue;
        if (transform.m_entityIndex == -1) continue;

        auto* entity = entityRegistry->modifyEntity(transform.m_entityIndex, true);

        entity->u_objectID = container.m_id;
        entity->u_flags = container.getEntityFlags();
        entity->u_highlightIndex = container.getHighlightIndex(ctx.m_assets);

        transform.updateEntity(ctx, entity);

        //entity->u_highlightIndex = getHighlightIndex(ctx.m_assets);
        //entity->u_highlightIndex = 1;

        entityIndex++;
    }
}

void NodeGenerator::bindBatch(
    const RenderContext& ctx,
    Node& container,
    Batch& batch)
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
