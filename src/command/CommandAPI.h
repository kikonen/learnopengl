#pragma once

#include <sol/sol.hpp>

class CommandEngine;

class CommandAPI final
{
public:
    CommandAPI(
        CommandEngine& commandEngine,
        sol::thread& m_runner);
    ~CommandAPI() = default;

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
        bool relative,
        float x, float y, float z);

    int lua_moveSplineTo(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float secs,
        bool relative,
        float px, float py, float pz,
        float x, float y, float z);

    int lua_rotateTo(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float secs,
        bool relative,
        float x, float y, float z);

    int lua_scaleTo(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float secs,
        bool relative,
        float x, float y, float z);

    int lua_start(
        int afterCommandId,
        int objectID,
        float initialDelay,
        sol::function fn,
        sol::variadic_args va);

private:
   CommandEngine& m_commandEngine;
   sol::thread& m_runner;
};
