#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include "asset/Assets.h"


class UpdateContext;
class Registry;
class Command;

class CommandEngine final
{
public:
    CommandEngine(const Assets& assets);
    ~CommandEngine() = default;

    void prepare(Registry* registry);

    void update(const UpdateContext& ctx);

    int addCommand(std::unique_ptr<Command> cmd) noexcept;
    void cancel(int commandId) noexcept;

    bool isAlive(int commandId) noexcept;

private:
    bool isCanceled(int commandId) noexcept;
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

    std::unordered_map<int, Command*> m_commands;

    std::vector<int> m_canceled;

    //int m_oldestAliveCommandId = -1;
};
