#include "command/CommandEngine.h"

#include "model/Node.h"

#include "command/CancelCommand.h"

#include "command/NodeCommand.h"
#include "command/MoveNode.h"
#include "command/MoveSplineNode.h"
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
    processCanceled(ctx);
    processPending(ctx);
    processBlocked(ctx);
    processWaiting(ctx);
    processActive(ctx);
}

bool CommandEngine::isCanceled(int commandId)
{
    return std::find(m_canceled.begin(), m_canceled.end(), commandId) != m_canceled.end();
}

bool CommandEngine::isValid(const RenderContext& ctx, Command* cmd)
{
    if (!cmd->isNode()) return true;

    auto objectID = (dynamic_cast<NodeCommand*>(cmd))->m_objectID;
    return ctx.registry.getNode(objectID);
}

void CommandEngine::cancel(int commandId)
{
    m_canceled.push_back(commandId);
}

void CommandEngine::processCanceled(const RenderContext& ctx)
{
    // NOTE KI can cancel only *EXISTING* commands not future commands
    if (m_canceled.empty()) return;

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

void CommandEngine::processPending(const RenderContext& ctx)
{
    // NOTE KI scripts cannot exist before node is in registry
    // => thus it MUST exist
    if (m_pending.empty()) return;

    for (auto& cmd : m_pending) {
        // canceled; discard
        if (cmd->m_canceled) continue;

        // => Discard node; it has disappeared
        if (!isValid(ctx, cmd.get())) continue;

        m_commands[cmd->m_id] = cmd.get();

        auto prev = cmd->m_afterCommandId > 0 ? m_commands[cmd->m_afterCommandId] : nullptr;
        if (prev) {
            prev->m_next.push_back(cmd->m_id);
            m_blocked.emplace_back(std::move(cmd));
        }
        else {
            // NOTE KI either no prev or prev already executed and discarded
            m_waiting.emplace_back(std::move(cmd));
        }
    }
    m_pending.clear();
}

void CommandEngine::processBlocked(const RenderContext& ctx)
{
    if (m_blocked.empty()) return;

    bool cleanup = false;
    for (auto& cmd : m_blocked) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanup = true;
            continue;
        }

        // => Discard node; it has disappeared
        if (!isValid(ctx, cmd.get())) {
            cmd->m_canceled = true;
            cleanup = true;
            continue;
        }

        auto prev = m_commands[cmd->m_afterCommandId];
        if (!prev) {
            // NOTE KI *ODD* prev disappeared
            cmd->m_execute = true;
        }

        // NOTE KI execute flag can be set only when previous is finished
        if (!cmd->m_execute) continue;

        cleanup = true;
        m_waiting.emplace_back(std::move(cmd));
    }

    if (cleanup) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_blocked.begin(),
            m_blocked.end(),
            [this](auto& cmd) {
                if (cmd && cmd->m_canceled) m_commands.erase(cmd->m_id);
                return !cmd || cmd->m_canceled;
            });
        m_blocked.erase(it, m_blocked.end());
    }
}

void CommandEngine::processWaiting(const RenderContext& ctx)
{
    if (m_waiting.empty()) return;

    bool cleanup = false;
    for (auto& cmd : m_waiting) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanup = true;
            continue;
        }

        // => Discard node; it has disappeared
        if (!isValid(ctx, cmd.get())) {
            cmd->m_canceled = true;
            cleanup = true;
            continue;
        }

        cmd->wait(ctx);
        if (!cmd->m_ready) continue;

        cleanup = true;

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

    if (cleanup) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_waiting.begin(),
            m_waiting.end(),
            [this](auto& cmd) {
                if (cmd && cmd->m_canceled) m_commands.erase(cmd->m_id);
                return !cmd || cmd->m_ready || cmd->m_canceled;
            });
        m_waiting.erase(it, m_waiting.end());
    }
}

void CommandEngine::processActive(const RenderContext& ctx)
{
    if (m_active.empty()) return;

    bool cleanup = false;
    for (auto& cmd : m_active) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanup = true;
            continue;
        }

        // => Discard node; it has disappeared
        if (!isValid(ctx, cmd.get())) {
            cmd->m_canceled = true;
            cleanup = true;
            continue;
        }

        cmd->execute(ctx);

        if (cmd->m_finished) {
            for (auto nextId : cmd->m_next) {
                auto cmd = m_commands[nextId];
                if (!cmd) continue;
                cmd->m_execute = true;
            }
            cleanup = true;
            //cmd->m_callback(cmd->m_node);
        }
    }

    if (cleanup) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_active.begin(),
            m_active.end(),
            [this](auto& cmd) {
                if (cmd->m_finished || cmd->m_canceled) m_commands.erase(cmd->m_id);
                return cmd->m_finished || cmd->m_canceled;;
            });
        m_active.erase(it, m_active.end());
    }
}

int CommandEngine::lua_cancel(
    int afterCommandId,
    float initialDelay,
    float secs,
    int commandId)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<CancelCommand>(
            afterCommandId,
            initialDelay,
            secs,
            commandId));
    return cmd->m_id;
}

int CommandEngine::lua_moveTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<MoveNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}

int CommandEngine::lua_moveSplineTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float px, float py, float pz,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<MoveSplineNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ px, py, pz },
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}

int CommandEngine::lua_rotateTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<RotateNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}

int CommandEngine::lua_scaleTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float x, float y, float z)
{
    auto& cmd = m_pending.emplace_back(
        std::make_unique<ScaleNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ x, y, z }));
    return cmd->m_id;
}
