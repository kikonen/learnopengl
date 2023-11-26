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
        sol::function fn) noexcept;

    int lua_resume(
        int coroutineID,
        const sol::table& lua_opt,
        const std::string& callbackFn) noexcept;

private:
   CommandEngine* const m_commandEngine;
   ScriptEngine* const m_scriptEngine;
   const int m_objectID;

   std::vector<Coroutine*> m_coroutines;
};
