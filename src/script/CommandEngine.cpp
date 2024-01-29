#include "CommandEngine.h"
#include "CommandEngine_impl.h"

#include "model/Node.h"

#include "pool/NodeHandle.h"

#include "api/CancelCommand.h"
#include "api/Wait.h"
#include "api/Sync.h"

#include "CommandEntry.h"

#include "api/Command.h"

#include "api/NodeCommand.h"
#include "api/MoveNode.h"
#include "api/MoveSplineNode.h"
#include "api/RotateNode.h"
#include "api/ScaleNode.h"
#include "api/ResumeNode.h"
#include "api/StartNode.h"

#include "api/AudioPlay.h"
#include "api/AudioPause.h"
#include "api/AudioStop.h"

#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    constexpr size_t COMMANDS_SIZE = 10000;
}

namespace script
{
    CommandEngine::CommandEngine()
        : m_cleanupStep(5)
    {
        m_pending.reserve(COMMANDS_SIZE);
        m_blocked.reserve(COMMANDS_SIZE);
        m_active.reserve(COMMANDS_SIZE);
    }

    CommandEngine::~CommandEngine() = default;

    void CommandEngine::prepare(Registry* registry)
    {
        registry->m_dispatcher->addListener(
            event::Type::animate_wait,
            [this](const event::Event& e) {
                auto& anim = e.body.animate;
                KI_OUT(fmt::format("ANIM:wait {}\n", anim.target));
                addCommand<Wait>(
                    anim.after,
                    Wait{
                        anim.duration,
                    });
            });

        registry->m_dispatcher->addListener(
            event::Type::animate_move,
            [this](const event::Event& e) {
                auto& anim = e.body.animate;
                KI_OUT(fmt::format("ANIM:move {}\n", anim.target));
                addCommand(
                    anim.after,
                    MoveNode{
                        anim.target,
                        anim.duration,
                        anim.relative,
                        anim.data
                    });
            });

        registry->m_dispatcher->addListener(
            event::Type::animate_rotate,
            [this](const event::Event& e) {
                auto& anim = e.body.animate;
                KI_OUT(fmt::format("ANIM:rotate {}\n", anim.target));
                addCommand(
                    anim.after,
                    RotateNode {
                        anim.target,
                        anim.duration,
                        anim.relative,
                        anim.data,
                        anim.data2.x
                    });
            });
    }

    void CommandEngine::update(const UpdateContext& ctx)
    {
        //updateOldest();
        processPending(ctx);
        processBlocked(ctx);
        processActive(ctx);
        processCleanup(ctx);
    }

    void CommandEngine::addPending(
        const CommandHandle& handle) noexcept
    {
        std::lock_guard lock{ m_pendingLock };
        m_pending.emplace_back(handle);
    }

    void CommandEngine::cancel(script::command_id commandId) noexcept
    {
        const auto& it = m_alive.find(commandId);
        if (it != m_alive.end()) {
            auto handle = it->second;
            if (auto* entry = handle.toCommand()) {
                killEntry(handle, entry);
            }
        }
    }

    bool CommandEngine::isAlive(script::command_id commandId) const noexcept
    {
        return m_alive.find(commandId) != m_alive.end();
    }

    bool CommandEngine::hasPending() const noexcept
    {
        std::lock_guard lock{ m_pendingLock };
        return !m_pending.empty();
    }

    void CommandEngine::activateNext(const CommandEntry* prevEntry) noexcept
    {
        if (prevEntry->m_next.empty()) return;

        for (auto nextId : prevEntry->m_next) {
            const auto& it = m_alive.find(nextId);
            if (it != m_alive.end()) {
                if (auto* entry = it->second.toCommand()) {
                    entry->m_ready = true;
                }
            }
        }
    }

    void CommandEngine::bindNext(CommandEntry* nextEntry) noexcept
    {
        const auto& it = m_alive.find(nextEntry->afterId);

        if (it != m_alive.end()) {
            auto* entry = it->second.toCommand();
            if (entry) {
                entry->m_next.push_back(nextEntry->m_id);
            }
        } else {
            nextEntry->m_ready = true;
        }
    }

    void CommandEngine::killEntry(
        CommandHandle& handle,
        CommandEntry* deadEntry) noexcept
    {
        activateNext(deadEntry);
        deadEntry->m_alive = false;

        m_alive.erase(deadEntry->m_id);
        handle.release();
    }

    void CommandEngine::processPending(const UpdateContext& ctx) noexcept
    {
        std::lock_guard lock{ m_pendingLock };

        if (m_pending.empty()) return;

        for (auto& handle : m_pending) {
            auto* entry = handle.toCommand();

            if (!entry || !entry->m_alive) continue;

            bindNext(entry);

            m_blocked.emplace_back(handle);

            m_alive.insert({ handle.toId(), handle});
        }

        m_pending.clear();
    }

    void CommandEngine::processBlocked(const UpdateContext& ctx) noexcept
    {
        if (m_blocked.empty()) return;

        for (auto& handle : m_blocked) {
            auto* entry = handle.toCommand();

            if (!entry || !entry->m_alive || entry->m_active) {
                m_blockedDeadCount++;
                continue;
            }

            // NOTE KI execute flag can be set only when previous is finished
            if (!entry->m_ready) {
                //const auto& it = m_alive.find(entry->afterId);
                //auto* nextEntry = it != m_alive.end() ? it->second.toCommand() : nullptr;
                continue;
            }
            Command* cmd{ entry->m_cmd };
            cmd->bind(ctx);

            if (cmd->isCompleted()) {
                killEntry(handle, entry);
                m_blockedDeadCount++;
                continue;
            }

            m_active.emplace_back(handle);

            entry->m_active = true;
            m_blockedDeadCount++;
        }
    }

    void CommandEngine::processActive(const UpdateContext& ctx)
    {
        if (m_active.empty()) return;

        for (auto& handle : m_active) {
            auto* entry = handle.toCommand();

            if (!entry || !entry->m_alive) {
                m_activeDeadCount++;
                continue;
            }

            Command* cmd{ entry->m_cmd };
            cmd->execute(ctx);

            if (cmd->isCompleted()) {
                killEntry(handle, entry);
                m_activeDeadCount++;
            }
        }
    }

    void CommandEngine::processCleanup(const UpdateContext& ctx) noexcept
    {
        auto blockedCleanup = (m_blocked.size() / (float)m_blockedDeadCount) < 0.5;
        auto activeCleanup = (m_active.size() / (float)m_activeDeadCount) < 0.5;

        if (blockedCleanup) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_blocked.begin(),
                m_blocked.end(),
                [this](auto& handle) {
                    auto* entry = handle.toCommand();
                    return !entry || !entry->m_alive || entry->m_active;
                });
            m_blocked.erase(it, m_blocked.end());

            m_blockedDeadCount = 0;
        }

        if (activeCleanup) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_active.begin(),
                m_active.end(),
                [this](auto& handle) {
                    auto* entry = handle.toCommand();
                    return !entry || !entry->m_alive;
                });
            m_active.erase(it, m_active.end());

            m_activeDeadCount = 0;
        }
    }
}
