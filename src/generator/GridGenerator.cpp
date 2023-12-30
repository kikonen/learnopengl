#include "GridGenerator.h"

#include <vector>

#include "asset/Mesh.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

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
    prepareInstances(
        assets,
        registry,
        container);
}

void GridGenerator::update(
    const UpdateContext& ctx,
    Node& container)
{
    const auto parentLevel = container.getParent()->getTransform().getMatrixLevel();
    if (m_containerMatrixLevel == parentLevel) return;
    updateInstances(
        ctx,
        container);
    container.modifyTransform().m_dirtyEntity = true;
    m_containerMatrixLevel = parentLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    Node& container)
{
    const auto& parentTransform = container.getParent()->getTransform();

    const auto& containerTransform = container.getTransform();
    int idx = 0;

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto& transform = m_transforms[idx];

                const glm::vec3 pos{ x * m_xStep, y * m_yStep, z * m_zStep };

                transform.setPosition(containerTransform.getPosition() + pos);

                transform.setMaterialIndex(containerTransform.getMaterialIndex());
                transform.setVolume(containerTransform.getVolume());

                transform.setQuatRotation(containerTransform.getQuatRotation());
                transform.setScale(containerTransform.getScale());

                transform.updateModelMatrix(parentTransform);

                idx++;
            }
        }
    }

    setActiveRange(m_reservedFirst, m_reservedCount);
    container.m_instancer = this;
}

void GridGenerator::prepareInstances(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    auto& entityRegistry = *registry->m_entityRegistry;

    m_reservedCount = m_zCount * m_xCount * m_yCount;
    m_reservedFirst = entityRegistry.registerEntityRange(m_reservedCount);

    m_transforms.reserve(m_reservedCount);

    for (int i = 0; i < m_reservedCount; i++) {
        auto& transform = m_transforms.emplace_back();
        transform.m_entityIndex = m_reservedFirst + i;
    }
}
