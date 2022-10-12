#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "asset/Assets.h"

#include "command/Command.h"

class RenderContext;

class CommandEngine
{
public:
    CommandEngine();
    ~CommandEngine() = default;

    void prepare(
        const Assets& assets);

    void update(const RenderContext& ctx);

    bool isCanceled(int commandId);
    void cancel(int commandId);

public:
    int lua_cancel(
        float initialDelay,
        float secs,
        int commandId);

    int lua_moveTo(
        int objectID,
        float initialDelay,
        float secs,
        float x, float y, float z);

    int lua_rotateTo(
        int objectID,
        float initialDelay,
        float secs,
        float x, float y, float z);

    int lua_scaleTo(
        int objectID,
        float initialDelay,
        float secs,
        float x, float y, float z);

private:
    std::vector<std::unique_ptr<Command>> m_active;
    std::vector<std::unique_ptr<Command>> m_waiting;
    std::vector<std::unique_ptr<Command>> m_pending;

    std::vector<int> m_canceled;
};
