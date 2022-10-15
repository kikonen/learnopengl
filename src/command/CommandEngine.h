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
        const Assets& assets);

    void update(const RenderContext& ctx);

    int addCommand(std::unique_ptr<Command> cmd);
    void cancel(int commandId);

private:
    bool isCanceled(int commandId);
    bool isValid(const RenderContext& ctx, Command* cmd);

    void processCanceled(const RenderContext& ctx);
    void processPending(const RenderContext& ctx);
    void processBlocked(const RenderContext& ctx);
    void processWaiting(const RenderContext& ctx);
    void processActive(const RenderContext& ctx);

private:
    std::vector<std::unique_ptr<Command>> m_pending;
    std::vector<std::unique_ptr<Command>> m_blocked;
    std::vector<std::unique_ptr<Command>> m_waiting;
    std::vector<std::unique_ptr<Command>> m_active;

    std::map<int, Command*> m_commands;

    std::vector<int> m_canceled;
};
