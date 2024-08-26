#include "GridGenerator.h"

#include <vector>
#include <random>

#include "util/debug.h"

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

void GridGenerator::prepareWT(
    const PrepareContext& ctx,
    Node& container)
{
    container.m_instancer = this;

    prepareInstances(
        ctx,
        container);

    prepareSnapshots(*ctx.m_registry->m_workerSnapshotRegistry);
}

void GridGenerator::updateWT(
    const UpdateContext& ctx,
    const Node& container)
{
    const auto parentLevel = container.getParent()->getState().getMatrixLevel();
    if (m_containerMatrixLevel == parentLevel) return;
    updateInstances(
        ctx,
        container);
    auto& state = container.getState();
    state.m_dirtySnapshot = true;
    m_containerMatrixLevel = parentLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    const Node& container)
{
    switch (m_mode) {
    case GeneratorMode::grid:
        updateGrid(ctx, container);
        break;
    case GeneratorMode::random:
        updateRandom(ctx, container);
        break;
    }

    m_reservedCount = static_cast<uint32_t>(m_states.size());
    setActiveRange(0, m_reservedCount);
}

void GridGenerator::updateGrid(
    const UpdateContext& ctx,
    const Node& container)
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

                state.setBaseRotation(containerState.getBaseRotation());
                state.setRotation(containerState.getRotation());
                state.setScale(containerState.getScale());

                state.updateModelMatrix(parentState);

                idx++;
            }
        }
    }
}

void GridGenerator::updateRandom(
    const UpdateContext& ctx,
    const Node& container)
{
    const auto& parentState = container.getParent()->getState();
    const auto& containerState = container.getState();
    const auto count = m_zCount * m_xCount * m_yCount;

    std::random_device devX;
    std::random_device devY;
    std::random_device devZ;

    std::mt19937 rngX(devX());
    rngX.seed(m_seed.x);

    std::mt19937 rngY(devY());
    rngY.seed(m_seed.y);

    std::mt19937 rngZ(devZ());
    rngZ.seed(m_seed.z);

    constexpr int RANGE = 2 ^ 31;
    std::uniform_int_distribution<std::mt19937::result_type> uniform_dist(0, RANGE);

    const float maxX = m_xCount * m_xStep;
    const float maxY = m_yCount * m_yStep;
    const float maxZ = m_zCount * m_zStep;

    for (int idx = 0; idx < count; idx++) {
        auto& state = m_states[idx];

        glm::vec3 d{
            (float)uniform_dist(rngX) / (float)RANGE,
            (float)uniform_dist(rngY) / (float)RANGE,
            (float)uniform_dist(rngZ) / (float)RANGE
        };

        //KI_INFO_OUT(fmt::format("p={}", d));

        const glm::vec3 pos{
            maxX * d.x,
            maxY * d.y,
            maxZ * d.z };

        state.setPosition(containerState.getPosition() + pos);

        state.setVolume(containerState.getVolume());

        state.setBaseRotation(containerState.getBaseRotation());
        state.setRotation(containerState.getRotation());
        state.setScale(containerState.getScale());

        state.updateModelMatrix(parentState);
    }
}

void GridGenerator::prepareInstances(
    const PrepareContext& ctx,
    const Node& container)
{
    auto* type = container.m_typeHandle.toType();

    const auto count = m_zCount * m_xCount * m_yCount;

    m_states.reserve(count);

    for (int i = 0; i < count; i++) {
        auto& state = m_states.emplace_back();
        state.m_flags = type->resolveEntityFlags();
    }
}
