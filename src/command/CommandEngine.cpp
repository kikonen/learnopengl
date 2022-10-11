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
    // NOTE KI scripts cannot exist before node is in registry
    // => thus it MUST exist
    for (auto& cmd : m_pending) {
        const auto& node = ctx.registry.getNode(cmd->m_objectID);
        // => Discard node; it has disappeared
        if (!node) continue;

        m_waiting.emplace_back(std::move(cmd));
    }
    m_pending.clear();

    bool cleanupWaiting = false;
    for (auto& cmd : m_waiting) {
        cmd->wait(ctx);
        if (!cmd->m_ready) continue;

        cleanupWaiting = true;

        const auto& node = ctx.registry.getNode(cmd->m_objectID);
        // => Discard node; it has disappeared
        if (!node) continue;

        cmd->bind(ctx, node);
        m_active.emplace_back(std::move(cmd));
    }

    bool cleanupActive = false;
    for (auto& cmd : m_active) {
        cmd->execute(ctx);
        if (cmd->m_finished) {
            cleanupActive = true;
            //cmd->m_callback(cmd->m_node);
        }
    }

    if (cleanupWaiting) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it =std::remove_if(
            m_waiting.begin(),
            m_waiting.end(),
            [](auto& cmd) { return !cmd || cmd->m_ready; });
        m_waiting.erase(it, m_waiting.end());
    }

    if (cleanupActive) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_active.begin(),
            m_active.end(),
            [](auto& cmd) { return cmd->m_finished; });
        m_active.erase(it, m_active.end());
    }
}

void CommandEngine::lua_moveTo(
    int objectID,
    float initialDelay,
    float secs,
    float x, float y, float z)
{
    m_pending.emplace_back(
        std::make_unique<MoveNode>(
            objectID,
            initialDelay,
            secs,
            glm::vec3{ x, y, z }));
}

void CommandEngine::lua_rotateTo(
    int objectID,
    float initialDelay,
    float secs,
    float x, float y, float z)
{
    m_pending.emplace_back(
        std::make_unique<RotateNode>(
            objectID,
            initialDelay,
            secs,
            glm::vec3{ x, y, z }));
}

void CommandEngine::lua_scaleTo(
    int objectID,
    float initialDelay,
    float secs,
    float x, float y, float z)
{
    m_pending.emplace_back(
        std::make_unique<ScaleNode>(
            objectID,
            initialDelay,
            secs,
            glm::vec3{ x, y, z }));
}
