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
    const auto parentLevel = container.getParent()->getState().getMatrixLevel();
    if (m_containerMatrixLevel == parentLevel) return;
    updateInstances(
        ctx,
        container);
    auto& state = container.modifyState();
    state.m_dirtySnapshot = true;
    m_containerMatrixLevel = parentLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    Node& container)
{
    const auto& parentState = container.getParent()->getState();

    const auto& containerState = container.getState();
    int idx = 0;

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto& state = m_states[idx];

                const glm::vec3 pos{ x * m_xStep, y * m_yStep, z * m_zStep };

                state.setPosition(containerState.getPosition() + pos);

                state.setVolume(containerState.getVolume());

                state.setBaseTransform(containerState.getBaseTransform());
                state.setBaseScale(containerState.getBaseScale());

                state.setQuatRotation(containerState.getQuatRotation());
                state.setScale(containerState.getScale());

                state.updateModelMatrix(parentState);

                idx++;
            }
        }
    }

    m_reservedCount = static_cast<uint32_t>(m_states.size());
    setActiveRange(0, m_reservedCount);

    container.m_instancer = this;
}

void GridGenerator::prepareInstances(
    const PrepareContext& ctx,
    Node& node)
{
    auto* type = node.m_typeHandle.toType();

    const auto count = m_zCount * m_xCount * m_yCount;

    m_states.reserve(count);

    for (int i = 0; i < count; i++) {
        auto& state = m_states.emplace_back();
        state.m_flags = type->resolveEntityFlags();
    }
}
