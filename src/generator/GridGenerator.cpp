#include "GridGenerator.h"

#include <vector>

#include "asset/Mesh.h"

#include "model/Node.h"

#include "scene/RenderContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"


GridGenerator::GridGenerator()
{
}

void GridGenerator::prepare(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    createInstances(
        assets,
        registry,
        container);
}

void GridGenerator::update(
    const RenderContext& ctx,
    Node& container,
    Node* containerParent)
{
    if (m_matrixLevel == container.getMatrixLevel());
    updateInstances(
        ctx,
        container);
    m_matrixLevel = container.getMatrixLevel();
}

void GridGenerator::updateInstances(
    const RenderContext& ctx,
    Node& container)
{
    auto& entityRegistry = *ctx.m_registry->m_entityRegistry;

    const auto& nodePos = container.getPosition();
    auto* nodeEntity = entityRegistry.getEntity(container.m_entityIndex);

    int entityIndex = container.getInstancedIndex();

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto* entity = entityRegistry.updateEntity(entityIndex, true);
                *entity = *nodeEntity;

                const glm::vec3 posAdjustment{ x * m_xStep, y * m_yStep, z * m_zStep };
                entity->adjustPosition(posAdjustment);

                entityIndex++;
            }
        }
    }
}

void GridGenerator::createInstances(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    auto& entityRegistry = *registry->m_entityRegistry;

    const int entityCount = m_zCount * m_xCount * m_yCount;
    const int firstIndex = entityRegistry.addEntityRange(entityCount);

    node.setEntityRange(firstIndex, entityCount);
}
