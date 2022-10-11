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

public:
    void lua_moveTo(
        int objectID,
        float secs,
        float x, float y, float z);

    void lua_rotateTo(
        int objectID,
        float secs,
        float x, float y, float z);

    void lua_scaleTo(
        int objectID,
        float secs,
        float x, float y, float z);

private:
    std::vector<std::unique_ptr<Command>> m_active;
    std::vector<std::unique_ptr<Command>> m_pending;
};
