#include "CommandEngine.h"

#include "model/Node.h"

#include "pool/IdGenerator.h"
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
    IdGenerator<script::command_id> ID_GENERATOR;

    constexpr size_t COMMANDS_SIZE = 10000;
}

namespace script
{
    CommandEngine::CommandEngine()
        : m_cleanupStep(5),
        m_pending{ std::make_unique<std::vector<CommandEntry>>() },
        m_blocked{ std::make_unique<std::vector<CommandEntry>>() },
        m_active{ std::make_unique<std::vector<CommandEntry>>() }
    {
        m_pending->reserve(COMMANDS_SIZE);
        m_blocked->reserve(COMMANDS_SIZE);
        m_active->reserve(COMMANDS_SIZE);
    }

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

    script::command_id CommandEngine::addCommand(
        CommandEntry&& entry) noexcept
    {
        std::lock_guard<std::mutex> lock{ m_pendingLock };

        auto& moved = m_pending->emplace_back(std::move(entry));

        moved.m_id = ID_GENERATOR.nextId();
        moved.m_cmd->setId(moved.m_id);

        return moved.m_id;
    }

    void CommandEngine::cancel(script::command_id commandId) noexcept
    {
        bool found = false;
        {
            std::lock_guard<std::mutex> lock{ m_pendingLock };
            for (auto& entry : *m_pending) {
                if (entry.m_id == commandId) {
                    killEntry(entry);
                    found = true;
                }
            }
        }

        if (!found) {
            for (auto& entry : *m_blocked) {
                if (entry.m_id == commandId) {
                    killEntry(entry);
                    m_blockedDeadCount++;
                    found = true;
                }
            }
        }

        if (!found) {
            for (auto& entry : *m_active) {
                if (entry.m_id == commandId) {
                    killEntry(entry);
                    m_activeDeadCount++;
                    found = true;
                }
            }
        }
    }

    bool CommandEngine::isAlive(script::command_id commandId) const noexcept
    {
        return m_alive.find(commandId) != m_alive.end();
    }

    bool CommandEngine::hasPending() const noexcept
    {
        std::lock_guard<std::mutex> lock{ m_pendingLock };
        return !m_pending->empty();
    }

    void CommandEngine::activateNext(const CommandEntry& prevEntry) noexcept
    {
        if (prevEntry.m_next.empty()) return;

        for (auto nextId : prevEntry.m_next) {
            for (auto& entry : *m_blocked) {
                if (entry.m_id == nextId) {
                    entry.m_ready = true;
                }
            }
        }
    }

    void CommandEngine::bindNext(CommandEntry& nextEntry) noexcept
    {
        const auto afterId = nextEntry.afterId;
        bool found = false;

        if (afterId > 0 && !found) {
            for (auto& entry : *m_blocked) {
                if (entry.m_id == afterId) {
                    entry.m_next.push_back(nextEntry.m_id);
                    found = true;
                    break;
                }
            }
        }

        if (afterId > 0 && !found) {
            for (auto& entry : *m_active) {
                if (entry.m_id == afterId) {
                    entry.m_next.push_back(nextEntry.m_id);
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            nextEntry.m_ready = true;
        }
    }

    void CommandEngine::killEntry(CommandEntry& deadEntry) noexcept
    {
        deadEntry.m_alive = false;
        m_alive.erase(deadEntry.m_id);

        activateNext(deadEntry);
    }

    void CommandEngine::processPending(const UpdateContext& ctx) noexcept
    {
        std::lock_guard<std::mutex> lock{ m_pendingLock };

        if (m_pending->empty()) return;

        for (auto& entry : *m_pending) {
            if (!entry.m_alive) continue;

            bindNext(entry);

            auto& moved = m_blocked->emplace_back(std::move(entry));

            m_alive.insert({ entry.m_id, true });
        }

        m_pending->clear();
    }

    void CommandEngine::processBlocked(const UpdateContext& ctx) noexcept
    {
        if (m_blocked->empty()) return;

        for (auto& entry : *m_blocked) {
            if (!entry.m_alive) continue;

            // NOTE KI execute flag can be set only when previous is finished
            if (!entry.m_ready) continue;

            Command* cmd{ entry.m_cmd };
            cmd->bind(ctx);

            if (cmd->isCompleted()) {
                killEntry(entry);
                m_blockedDeadCount++;
                continue;
            }

            auto& moved = m_active->emplace_back(std::move(entry));

            // NOTE KI mark for cleanup
            entry.m_alive = false;
            m_blockedDeadCount++;
        }
    }

    void CommandEngine::processActive(const UpdateContext& ctx)
    {
        if (m_active->empty()) return;

        for (auto& entry : *m_active) {
            if (!entry.m_alive) continue;

            Command* cmd{ entry.m_cmd };
            cmd->execute(ctx);

            if (cmd->isCompleted()) {
                killEntry(entry);
                m_activeDeadCount++;
            }
        }
    }

    void CommandEngine::processCleanup(const UpdateContext& ctx) noexcept
    {
        auto blockedCleanup = (m_blocked->size() / (float)m_blockedDeadCount) > 0.5;
        auto activeCleanup = (m_active->size() / (float)m_activeDeadCount) > 0.5;

        if (blockedCleanup) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_blocked->begin(),
                m_blocked->end(),
                [this](auto& entry) {
                    return !entry.m_alive;
                });
            m_blocked->erase(it, m_blocked->end());

            m_blockedDeadCount = 0;
        }

        if (activeCleanup) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_active->begin(),
                m_active->end(),
                [this](auto& entry) {
                    return !entry.m_alive;
                });
            m_active->erase(it, m_active->end());

            m_activeDeadCount = 0;
        }
    }
}
