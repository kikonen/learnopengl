#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "command/Command.h"

class RenderContext;

class CommandEngine
{
public:
    CommandEngine();
    ~CommandEngine() = default;

    void executeCommands(const RenderContext& ctx);

public:
    void lua_moveTo(
        int objectID,
        float secs,
        std::array<float, 3> pos);

private:
    std::vector<std::unique_ptr<Command>> m_commands;
};
