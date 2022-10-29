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
        float secs,
        int commandId);

    int lua_wait(
        int afterCommandId,
        float secs);

    int lua_move(
        int objectID,
        sol::table opt,
        sol::table pos);

    int lua_moveSpline(
        int objectID,
        sol::table opt,
        sol::table p,
        sol::table pos);

    int lua_rotate(
        int objectID,
        sol::table opt,
        sol::table rot);

    int lua_scale(
        int objectID,
        sol::table opt,
        sol::table scale);

    int lua_start(
        int objectID,
        int afterCommandId,
        sol::function fn,
        sol::variadic_args va);

private:
   CommandEngine& m_commandEngine;
   sol::thread& m_runner;
};
