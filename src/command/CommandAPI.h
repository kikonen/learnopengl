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
        int commandId) noexcept;

    int lua_wait(
        int afterCommandId,
        float secs) noexcept;

    int lua_sync(
        const sol::table& lua_opt,
        const sol::table& lua_ids) noexcept;

    int lua_move(
        int objectID,
        const sol::table& lua_opt,
        const sol::table& pos) noexcept;

    int lua_moveSpline(
        int objectID,
        const sol::table& lua_opt,
        const sol::table& p,
        const sol::table& pos) noexcept;

    int lua_rotate(
        int objectID,
        const sol::table& lua_opt,
        const sol::table& rot) noexcept;

    int lua_scale(
        int objectID,
        const sol::table& lua_opt,
        const sol::table& scale) noexcept;

    int lua_start(
        int objectID,
        const sol::table& lua_opt,
        sol::function fn,
        sol::variadic_args va) noexcept;

    int lua_resume(
        int objectID,
        const sol::table& lua_opt,
        const std::string& callbackFn) noexcept;

private:
   CommandEngine& m_commandEngine;
   sol::thread& m_runner;
};
