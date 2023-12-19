#pragma once

#include <map>
#include <vector>

#include <sol/sol.hpp>

#include "ki/size.h"

#include "Coroutine.h"

class Node;

namespace script
{
    class CommandEngine;
    class ScriptEngine;

    // Wrapper for CommandEngine calls from script
    // => wrap reference to node so that no need to explicitly pass "id" from Lua
    class CommandAPI final
    {
    public:
        CommandAPI(
            ScriptEngine* scriptEngine,
            CommandEngine* commandEngine,
            ki::node_id nodeId);
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
            const sol::table& lua_dir,
            const float lua_degrees) noexcept;

        int lua_scale(
            const sol::table& lua_opt,
            const sol::table& scale) noexcept;

        int lua_audioPlay(
            const sol::table& lua_opt) noexcept;

        int lua_audioPause(
            const sol::table& lua_opt) noexcept;

        int lua_audioStop(
            const sol::table& lua_opt) noexcept;

        int lua_start(
            const sol::table& lua_opt,
            sol::function fn) noexcept;

        int lua_resume(
            const sol::table& lua_opt,
            int coroutineID) noexcept;

    private:
        CommandEngine* const m_commandEngine;
        ScriptEngine* const m_scriptEngine;
        const ki::node_id m_nodeId;

        std::vector<Coroutine*> m_coroutines;
    };
}
