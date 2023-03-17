#include "command/CommandEngine.h"

#include "model/Node.h"

#include "api/NodeCommand.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


CommandEngine::CommandEngine(const Assets& assets)
    : m_assets(assets),
    m_cleanupStep(5)
{}

void CommandEngine::prepare()
{
}

void CommandEngine::update(const UpdateContext& ctx)
{
    //updateOldest();
    processCanceled(ctx);
    processPending(ctx);
    processBlocked(ctx);
    processActive(ctx);
    processCleanup(ctx);
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

bool CommandEngine::isValid(const UpdateContext& ctx, Command* cmd) noexcept
{
    if (!cmd->isNode()) return true;

    auto objectID = (dynamic_cast<NodeCommand*>(cmd))->m_objectID;
    return ctx.m_registry->m_nodeRegistry->getNode(objectID);
}

void CommandEngine::cancel(int commandId) noexcept
{
    m_canceled.push_back(commandId);
}

bool CommandEngine::isAlive(int commandId) noexcept
{
    return m_commands.find(commandId) != m_commands.end();
}

void CommandEngine::activateNext(const Command* cmd) noexcept
{
    if (cmd->m_next.empty()) return;

    for (auto nextId : cmd->m_next) {
        const auto& it = m_commands.find(nextId);
        if (it != m_commands.end()) {
            it->second->m_ready = true;
        }
    }
}

void CommandEngine::processCanceled(const UpdateContext& ctx) noexcept
{
    // NOTE KI can cancel only *EXISTING* commands not future commands
    if (m_canceled.empty()) return;

    for (auto& cmd : m_pending) {
        if (!isCanceled(cmd->m_id)) continue;
        cmd->m_canceled = true;
    }

    for (auto& cmd : m_blocked) {
        if (!isCanceled(cmd->m_id)) continue;
        cmd->m_canceled = true;
    }

    for (auto& cmd : m_active) {
        if (!isCanceled(cmd->m_id)) continue;
        cmd->m_canceled = true;
    }

    m_canceled.clear();
}

void CommandEngine::processPending(const UpdateContext& ctx) noexcept
{
    // NOTE KI scripts cannot exist before node is in registry
    // => thus it MUST exist
    if (m_pending.empty()) return;

    for (auto& cmd : m_pending) {
        // canceled; discard
        if (cmd->m_canceled) continue;

        m_commands.insert(std::make_pair(cmd->m_id, cmd.get()));

        bool ready = true;
        if (cmd->m_afterCommandId > 0) {
            const auto& it = m_commands.find(cmd->m_afterCommandId);
            if (it != m_commands.end()) {
                it->second->m_next.push_back(cmd->m_id);
                ready = false;
            }
        }
        cmd->m_ready = ready;

        m_blocked.emplace_back(std::move(cmd));
    }
    m_pending.clear();
}

void CommandEngine::processBlocked(const UpdateContext& ctx) noexcept
{
    if (m_blocked.empty()) return;

    for (auto& cmd : m_blocked) {
        // canceled; discard
        if (cmd->m_canceled) {
            activateNext(cmd.get());
            m_blockedCleanup = true;
            continue;
        }

        // NOTE KI execute flag can be set only when previous is finished
        if (!cmd->m_ready) continue;

        m_blockedCleanup = true;

        if (cmd->isNode()) {
            auto* nodeCmd = dynamic_cast<NodeCommand*>(cmd.get());
            auto* node = ctx.m_registry->m_nodeRegistry->getNode(nodeCmd->m_objectID);
            if (!node) {
                activateNext(cmd.get());
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
}

void CommandEngine::processActive(const UpdateContext& ctx)
{
    if (m_active.empty()) return;

    for (auto& cmd : m_active) {
        if (!cmd->isCompleted()) {
            cmd->execute(ctx);
        }

        if (cmd->isCompleted()) {
            activateNext(cmd.get());
            m_activeCleanup = true;
        }
    }
}

void CommandEngine::processCleanup(const UpdateContext& ctx) noexcept
{
    // TODO KI current m_blocked logic requires cleanup on every step
    //m_cleanupIndex++;
    //if ((m_cleanupIndex % m_cleanupStep) != 0) return;

    if (m_blockedCleanup) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_blocked.begin(),
            m_blocked.end(),
            [this](auto& cmd) {
                if (cmd && cmd->m_canceled) m_commands.erase(cmd->m_id);
                return !cmd || cmd->m_canceled;
            });
        m_blocked.erase(it, m_blocked.end());

        m_blockedCleanup = false;
    }

    if (m_activeCleanup) {
        // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
        const auto& it = std::remove_if(
            m_active.begin(),
            m_active.end(),
            [this](auto& cmd) {
                if (cmd->m_finished || cmd->m_canceled) m_commands.erase(cmd->m_id);
                return cmd->m_finished || cmd->m_canceled;;
            });
        m_active.erase(it, m_active.end());

        m_activeCleanup = false;
    }
}

//void CommandEngine::updateOldest() noexcept
//{
//    int min = INT_MAX;
//    for (const auto& cmd : m_pending) {
//        if (cmd->m_id < min) {
//            min = cmd->m_id;
//        }
//    }
//    for (const auto& cmd : m_blocked) {
//        if (cmd->m_id < min) {
//            min = cmd->m_id;
//        }
//    }
//    for (const auto& cmd : m_active) {
//        if (cmd->m_id < min) {
//            min = cmd->m_id;
//        }
//    }
//    m_oldestAliveCommandId = min;
//}
