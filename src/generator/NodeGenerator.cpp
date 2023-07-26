#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "render/Batch.h"

#include "registry/EntityRegistry.h"

void NodeGenerator::updateEntity(
    const UpdateContext& ctx,
    Node& container,
    EntityRegistry* entityRegistry)
{
    if (m_activeCount == 0) return;

    int entityIndex = m_reservedFirst;

    for (auto& instance : m_instances) {
        if (!instance.m_entityDirty) continue;
        if (instance.m_entityIndex == -1) continue;

        auto* entity = entityRegistry->updateEntity(instance.m_entityIndex, true);
        instance.updateEntity(ctx, entity);

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

    for (auto& instance : m_instances)
    {
        minmax.minmax(instance.getPosition());
    }

    return minmax.getVolume();
}
