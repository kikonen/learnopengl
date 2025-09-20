#include "NodeCommandAPI.h"

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_format.h"

#include "ki/size.h"

#include "model/Node.h"

#include "physics/physics_util.h"

#include "nav/Path.h"

#include "script/CommandEngine.h"
#include "script/CommandEngine_impl.h"
#include "script/CommandEntry.h"
#include "script/ScriptSystem.h"

#include "script/command/Cancel.h"
#include "script/command/CancelMultiple.h"
#include "script/command/Wait.h"
#include "script/command/Sync.h"

#include "script/command/MoveNode.h"
#include "script/command/MoveSplineNode.h"
#include "script/command/MovePathNode.h"
#include "script/command/RotateNode.h"
#include "script/command/ScaleNode.h"
#include "script/command/ResumeNode.h"
#include "script/command/StartNode.h"
#include "script/command/SelectNode.h"
#include "script/command/EmitEvent.h"

#include "script/command/SetTextNode.h"
#include "script/command/SetVisibleNode.h"

#include "script/command/AudioPlay.h"
#include "script/command/AudioPause.h"
#include "script/command/AudioStop.h"

#include "script/command/ParticleEmit.h"
#include "script/command/ParticleStop.h"

#include "script/command/AnimationPlay.h"

#include "script/command/RayCast.h"
#include "script/command/RayCastMultiple.h"
#include "script/command/FindPath.h"

#include "script/command/EmitEvent.h"

#include "script/lua_util.h"

namespace {
    const static std::string TABLE_TMP = "tmp";
}

namespace script::api
{
    NodeCommandAPI::NodeCommandAPI(
        CommandEngine* const commandEngine)
        :m_commandEngine{ commandEngine }
    {}

    NodeCommandAPI::~NodeCommandAPI() = default;

    std::string NodeCommandAPI::str() const noexcept
    {
        return fmt::format("<CMD_API>");
    }

