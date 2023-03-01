#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "scene/Batch.h"

#include "registry/EntityRegistry.h"

void NodeGenerator::updateEntity(
    const RenderContext& ctx,
    Node& container,
    EntityRegistry* entityRegistry)
{
    if (m_activeCount == 0) return;

    int entityIndex = m_reservedFirst;

    for (auto& instance : m_instances) {
        if (!instance.m_entityDirty) continue;

        auto* entity = entityRegistry->updateEntity(entityIndex, true);

        instance.m_entityIndex = entityIndex;
        entity->setObjectID(container.m_objectID);

        instance.updateEntity(entity);

        //entity->u_highlightIndex = getHighlightIndex(ctx);

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

const glm::vec4& NodeGenerator::calculateVolume()
{
    AABB minmax{ true };

    for (auto& instance : m_instances)
    {
        minmax.minmax(instance.getPosition());
    }

    return minmax.getVolume();
}
