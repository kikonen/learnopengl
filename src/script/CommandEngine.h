#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "asset/Assets.h"

#include "ki/size.h"
#include "size.h"

struct UpdateContext;
class Registry;

namespace script
{
    class Command;

    class CommandEngine final
    {
    public:
        CommandEngine(const Assets& assets);
        ~CommandEngine() = default;

        void prepare(Registry* registry);

        void update(const UpdateContext& ctx);

        script::command_id addCommand(std::unique_ptr<Command> cmd) noexcept;
        void cancel(script::command_id commandId) noexcept;

        bool isAlive(script::command_id commandId) noexcept;

    private:
        bool isCanceled(script::command_id commandId) noexcept;
        bool isValid(const UpdateContext& ctx, Command* cmd) noexcept;

        void activateNext(const Command* cmd) noexcept;

        void processCanceled(const UpdateContext& ctx) noexcept;
        void processPending(const UpdateContext& ctx) noexcept;
        void processBlocked(const UpdateContext& ctx) noexcept;
        void processActive(const UpdateContext& ctx);
        void processCleanup(const UpdateContext& ctx) noexcept;

        //void updateOldest() noexcept;

    private:
        const Assets& m_assets;

        std::mutex m_lock{};

        std::vector<std::unique_ptr<Command>> m_pending;
        std::vector<std::unique_ptr<Command>> m_blocked;
        std::vector<std::unique_ptr<Command>> m_active;

        bool m_blockedCleanup = false;
        bool m_pendingCleanup = false;
        bool m_activeCleanup = false;

        // NOTE KI don't do cleanup on every iteration
        // - just minor temporary memory leak
        int m_cleanupIndex = 0;
        int m_cleanupStep = 0;

        std::unordered_map<script::command_id, Command*> m_commands;

        std::vector<script::command_id> m_canceled;

        //script::command_id m_oldestAliveCommandId = -1;
    };
}
