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
    prepareInstances(
        assets,
        registry,
        container);
}

void GridGenerator::update(
    const RenderContext& ctx,
    Node& container,
    Node* containerParent)
{
    if (m_containerMatrixLevel == container.getMatrixLevel()) return;
    updateInstances(
        ctx,
        container,
        containerParent);
    m_containerMatrixLevel = container.getMatrixLevel();
}

void GridGenerator::updateInstances(
    const RenderContext& ctx,
    Node& container,
    Node* containerParent)
{
    const auto& containerMatrix = containerParent->getModelMatrix();
    const auto containerLevel = containerParent->getMatrixLevel();

    const auto& containerInstance = container.getInstance();
    int idx = 0;

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto& instance = m_instances[idx];

                const glm::vec3 pos{ x * m_xStep, y * m_yStep, z * m_zStep };

                instance.setPosition(containerInstance.getPosition() + pos);

                instance.setObjectID(containerInstance.getObjectID());
                instance.setFlags(containerInstance.getFlags());
                instance.setMaterialIndex(containerInstance.getMaterialIndex());
                instance.setVolume(containerInstance.getVolume());

                instance.setRotation(containerInstance.getRotation());
                instance.setScale(containerInstance.getScale());

                instance.updateModelMatrix(containerMatrix, containerLevel);

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
