#include "command/CommandEngine.h"

#include "model/Node.h"

#include "command/MoveNode.h"
#include "command/RotateNode.h"
#include "command/ScaleNode.h"

#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"

CommandEngine::CommandEngine()
{}

void CommandEngine::prepare(
    const Assets& assets)
{
}

void CommandEngine::update(const RenderContext& ctx)
{
    for (auto& cmd : m_pending) {
        const auto& node = ctx.registry.getNode(cmd->m_objectID);
        if (!node) continue;
        cmd->bind(ctx, node);
        m_active.push_back(std::move(cmd));
    }
    m_pending.clear();

    std::cout << ctx.clock.elapsedSecs << "\n";
    bool cleanup = false;
    for (auto& cmd : m_active) {
        if (!cmd) continue;
        cmd->execute(ctx);
        if (cmd->m_finished) {
            cleanup = true;
            //cmd->m_callback(cmd->m_node);
        }
    }

    if (cleanup) {
        std::remove_if(
            m_active.begin(),
            m_active.end(),
            [](auto& cmd) { return !cmd || cmd->m_finished; });
    }
}

void CommandEngine::lua_moveTo(
    int objectID,
    float secs,
    float x, float y, float z)
{
    m_pending.emplace_back(
        std::make_unique<MoveNode>(
            objectID,
            secs,
            glm::vec3{ x, y, z }));
}

void CommandEngine::lua_rotateTo(
    int objectID,
    float secs,
    float x, float y, float z)
{
    m_pending.emplace_back(
        std::make_unique<RotateNode>(
            objectID,
            secs,
            glm::vec3{ x, y, z }));
}

void CommandEngine::lua_scaleTo(
    int objectID,
    float secs,
    float x, float y, float z)
{
    m_pending.emplace_back(
        std::make_unique<ScaleNode>(
            objectID,
            secs,
            glm::vec3{ x, y, z }));
}
