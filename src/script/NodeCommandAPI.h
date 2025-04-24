#pragma once

#include <map>
#include <vector>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "script/lua_binding.h"

namespace script
{
    class CommandEngine;

    // Wrapper for CommandEngine calls from script
    // => wrap reference to node so that no need to explicitly pass "id" from Lua
    class NodeCommandAPI final
    {
    public:
        NodeCommandAPI(
            CommandEngine* const commandEngine,
            pool::NodeHandle handle);
        ~NodeCommandAPI();

        std::string str() const noexcept;

    public:
        int lua_cancel(
            const sol::table& lua_opt,
            const sol::table& lua_commandIds) noexcept;

        int lua_wait(
            const sol::table& lua_opt) noexcept;

        int lua_sync(
            const sol::table& lua_opt,
            const sol::table& lua_ids) noexcept;

        int lua_move(
            const sol::table& lua_opt,
            const glm::vec3& pos) noexcept;

        int lua_move_spline(
            const sol::table& lua_opt,
            const glm::vec3& p,
            const glm::vec3& pos) noexcept;

        int lua_rotate(
            const sol::table& lua_opt,
            const glm::vec3& axis,
            const float lua_degrees) noexcept;

        int lua_scale(
            const sol::table& lua_opt,
            const glm::vec3& scale) noexcept;

        int lua_set_text(
            const sol::table& lua_opt,
            const sol::table& lua_text) noexcept;

        int lua_set_visible(
            const sol::table& lua_opt,
            bool visible) noexcept;

        int lua_select(
            const sol::table& lua_opt,
            bool select,
            bool append) noexcept;

        int lua_audio_play(
            const sol::table& lua_opt) noexcept;

        int lua_audio_pause(
            const sol::table& lua_opt) noexcept;

        int lua_audio_stop(
            const sol::table& lua_opt) noexcept;

        int lua_animation_play(
            const sol::table& lua_opt) noexcept;

        int lua_particle_emit(
            const sol::table& lua_opt) noexcept;

        int lua_particle_stop(
            const sol::table& lua_opt) noexcept;

        int lua_ray_cast(
            const sol::table& lua_opt,
            const sol::table& lua_dirs,
            const sol::function& callback) noexcept;

        int lua_invoke(
            const sol::table& lua_opt,
            const sol::function& fn,
            const sol::optional<sol::table>& fn_args) noexcept;

        int lua_emit(
            const sol::table& lua_opt,
            const sol::table& event) noexcept;

    private:
        CommandEngine* const m_commandEngine;
        const pool::NodeHandle m_handle;
    };
}
