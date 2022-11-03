#pragma once

#include <vector>
#include <map>
#include <memory>

#include "asset/Assets.h"

#include "command/Command.h"

class RenderContext;

class CommandEngine final
{
public:
    CommandEngine();
    ~CommandEngine() = default;

    void prepare(
        const Assets& assets) noexcept;

    void update(const RenderContext& ctx) noexcept;

    int addCommand(std::unique_ptr<Command> cmd) noexcept;
    void cancel(int commandId) noexcept;

private:
    bool isCanceled(int commandId) noexcept;
    bool isValid(const RenderContext& ctx, Command* cmd) noexcept;

    void processCanceled(const RenderContext& ctx) noexcept;
    void processPending(const RenderContext& ctx) noexcept;
    void processBlocked(const RenderContext& ctx) noexcept;
    void processActive(const RenderContext& ctx) noexcept;

private:
    std::vector<std::unique_ptr<Command>> m_pending;
    std::vector<std::unique_ptr<Command>> m_blocked;
    std::vector<std::unique_ptr<Command>> m_active;

    std::map<int, Command*> m_commands;

    std::vector<int> m_canceled;
};
