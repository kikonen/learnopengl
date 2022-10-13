#pragma once

#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>

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

    void cancel(int commandId);

private:
    bool isCanceled(int commandId);
    bool isValid(const RenderContext& ctx, Command* cmd);

    void processCanceled(const RenderContext& ctx);
    void processPending(const RenderContext& ctx);
    void processBlocked(const RenderContext& ctx);
    void processWaiting(const RenderContext& ctx);
    void processActive(const RenderContext& ctx);
    
public:
    int lua_cancel(
        int afterCommandId,
        float initialDelay,
        float secs,
        int commandId);

    int lua_moveTo(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float secs,
        float x, float y, float z);

    int lua_rotateTo(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float secs,
        float x, float y, float z);

    int lua_scaleTo(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float secs,
        float x, float y, float z);

private:
    std::vector<std::unique_ptr<Command>> m_pending;
    std::vector<std::unique_ptr<Command>> m_blocked;
    std::vector<std::unique_ptr<Command>> m_waiting;
    std::vector<std::unique_ptr<Command>> m_active;

    std::map<int, Command*> m_commands;

    std::vector<int> m_canceled;
};
