#include "GridGenerator.h"

#include <vector>
#include <random>

#include "util/debug.h"

#include "ki/sid.h"

#include "mesh/MeshType.h"
#include "mesh/Mesh.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


GridGenerator::GridGenerator()
{
}

void GridGenerator::prepareWT(
    const PrepareContext& ctx,
    Node& container)
{
    container.m_visible = false;
    prepareInstances(
        ctx,
        container);
}

void GridGenerator::updateWT(
    const UpdateContext& ctx,
    const Node& container)
{
    //if (m_setupDone) return;
    //m_setupDone = true;

    const auto containerLevel = container.getState().getMatrixLevel();
    if (m_containerMatrixLevel == containerLevel) return;
    updateInstances(
        ctx,
        container);
    auto& state = container.getState();
    state.m_dirtySnapshot = true;
    m_containerMatrixLevel = containerLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    const Node& container)
{
    const auto& containerState = container.getState();

    for (auto& handle : m_nodes)
    {
        auto* node = handle.toNode();
        if (!node) continue;
        auto& state = node->modifyState();
        state.setRotation(containerState.getRotation());
        //state.updateModelMatrix(containerState);
    }
}

void GridGenerator::prepareInstances(
    const PrepareContext& ctx,
    const Node& container)
{
    auto& nodeRegistry = NodeRegistry::get();
    auto& dispatcher = ctx.m_registry->m_dispatcherWorker;

    auto* type = container.m_typeHandle.toType();
    const auto& containerState = container.getState();

    const auto* parent = container.getParent();

    const auto count = m_zCount * m_xCount * m_yCount;

    std::vector<NodeState> states;
    states.reserve(count);

    for (int i = 0; i < count; i++) {
        auto& state = states.emplace_back();
        state.m_flags = type->resolveEntityFlags();
        state.setPivot(containerState.getPivot());
        state.setBaseRotation(containerState.getBaseRotation());
        state.setRotation(containerState.getRotation());
        state.setPosition(containerState.getPosition());
        state.setScale(containerState.getScale());
        state.setVolume(containerState.getVolume());
    }

    switch (m_mode) {
    case GeneratorMode::grid:
        prepareGrid(container, states);
        break;
    case GeneratorMode::random:
        prepareRandom(container, states);
        break;
    }

    m_nodes.reserve(states.size());
    for (int idx = 0; auto& state : states)
    {
        ki::node_id nodeId{ StringID::nextID() };
        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        node->m_name = fmt::format("grid-", idx);
        node->m_typeHandle = container.m_typeHandle;
        m_nodes.push_back(handle);

        event::Event evt { event::Type::node_add };
        evt.blob = std::make_unique<event::BlobData>();
        evt.blob->body.state = state;
        evt.body.node = {
            .target = handle.toId(),
            .parentId = parent->getId(),
        };
        assert(evt.body.node.target > 1);
        dispatcher->send(evt);
        idx++;
    }
}

void GridGenerator::prepareGrid(
    const Node& container,
    std::vector<NodeState>& states) const
{
    const auto& containerState = container.getState();
    int idx = 0;

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto& state = states[idx];

                const glm::vec3 pos{ x * m_xStep, y * m_yStep, z * m_zStep };

                state.setPosition(containerState.getPosition() + pos);

                idx++;
            }
        }
    }
}

void GridGenerator::prepareRandom(
    const Node& container,
    std::vector<NodeState>& states) const
{
    const auto& containerState = container.getState();
    const auto count = m_zCount * m_xCount * m_yCount;

    std::random_device devPos;
    std::random_device devScale;
    std::random_device devRot;

    std::mt19937 rngPos(devPos());
    rngPos.seed(m_seed.x);

    std::mt19937 rngScale(devScale());
    rngScale.seed(m_seed.y);

    std::mt19937 rngRot(devRot());
    rngRot.seed(m_seed.z);

    constexpr int RANGE = INT_MAX;
    std::uniform_int_distribution<std::mt19937::result_type> uniform_dist(0, RANGE);

    const float maxX = m_xCount * m_xStep;
    const float maxY = m_yCount * m_yStep;
    const float maxZ = m_zCount * m_zStep;

    for (int idx = 0; idx < count; idx++) {
        auto& state = states[idx];

        {
            glm::uvec3 v{
                uniform_dist(rngPos),
                uniform_dist(rngPos),
                uniform_dist(rngPos)
            };

            glm::vec3 d{
                static_cast<float>(v.x) / (float)RANGE,
                static_cast<float>(v.y) / (float)RANGE,
                static_cast<float>(v.z) / (float)RANGE,
            };

            //KI_INFO_OUT(fmt::format("r={}, v={}, p={}", RANGE, v, d));

            const glm::vec3 pos{
                maxX * d.x,
                maxY * d.y,
                maxZ * d.z };

            state.setPosition(containerState.getPosition() + pos);
        }
        {
            glm::uvec3 v{
                uniform_dist(rngRot),
                uniform_dist(rngRot),
                uniform_dist(rngRot)
            };

            glm::vec3 d{
                static_cast<float>(v.x) / (float)RANGE,
                static_cast<float>(v.y) / (float)RANGE,
                static_cast<float>(v.z) / (float)RANGE,
            };

            float degrees = 360.f * d.y;
            const auto rot = util::axisDegreesToQuat({ 0, 1.f, 0 }, degrees);

            state.setRotation(containerState.getRotation() * rot);
        }
        {
            glm::uvec3 v{
                uniform_dist(rngScale),
                uniform_dist(rngScale),
                uniform_dist(rngScale)
            };

            glm::vec3 d{
                static_cast<float>(v.x) / (float)RANGE,
                static_cast<float>(v.y) / (float)RANGE,
                static_cast<float>(v.z) / (float)RANGE,
            };

            const float scale = 0.8f + 0.4f * d.y;

            state.setScale(containerState.getScale() * scale);
        }
    }
}
