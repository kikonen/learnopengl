#pragma once

#include <vector>
#include <map>
#include <memory>

#include "asset/Assets.h"

#include "api/Command.h"

#include "scene/RenderContext.h"

class RenderContext;

class CommandEngine final
{
public:
    CommandEngine(const Assets& assets);
    ~CommandEngine() = default;

    void prepare();

    void update(const RenderContext& ctx);

    int addCommand(std::unique_ptr<Command> cmd) noexcept;
    void cancel(int commandId) noexcept;

private:
    bool isCanceled(int commandId) noexcept;
    bool isValid(const RenderContext& ctx, Command* cmd) noexcept;

    void processCanceled(const RenderContext& ctx) noexcept;
    void processPending(const RenderContext& ctx) noexcept;
    void processBlocked(const RenderContext& ctx) noexcept;
    void processActive(const RenderContext& ctx);
    void processCleanup(const RenderContext& ctx) noexcept;

    void updateOldest() noexcept;

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

    std::map<int, Command*> m_commands;

    std::vector<int> m_canceled;

    int m_oldestAliveCommandId = -1;
};
