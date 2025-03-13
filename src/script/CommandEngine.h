#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "size.h"

struct UpdateContext;
class Registry;

namespace script
{
    struct CommandEntry;
    class CommandHandle;

    class CommandEngine final
    {
        friend class Cancel;
        friend class Sync;

    public:
        static void init() noexcept;
        static void release() noexcept;
        static CommandEngine& get() noexcept;

        CommandEngine();
        CommandEngine& operator=(const CommandEngine&) = delete;

        ~CommandEngine();

        void clear();

        void shutdown();
        void prepare(Registry* registry);

        void update(const UpdateContext& ctx);

        void cancelCommand(script::command_id commandId);

        template<typename T>
        script::command_id addCommand(
            script::command_id afterId,
            T&& cmd) noexcept;

        size_t getPendingCount() const noexcept;
        size_t getBlockedCount() const noexcept;
        size_t getActiveCount() const noexcept;

    protected:
        bool isAlive(script::command_id commandId) const noexcept;

        bool hasPending() const noexcept;

        void addPending(
            const CommandHandle& handle) noexcept;

        void cancel(script::command_id commandId) noexcept;

    private:
        void activateNext(const CommandEntry* prevEntry) noexcept;
        void bindNext(CommandEntry* nextEntry) noexcept;

        void killEntry(
            CommandHandle& handle,
            CommandEntry* deadEntry) noexcept;

        void processCanceled(const UpdateContext& ctx) noexcept;
        void processPending(const UpdateContext& ctx) noexcept;
        void processBlocked(const UpdateContext& ctx) noexcept;
        void processActive(const UpdateContext& ctx);
        void processCleanup(const UpdateContext& ctx) noexcept;

        //void updateOldest() noexcept;

    private:
        mutable std::mutex m_pendingLock{};

        std::vector<CommandHandle> m_pending;
        std::vector<CommandHandle> m_blocked;
        std::vector<CommandHandle> m_active;

        size_t m_blockedDeadCount{ 0 };
        size_t m_activeDeadCount{ 0 };

        // NOTE KI don't do cleanup on every iteration
        // - just minor temporary memory leak
        int m_cleanupIndex{ 0 };
        int m_cleanupStep{ 0 };

        std::unordered_map<script::command_id, CommandHandle> m_alive;
    };
}
