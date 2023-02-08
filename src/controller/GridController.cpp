#include "GridController.h"

#include <vector>

#include "asset/Mesh.h"

#include "model/Node.h"

#include "scene/RenderContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"


GridController::GridController()
{
}

void GridController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    createInstances(
        assets,
        registry,
        node);
}

bool GridController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent)
{
    if (m_matrixLevel == node.getMatrixLevel()) return false;
    updateInstances(
        ctx,
        node);
    m_matrixLevel = node.getMatrixLevel();
    return true;
}

void GridController::updateInstances(
    const RenderContext& ctx,
    Node& node)
{
    auto& entityRegistry = *ctx.m_registry->m_entityRegistry;

    const auto& nodePos = node.getPosition();
    auto* nodeEntity = entityRegistry.get(node.m_entityIndex);

    int entityIndex = node.getInstancedIndex();

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto* entity = entityRegistry.update(entityIndex, true);
                *entity = *nodeEntity;

                const glm::vec3 posAdjustment{ x * m_xStep, y * m_yStep, z * m_zStep };
                entity->adjustPosition(posAdjustment);

                entityIndex++;
            }
        }
    }
}

void GridController::createInstances(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    auto& entityRegistry = *registry->m_entityRegistry;

    const int entityCount = m_zCount * m_xCount * m_yCount;
    const int firstIndex = entityRegistry.addRange(entityCount);

    node.setEntityRange(firstIndex, entityCount);
}
