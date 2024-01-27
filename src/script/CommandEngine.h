#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "ki/size.h"
#include "size.h"

#include "CommandEntry.h"

struct UpdateContext;
class Registry;

namespace script
{
    struct CommandEntry;

    class CommandEngine final
    {
    public:
        CommandEngine();
        ~CommandEngine() = default;

        void prepare(Registry* registry);

        void update(const UpdateContext& ctx);

        template<typename T>
        script::command_id addCommand(
            script::command_id afterId,
            T&& cmd) noexcept
        {
            //return addCommand({ afterId, std::move(cmd) });
            CommandEntry entry;
            entry.afterId = afterId;
            entry.set<T>(std::move(cmd));
            return addCommand(std::move(entry));
        }

        script::command_id addCommand(
            CommandEntry&& entry) noexcept;

        void cancel(script::command_id commandId) noexcept;

        bool isAlive(script::command_id commandId) const noexcept;

        bool hasPending() const noexcept;

    private:
        void activateNext(const CommandEntry& prevEntry) noexcept;
        void bindNext(CommandEntry& nextEntry) noexcept;
        void killEntry(CommandEntry& deadEntry) noexcept;

        void processCanceled(const UpdateContext& ctx) noexcept;
        void processPending(const UpdateContext& ctx) noexcept;
        void processBlocked(const UpdateContext& ctx) noexcept;
        void processActive(const UpdateContext& ctx);
        void processCleanup(const UpdateContext& ctx) noexcept;

        //void updateOldest() noexcept;

    private:
        mutable std::mutex m_pendingLock{};

        std::unique_ptr<std::vector<CommandEntry>> m_pending;
        std::unique_ptr<std::vector<CommandEntry>> m_blocked;
        std::unique_ptr<std::vector<CommandEntry>> m_active;

        size_t m_blockedDeadCount{ 0 };
        size_t m_activeDeadCount{ 0 };

        // NOTE KI don't do cleanup on every iteration
        // - just minor temporary memory leak
        int m_cleanupIndex{ 0 };
        int m_cleanupStep{ 0 };

        std::unordered_map<script::command_id, bool> m_alive;
    };
}
