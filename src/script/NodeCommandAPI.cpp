#include "NodeCommandAPI.h"

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_format.h"

#include "ki/size.h"

#include "model/Node.h"

#include "physics/physics_util.h"

#include "script/CommandEngine.h"
#include "script/CommandEngine_impl.h"
#include "script/CommandEntry.h"
#include "script/ScriptSystem.h"

#include "api/Cancel.h"
#include "api/Wait.h"
#include "api/Sync.h"

#include "api/MoveNode.h"
#include "api/MoveSplineNode.h"
#include "api/RotateNode.h"
#include "api/ScaleNode.h"
#include "api/ResumeNode.h"
#include "api/StartNode.h"
#include "api/SelectNode.h"
#include "api/EmitEvent.h"

#include "api/SetTextNode.h"
#include "api/SetVisibleNode.h"

#include "api/AudioPlay.h"
#include "api/AudioPause.h"
#include "api/AudioStop.h"

#include "api/ParticleEmit.h"
#include "api/ParticleStop.h"

#include "api/AnimationPlay.h"

#include "api/EmitEvent.h"

#include "script/lua_util.h"

namespace {
}

namespace script
{
    NodeCommandAPI::NodeCommandAPI(
        CommandEngine* const commandEngine,
        pool::NodeHandle handle)
        :m_commandEngine{ commandEngine },
        m_handle{ handle }
    {}

    NodeCommandAPI::~NodeCommandAPI() = default;

    std::string NodeCommandAPI::str() const noexcept
    {
        return fmt::format("<CMD_API: {}>", m_handle.str());
    }

    int NodeCommandAPI::lua_cancel(
        const sol::table& lua_opt,
        const sol::table& lua_commandIds) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("wait: command={}, opt={}", commandId, opt.str()));

        std::vector<script::command_id> commandIds;

        lua_commandIds.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& commandId = key.as<int>();
            commandIds.push_back(static_cast<script::command_id>(commandId));
            });

        return m_commandEngine->addCommand(
            opt.afterId,
            Cancel{
                opt.duration,
                commandIds
            });
    }

    int NodeCommandAPI::lua_wait(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("wait: opt={}", opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            Wait{
                opt.duration
            });
    }

    int NodeCommandAPI::lua_sync(
        const sol::table& lua_opt,
        const sol::table& lua_ids) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto commandIds = readCommandIds(lua_ids);

        //KI_INFO_OUT(fmt::format("sync: commandIds={}, opt={}", commandIds[0], opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            Sync{
                opt.duration,
                commandIds
            });
    }

    int NodeCommandAPI::lua_move(
        const sol::table& lua_opt,
        const glm::vec3& pos) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("move: node={}, pos={}, opt={}", m_nodeId, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveNode{
                getHandle(opt.nodeId, m_handle),
                opt.duration,
                opt.relative,
                pos
            });
    }

    int NodeCommandAPI::lua_move_spline(
        const sol::table& lua_opt,
        const glm::vec3& p,
        const glm::vec3& pos) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("move_spline: node={}, p={}, pos={}, opt={}", m_nodeId, p, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveSplineNode{
                getHandle(opt.nodeId, m_handle),
                opt.duration,
                opt.relative,
                p,
                pos
            });
    }

    int NodeCommandAPI::lua_rotate(
        const sol::table& lua_opt,
        const glm::vec3& axis,
        const float lua_degrees) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("rotate: node={}, rot = {}, opt={}", m_nodeId, rot, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            RotateNode{
                getHandle(opt.nodeId, m_handle),
                opt.duration,
                opt.relative,
                axis,
                lua_degrees
            });
    }

    int NodeCommandAPI::lua_scale(
        const sol::table& lua_opt,
        const glm::vec3& scale) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("scale: node={}, scale={}, opt={}", m_nodeId, scale, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            ScaleNode{
                getHandle(opt.nodeId, m_handle),
                opt.duration,
                opt.relative,
                scale
            });
    }

    int NodeCommandAPI::lua_set_text(
        const sol::table& lua_opt,
        const sol::table& lua_text) noexcept
    {
        const auto opt = readOptions(lua_opt);

        std::string text;
        lua_text.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "text") {
                text = value.as<std::string>();
            }
        });

        return m_commandEngine->addCommand(
            opt.afterId,
            SetTextNode{
                getHandle(opt.nodeId, m_handle),
                text
            });
    }

    int NodeCommandAPI::lua_set_visible(
        const sol::table& lua_opt,
        bool visible) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            SetVisibleNode{
                getHandle(opt.nodeId, m_handle),
                visible
            });
    }

    int NodeCommandAPI::lua_select(
        const sol::table& lua_opt,
        bool select,
        bool append) noexcept
    {
        auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            SelectNode{
                getHandle(opt.nodeId, m_handle),
                select,
                append
            });
    }

    int NodeCommandAPI::lua_audio_play(
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPlay{
                getHandle(opt.nodeId, m_handle),
                opt.sid,
                opt.sync
            });
    }

    int NodeCommandAPI::lua_audio_pause(
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPause{
                getHandle(opt.nodeId, m_handle),
                opt.sid
            });
    }

    int NodeCommandAPI::lua_audio_stop(
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioStop{
                getHandle(opt.nodeId, m_handle),
                opt.sid
            });
    }

    int NodeCommandAPI::lua_animation_play(
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AnimationPlay{
                getHandle(opt.nodeId, m_handle),
                opt.sid,
                opt.speed,
                opt.repeat
            });
    }

    int NodeCommandAPI::lua_particle_emit(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            ParticleEmit{
                getHandle(opt.nodeId, m_handle),
                opt.count,
                opt.sync
            });
    }

    int NodeCommandAPI::lua_particle_stop(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            ParticleStop{
                getHandle(opt.nodeId, m_handle)
            });
    }

    int NodeCommandAPI::lua_ray_cast(
        const sol::table& lua_opt,
        const sol::table& lua_dirs,
        const sol::function& lua_callback) noexcept
    {
        const auto opt = readOptions(lua_opt);

        std::vector<glm::vec3> dirs;

        lua_dirs.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& dir = value.as<glm::vec3>();
            dirs.push_back(dir);
            });

        uint32_t collisionMask = physics::mask(physics::Category::player);

        auto callback = [this, opt, lua_callback](const std::vector<physics::RayHit>& hits) {
            auto& se = script::ScriptSystem::get();
            sol::table args = se.getLua().create_table();

            for (const auto& hit : hits) {
                auto* node = hit.handle.toNode();
                if (!node) continue;

                args["data"] = hit;

                //Node* node = nullptr;
                //node = getNode();

                se.invokeNodeFunction(
                    node,
                    opt.self,
                    lua_callback,
                    args);
            }
        };

        return m_commandEngine->addCommand(
            opt.afterId,
            RayCast{
                m_handle,
                dirs,
                400.f,
                collisionMask,
                callback});
    }

    int NodeCommandAPI::lua_invoke(
        const sol::table& lua_opt,
        const sol::function& fn,
        const sol::optional<sol::table>& fn_args) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            InvokeFunction{
                m_handle,
                opt.self,
                fn,
                fn_args.has_value() ? fn_args.value() : sol::table{}
            });
    }

    int NodeCommandAPI::lua_emit(
        const sol::table& lua_opt,
        const sol::table& event) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto ev = readEvent(event);

        return m_commandEngine->addCommand(
            opt.afterId,
            EmitEvent{
                m_handle,
                ev.listenerId,
                ev.type,
                ev.data
            });
    }
}
