#pragma once

#include <map>
#include <vector>

#include <sol/sol.hpp>

#include "ki/size.h"

#include "Coroutine.h"

#include "pool/NodeHandle.h"

class Node;

namespace script
{
    class CommandEngine;
    class ScriptEngine;

    // Wrapper for CommandEngine calls from script
    // => wrap reference to node so that no need to explicitly pass "id" from Lua
    class NodeCommandAPI final
    {
    public:
        NodeCommandAPI(
            ScriptEngine* scriptEngine,
            CommandEngine* commandEngine,
            pool::NodeHandle handle);
        ~NodeCommandAPI();

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

        int lua_set_text(
            const sol::table& lua_opt,
            const sol::table& lua_text) noexcept;

        int lua_audioPlay(
            const sol::table& lua_opt) noexcept;

        int lua_audioPause(
            const sol::table& lua_opt) noexcept;

        int lua_audioStop(
            const sol::table& lua_opt) noexcept;

        int lua_animationPlay(
            const sol::table& lua_opt) noexcept;

        int lua_start(
            const sol::table& lua_opt,
            sol::function fn) noexcept;

        int lua_resume_wrapper(
            const sol::table& lua_opt,
            int coroutineID) noexcept;

    private:
        CommandEngine* const m_commandEngine;
        ScriptEngine* const m_scriptEngine;
        const pool::NodeHandle m_handle;

        std::vector<Coroutine*> m_coroutines;
    };
}
