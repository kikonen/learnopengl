#pragma once

#include <map>
#include <vector>

#include <sol/sol.hpp>

#include "Coroutine.h"

class CommandEngine;
class Node;
class ScriptEngine;

// Wrapper for CommandEngine calls from script
// => wrap reference to node so that no need to explicitly pass "id" from Lua
class CommandAPI final
{
public:
    CommandAPI(
        ScriptEngine* scriptEngine,
        CommandEngine* commandEngine,
        int objectID);
    ~CommandAPI() = default;

public:
    int lua_cancel(
        const sol::table& lua_opt,
        int commandId) noexcept;

    int lua_wait(
        const sol::table& lua_opt) noexcept;

    int lua_sync(
        const sol::table& lua_opt,
        const sol::table& lua_ids) noexcept;

    int lua_move(
        const sol::table& lua_opt,
        const sol::table& pos) noexcept;

    int lua_moveSpline(
        const sol::table& lua_opt,
        const sol::table& p,
        const sol::table& pos) noexcept;

    int lua_rotate(
        const sol::table& lua_opt,
        const sol::table& rot) noexcept;

    int lua_scale(
        const sol::table& lua_opt,
        const sol::table& scale) noexcept;

    int lua_start(
        const sol::table& lua_opt,
        sol::function fn) noexcept;

    int lua_resume(
        const sol::table& lua_opt,
        int coroutineID) noexcept;

private:
   CommandEngine* const m_commandEngine;
   ScriptEngine* const m_scriptEngine;
   const int m_objectID;

   std::vector<Coroutine*> m_coroutines;
};
