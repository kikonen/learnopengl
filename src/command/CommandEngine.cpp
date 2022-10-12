#include "command/CommandEngine.h"

#include "model/Node.h"

#include "command/CancelCommand.h"

#include "command/NodeCommand.h"
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
    // NOTE KI can cancel only *EXISTING* commands not future commands
    if (!m_canceled.empty()) {
        for (auto& cmd : m_pending) {
            if (!isCanceled(cmd->m_id)) continue;
            cmd->m_canceled = true;
        }

        for (auto& cmd : m_waiting) {
            if (!isCanceled(cmd->m_id)) continue;
            cmd->m_canceled = true;
        }

        for (auto& cmd : m_active) {
            if (!isCanceled(cmd->m_id)) continue;
            cmd->m_canceled = true;
        }

        m_canceled.clear();
    }

    // NOTE KI scripts cannot exist before node is in registry
    // => thus it MUST exist
    if (!m_pending.empty()) {
        for (auto& cmd : m_pending) {
            // canceled; discard
            if (cmd->m_canceled) continue;

            if (cmd->isNode()) {
                auto nodeCmd = dynamic_cast<NodeCommand*>(cmd.get());
                const auto& node = ctx.registry.getNode(nodeCmd->m_objectID);
                // => Discard node; it has disappeared
                if (!node) continue;
            }

            m_waiting.emplace_back(std::move(cmd));
        }
        m_pending.clear();
    }

    bool cleanupWaiting = false;
    for (auto& cmd : m_waiting) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanupWaiting = true;
            continue;
        }

        cmd->wait(ctx);
        if (!cmd->m_ready) continue;

        cleanupWaiting = true;

        if (cmd->isNode()) {
            auto nodeCmd = dynamic_cast<NodeCommand*>(cmd.get());
            const auto& node = ctx.registry.getNode(nodeCmd->m_objectID);
            // => Discard node; it has disappeared
            if (!node) continue;
        }

        if (cmd->isNode()) {
            auto nodeCmd = dynamic_cast<NodeCommand*>(cmd.get());
            const auto& node = ctx.registry.getNode(nodeCmd->m_objectID);
            nodeCmd->bind(ctx, node);
        }
        else {
            cmd->bind(ctx);
        }
        m_active.emplace_back(std::move(cmd));
    }

    bool cleanupActive = false;
    for (auto& cmd : m_active) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanupActive = true;
            continue;
        }

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
            [](auto& cmd) { return !cmd || cmd->m_ready || cmd->m_canceled; });
        m_waiting.erase(it, m_waiting.end());
    }

    if (cleanupActive) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_active.begin(),
            m_active.end(),
            [](auto& cmd) { return cmd->m_finished || cmd->m_canceled;; });
        m_active.erase(it, m_active.end());
    }
}

bool CommandEngine::isCanceled(int commandId)
{
    return std::find(m_canceled.begin(), m_canceled.end(), commandId) != m_canceled.end();
}

void CommandEngine::cancel(int commandId)
{
    m_canceled.push_back(commandId);
}

int CommandEngine::lua_cancel(
    float initialDelay,
    float secs,
    int commandId)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<CancelCommand>(
            initialDelay,
            secs,
            commandId));
    return cmd->m_id;
}

int CommandEngine::lua_moveTo(
    int objectID,
    float initialDelay,
    float secs,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<MoveNode>(
            objectID,
            initialDelay,
            secs,
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}

int CommandEngine::lua_rotateTo(
    int objectID,
    float initialDelay,
    float secs,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<RotateNode>(
            objectID,
            initialDelay,
            secs,
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}

int CommandEngine::lua_scaleTo(
    int objectID,
    float initialDelay,
    float secs,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<ScaleNode>(
            objectID,
            initialDelay,
            secs,
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}
