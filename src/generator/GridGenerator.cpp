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
    const int parentLevel = container.getParent()->getMatrixLevel();
    if (m_containerMatrixLevel == parentLevel) return;
    updateInstances(
        ctx,
        container);
    m_containerMatrixLevel = parentLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    Node& container)
{
    const auto& parentInstance = container.getParent()->getInstance();

    const auto& containerInstance = container.getInstance();
    int idx = 0;

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto& instance = m_instances[idx];

                const glm::vec3 pos{ x * m_xStep, y * m_yStep, z * m_zStep };

                instance.setPosition(containerInstance.getPosition() + pos);

                instance.setId(containerInstance.getId());
                instance.setFlags(containerInstance.getFlags());
                instance.setMaterialIndex(containerInstance.getMaterialIndex());
                instance.setVolume(containerInstance.getVolume());

                instance.setRotation(containerInstance.getRotation());
                instance.setScale(containerInstance.getScale());

                instance.updateModelMatrix(parentInstance);

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
    m_reservedFirst = entityRegistry.addEntityRange(m_reservedCount);

    m_instances.reserve(m_reservedCount);

    for (int i = 0; i < m_reservedCount; i++) {
        auto& instance = m_instances.emplace_back();
        instance.m_entityIndex = m_reservedFirst + i;
    }
}
