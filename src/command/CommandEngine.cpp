#include "command/CommandEngine.h"

#include "model/Node.h"

#include "command/NodeCommand.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

class RenderContext;

CommandEngine::CommandEngine(const Assets& assets)
    : m_assets(assets)
{}

void CommandEngine::prepare()
{
}

void CommandEngine::update(const RenderContext& ctx) noexcept
{
    processCanceled(ctx);
    processPending(ctx);
    processBlocked(ctx);
    processActive(ctx);
}

int CommandEngine::addCommand(std::unique_ptr<Command> pcmd) noexcept
{
    auto& cmd = m_pending.emplace_back(std::move(pcmd));
    return cmd->m_id;
}

bool CommandEngine::isCanceled(int commandId) noexcept
{
    return std::find(m_canceled.begin(), m_canceled.end(), commandId) != m_canceled.end();
}

bool CommandEngine::isValid(const RenderContext& ctx, Command* cmd) noexcept
{
    if (!cmd->isNode()) return true;

    auto objectID = (dynamic_cast<NodeCommand*>(cmd))->m_objectID;
    return ctx.m_registry->m_nodeRegistry->getNode(objectID);
}

void CommandEngine::cancel(int commandId) noexcept
{
    m_canceled.push_back(commandId);
}

void CommandEngine::processCanceled(const RenderContext& ctx) noexcept
{
    // NOTE KI can cancel only *EXISTING* commands not future commands
    if (m_canceled.empty()) return;

    for (auto& cmd : m_pending) {
        if (!isCanceled(cmd->m_id)) continue;
        cmd->m_canceled = true;
    }

    for (auto& cmd : m_active) {
        if (!isCanceled(cmd->m_id)) continue;
        cmd->m_canceled = true;
    }

    m_canceled.clear();
}

void CommandEngine::processPending(const RenderContext& ctx) noexcept
{
    // NOTE KI scripts cannot exist before node is in registry
    // => thus it MUST exist
    if (m_pending.empty()) return;

    for (auto& cmd : m_pending) {
        // canceled; discard
        if (cmd->m_canceled) continue;

        m_commands[cmd->m_id] = cmd.get();

        if (cmd->m_afterCommandId > 0) {
            auto prev = m_commands[cmd->m_afterCommandId];
            if (prev) {
                prev->m_next.push_back(cmd->m_id);
            }
        }

        m_blocked.emplace_back(std::move(cmd));
    }
    m_pending.clear();
}

void CommandEngine::processBlocked(const RenderContext& ctx) noexcept
{
    if (m_blocked.empty()) return;

    bool cleanup = false;
    for (auto& cmd : m_blocked) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanup = true;
            continue;
        }

        auto prev = m_commands[cmd->m_afterCommandId];
        if (!prev) {
            // NOTE KI if command without prev; then ready
            cmd->m_ready = true;
        }

        // NOTE KI execute flag can be set only when previous is finished
        if (!cmd->m_ready) continue;

        cleanup = true;

        if (cmd->isNode()) {
            auto nodeCmd = dynamic_cast<NodeCommand*>(cmd.get());
            const auto & node = ctx.m_registry->m_nodeRegistry->getNode(nodeCmd->m_objectID);
            if (!node) {
                cmd->m_canceled = true;
                continue;
            }
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
            m_blocked.begin(),
            m_blocked.end(),
            [this](auto& cmd) {
                if (cmd && cmd->m_canceled) m_commands.erase(cmd->m_id);
                return !cmd || cmd->m_canceled;
            });
        m_blocked.erase(it, m_blocked.end());
    }
}

void CommandEngine::processActive(const RenderContext& ctx) noexcept
{
    if (m_active.empty()) return;

    bool cleanup = false;
    for (auto& cmd : m_active) {
        // canceled; discard
        if (cmd->m_canceled) {
            cleanup = true;
            continue;
        }

        //// => Discard node; it has disappeared
        //if (!isValid(ctx, cmd.get())) {
        //    cmd->m_canceled = true;
        //    cleanup = true;
        //    continue;
        //}

        cmd->execute(ctx);

        if (cmd->m_finished) {
            for (auto nextId : cmd->m_next) {
                auto cmd = m_commands[nextId];
                if (!cmd) continue;
                cmd->m_ready = true;
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
