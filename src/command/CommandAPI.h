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
        const sol::table& opt,
        const sol::table& pos);

    int lua_moveSpline(
        int objectID,
        const sol::table& opt,
        const sol::table& p,
        const sol::table& pos);

    int lua_rotate(
        int objectID,
        const sol::table& opt,
        const sol::table& rot);

    int lua_scale(
        int objectID,
        const sol::table& opt,
        const sol::table& scale);

    int lua_start(
        int objectID,
        int afterCommandId,
        sol::function fn,
        sol::variadic_args va);

private:
   CommandEngine& m_commandEngine;
   sol::thread& m_runner;
};