    int NodeCommandAPI::lua_cancel(
        const sol::table& lua_opt,
        int lua_commandId) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("wait: command={}, opt={}", commandId, opt.str()));

        std::vector<script::command_id> commandIds;

        script::command_id commandId = static_cast<script::command_id>(lua_commandId);

        return m_commandEngine->addCommand(
            opt.afterId,
            Cancel{
                opt.duration,
                commandId
            });
    }

    int NodeCommandAPI::lua_cancel_multiple(
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
            CancelMultiple{
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
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const glm::vec3& pos) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("move: node={}, pos={}, opt={}", m_nodeId, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.duration,
                opt.relative,
                pos
            });
    }

    int NodeCommandAPI::lua_move_spline(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const glm::vec3& p,
        const glm::vec3& pos) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("move_spline: node={}, p={}, pos={}, opt={}", m_nodeId, p, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveSplineNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.duration,
                opt.relative,
                p,
                pos
            });
    }

    int NodeCommandAPI::lua_move_path(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const std::vector<glm::vec3>& path) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("move: node={}, pos={}, opt={}", m_nodeId, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MovePathNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.duration,
                opt.relative,
                path
            });
    }

    int NodeCommandAPI::lua_rotate(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const glm::vec3& axis,
        const float lua_degrees) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("rotate: node={}, rot = {}, opt={}", m_nodeId, rot, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            RotateNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.duration,
                opt.relative,
                axis,
                lua_degrees
            });
    }

    int NodeCommandAPI::lua_scale(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const glm::vec3& scale) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("scale: node={}, scale={}, opt={}", m_nodeId, scale, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            ScaleNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.duration,
                opt.relative,
                scale
            });
    }

    int NodeCommandAPI::lua_set_text(
        pool::NodeHandle handle,
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
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                text
            });
    }

    int NodeCommandAPI::lua_set_visible(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        bool visible) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            SetVisibleNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                visible
            });
    }

    int NodeCommandAPI::lua_select(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        bool select,
        bool append) noexcept
    {
        auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            SelectNode{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                select,
                append
            });
    }

    int NodeCommandAPI::lua_audio_play(
        pool::NodeHandle handle,
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPlay{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.sid,
                opt.sync
            });
    }

    int NodeCommandAPI::lua_audio_pause(
        pool::NodeHandle handle,
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPause{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.sid
            });
    }

    int NodeCommandAPI::lua_audio_stop(
        pool::NodeHandle handle,
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioStop{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.sid
            });
    }

    int NodeCommandAPI::lua_animation_play(
        pool::NodeHandle handle,
        const sol::table& lua_opt) noexcept
    {
        auto opt = readOptions(lua_opt);

        if (opt.sid == 0) {
            opt.sid = SID(opt.name);
        }

        return m_commandEngine->addCommand(
            opt.afterId,
            AnimationPlay{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.sid,
                opt.speed,
                opt.repeat
            });
    }

    int NodeCommandAPI::lua_particle_emit(
        pool::NodeHandle handle,
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            ParticleEmit{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                opt.count,
                opt.sync
            });
    }

    int NodeCommandAPI::lua_particle_stop(
        pool::NodeHandle handle,
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            ParticleStop{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
            });
    }

    int NodeCommandAPI::lua_ray_cast(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const glm::vec3& lua_dir,
        bool notifyMiss,
        const sol::function& lua_callback) noexcept
    {
        const auto opt = readOptions(lua_opt);

        uint32_t collisionMask = physics::mask(physics::Category::player);

        auto callback = [this, handle, opt, lua_callback](int cid, const physics::RayHit& hit) {
            auto& scriptSystem = script::ScriptSystem::get();
            sol::table args = scriptSystem.getLua()[TABLE_TMP];

            auto* node = hit.handle.toNode();
            if (!node) return;

            args["cid"] = cid;
            args["data"] = hit;

            //Node* node = nullptr;
            //node = getNode();

            scriptSystem.invokeNodeFunction(
                handle.toNode(),
                opt.self,
                lua_callback,
                args);

            args["cid"] = nullptr;
            args["data"] = nullptr;
        };

        return m_commandEngine->addCommand(
            opt.afterId,
            RayCast{
                selectHandle(opt.nodeHandle, handle, opt.tagId),
                lua_dir,
                400.f,
                collisionMask,
                notifyMiss,
                callback});
    }

    int NodeCommandAPI::lua_ray_cast_multiple(
        pool::NodeHandle handle,
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

        auto callback = [this, handle, opt, lua_callback](int cid, const std::vector<physics::RayHit>& hits) {
            auto& scriptSystem = script::ScriptSystem::get();
            sol::table args = scriptSystem.getLua()[TABLE_TMP];

            for (const auto& hit : hits) {
                auto* node = hit.handle.toNode();
                if (!node) continue;

                args["cid"] = cid;
                args["data"] = hit;

                //Node* node = nullptr;
                //node = getNode();

                scriptSystem.invokeNodeFunction(
                    handle.toNode(),
                    opt.self,
                    lua_callback,
                    args);
            }

            args["cid"] = nullptr;
            args["data"] = nullptr;
            };

        return m_commandEngine->addCommand(
            opt.afterId,
            RayCastMultiple{
                handle,
                dirs,
                400.f,
                collisionMask,
                callback });
    }

    int NodeCommandAPI::lua_find_path(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const glm::vec3& startPos,
        const glm::vec3& endPos,
        const int maxPath,
        const sol::function& lua_callback) noexcept
    {
        const auto opt = readOptions(lua_opt);

        auto callback = [this, handle, opt, lua_callback](int cid, const nav::Path& path) {
            auto& scriptSystem = script::ScriptSystem::get();
            sol::table args = scriptSystem.getLua()[TABLE_TMP];

            args["cid"] = cid;
            args["data"] = path;

            scriptSystem.invokeNodeFunction(
                handle.toNode(),
                opt.self,
                lua_callback,
                args);

            args["cid"] = nullptr;
            args["data"] = nullptr;
        };

        return m_commandEngine->addCommand(
            opt.afterId,
            FindPath{
                handle,
                startPos,
                endPos,
                maxPath,
                callback });
    }

    int NodeCommandAPI::lua_invoke(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const sol::function& fn,
        const sol::optional<sol::table>& fn_args) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            InvokeFunction{
                handle,
                opt.self,
                fn,
                fn_args.has_value() ? fn_args.value() : sol::table{}
            });
    }

    int NodeCommandAPI::lua_emit(
        pool::NodeHandle handle,
        const sol::table& lua_opt,
        const sol::table& event) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto ev = readEvent(event);

        return m_commandEngine->addCommand(
            opt.afterId,
            EmitEvent{
                handle,
                ev.listenerId,
                ev.type,
                ev.data
            });
    }

    // https://thephd.dev/sol3-feature-complete
    void NodeCommandAPI::bind(sol::state& lua)
    {
        sol::usertype<NodeCommandAPI> t = lua.new_usertype<NodeCommandAPI>("NodeCommand");

        t["cancel"] = &NodeCommandAPI::lua_cancel;
        t["cancel_multiple"] = &NodeCommandAPI::lua_cancel_multiple;
        t["wait"] = &NodeCommandAPI::lua_wait;
        t["sync"] = &NodeCommandAPI::lua_sync;

        t["move"] = &NodeCommandAPI::lua_move;
        t["move_spline"] = &NodeCommandAPI::lua_move_spline;
        t["move_path"] = &NodeCommandAPI::lua_move_path;
        t["rotate"] = &NodeCommandAPI::lua_rotate;
        t["scale"] = &NodeCommandAPI::lua_scale;

        t["set_text"] = &NodeCommandAPI::lua_set_text;
        t["set_visible"] = &NodeCommandAPI::lua_set_visible;
        t["select"] = &NodeCommandAPI::lua_select;

        t["audio_play"] = &NodeCommandAPI::lua_audio_play;
        t["audio_pause"] = &NodeCommandAPI::lua_audio_pause;
        t["audio_stop"] = &NodeCommandAPI::lua_audio_stop;

        t["particle_emit"] = &NodeCommandAPI::lua_particle_emit;
        t["particle_stop"] = &NodeCommandAPI::lua_particle_stop;

        t["animation_play"] = &NodeCommandAPI::lua_animation_play;

        t["ray_cast"] = &NodeCommandAPI::lua_ray_cast;
        t["ray_cast_multiple"] = &NodeCommandAPI::lua_ray_cast_multiple;
        t["find_path"] = &NodeCommandAPI::lua_find_path;

        t["call"] = &NodeCommandAPI::lua_invoke;
        t["emit"] = &NodeCommandAPI::lua_emit;

        t.set_function(
            "__tostring",
            [](const NodeCommandAPI& v) {
                return v.str();
            }
        );
    }
}
