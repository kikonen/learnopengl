#include "GridGenerator.h"

#include <vector>

#include "mesh/MeshType.h"
#include "mesh/Mesh.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"


GridGenerator::GridGenerator()
{
}

void GridGenerator::prepare(
    const PrepareContext& ctx,
    Node& container)
{
    prepareInstances(
        ctx,
        container);

    prepareSnapshots(*ctx.m_registry->m_workerSnapshotRegistry);
}

void GridGenerator::updateWT(
    const UpdateContext& ctx,
    Node& container)
{
    const auto parentLevel = container.getParent()->getTransform().getMatrixLevel();
    if (m_containerMatrixLevel == parentLevel) return;
    updateInstances(
        ctx,
        container);
    auto& transform = container.modifyTransform();
    transform.m_dirtySnapshot = true;
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

                transform.setVolume(containerTransform.getVolume());

                transform.setQuatRotation(containerTransform.getQuatRotation());
                transform.setScale(containerTransform.getScale());

                transform.updateModelMatrix(parentTransform);

                idx++;
            }
        }
    }

    m_reservedCount = static_cast<uint32_t>(m_transforms.size());
    setActiveRange(0, m_reservedCount);

    container.m_instancer = this;
}

void GridGenerator::prepareInstances(
    const PrepareContext& ctx,
    Node& node)
{
    auto* type = node.m_typeHandle.toType();

    const auto count = m_zCount * m_xCount * m_yCount;

    m_transforms.reserve(count);

    for (int i = 0; i < count; i++) {
        auto& transform = m_transforms.emplace_back();
        transform.m_flags = type->resolveEntityFlags();
    }
}
