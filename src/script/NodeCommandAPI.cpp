#include "NodeCommandAPI.h"

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_format.h"

#include "ki/size.h"

#include "model/Node.h"

#include "ScriptEngine.h"

#include "script/CommandEngine.h"
#include "script/CommandEngine_impl.h"
#include "script/CommandEntry.h"

#include "api/Cancel.h"
#include "api/Wait.h"
#include "api/Sync.h"

#include "api/MoveNode.h"
#include "api/MoveSplineNode.h"
#include "api/RotateNode.h"
#include "api/ScaleNode.h"
#include "api/ResumeNode.h"
#include "api/StartNode.h"
#include "api/SetTextNode.h"

#include "api/AudioPlay.h"
#include "api/AudioPause.h"
#include "api/AudioStop.h"

#include "api/AnimationPlay.h"

namespace {
    struct CommandOptions {
        script::command_id afterId = 0;
        int index = 0;
        ki::sid_t sid = 0;
        float duration = 0.f;
        float speed = 1.f;
        bool relative = false;
        bool repeat = false;
        bool sync = false;

        std::string name;

        std::string str() const noexcept {
            return fmt::format(
                "<OPT: after={}, duration={}, speed={}, relative={}, repeat={}, sync={}, name={}>",
                afterId, duration, speed, relative, repeat, sync, name);
        }
    };

    CommandOptions readOptions(const sol::table& lua_opt) noexcept {
        CommandOptions opt;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "after") {
                opt.afterId = value.as<script::command_id>();
            }
            else if (k == "sid") {
                opt.sid = value.as<unsigned int>();
            }
            else if (k == "index") {
                opt.index = value.as<int>();
            }
            else if (k == "time") {
                opt.duration = value.as<float>();
            }
            else if (k == "duration") {
                opt.duration = value.as<float>();
            }
            else if (k == "speed") {
                opt.speed = value.as<float>();
            }
            else if (k == "relative") {
                opt.relative = value.as<bool>();
            }
            else if (k == "loop") {
                opt.repeat = value.as<bool>();
            }
            else if (k == "sync") {
                opt.sync = value.as<bool>();
            }
            else if (k == "name") {
                opt.name = value.as<std::string>();
            }
            });
        return opt;
    }

    glm::vec3 readVec3(const sol::table& v) noexcept {
        return glm::vec3{ v.get<float>(1), v.get<float>(2), v.get<float>(3) };
    }

    std::vector<script::command_id> readCommandIds(const sol::table& v) noexcept {
        std::vector<script::command_id> ids;
        v.for_each([&](sol::object const& key, sol::object const& value) {
            int id = value.as<script::command_id>();
            ids.push_back(id);
        });
        return ids;
    }
}

namespace script
{
    NodeCommandAPI::NodeCommandAPI(
        ScriptEngine* scriptEngine,
        CommandEngine* commandEngine,
        pool::NodeHandle handle)
        :m_scriptEngine{ scriptEngine },
        m_commandEngine{ commandEngine },
        m_handle{ handle }
    {}

    NodeCommandAPI::~NodeCommandAPI() = default;

    int NodeCommandAPI::lua_cancel(
        const sol::table& lua_opt,
        int commandId) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("wait: command={}, opt={}", commandId, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            Cancel{
                opt.duration,
                static_cast<script::command_id>(commandId)
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
        const sol::table& lua_pos) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto pos = readVec3(lua_pos);

        //KI_INFO_OUT(fmt::format("move: node={}, pos={}, opt={}", m_nodeId, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveNode{
                m_handle,
                opt.duration,
                opt.relative,
                pos
            });
    }

    int NodeCommandAPI::lua_moveSpline(
        const sol::table& lua_opt,
        const sol::table& lua_p,
        const sol::table& lua_pos) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto p = readVec3(lua_p);
        const auto pos = readVec3(lua_pos);

        //KI_INFO_OUT(fmt::format("move_spline: node={}, p={}, pos={}, opt={}", m_nodeId, p, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveSplineNode{
                m_handle,
                opt.duration,
                opt.relative,
                p,
                pos
            });
    }

    int NodeCommandAPI::lua_rotate(
        const sol::table& lua_opt,
        const sol::table& lua_dir,
        const float lua_degrees) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto dir = readVec3(lua_dir);

        //KI_INFO_OUT(fmt::format("rotate: node={}, rot = {}, opt={}", m_nodeId, rot, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            RotateNode{
                m_handle,
                opt.duration,
                opt.relative,
                dir,
                lua_degrees
            });
    }

    int NodeCommandAPI::lua_scale(
        const sol::table& lua_opt,
        const sol::table& lua_scale) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto scale = readVec3(lua_scale);

        //KI_INFO_OUT(fmt::format("scale: node={}, scale={}, opt={}", m_nodeId, scale, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            ScaleNode{
                m_handle,
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
                m_handle,
                opt.duration,
                text
            });
    }

    int NodeCommandAPI::lua_audioPlay(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPlay{
                m_handle,
                opt.sid,
                opt.sync
            });
    }

    int NodeCommandAPI::lua_audioPause(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPause{
                m_handle,
                opt.sid
            });
    }

    int NodeCommandAPI::lua_audioStop(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioStop{
                m_handle,
                opt.sid
            });
    }

    int NodeCommandAPI::lua_animationPlay(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AnimationPlay{
                m_handle,
                opt.name,
                opt.speed,
                opt.repeat
            });
    }

    int NodeCommandAPI::lua_start(
        const sol::table& lua_opt,
        sol::function fn) noexcept
    {
        const auto opt = readOptions(lua_opt);

        {
            m_coroutines.emplace_back(new Coroutine(m_scriptEngine->getLua(), fn, m_coroutines.size()));
        }
        auto& coroutine = m_coroutines[m_coroutines.size() - 1];

        //KI_INFO_OUT(fmt::format("start: node={}, opt={}, coroutine={}", m_nodeId, opt.str(), coroutine->m_id));

        return m_commandEngine->addCommand(
            opt.afterId,
            StartNode{
                m_handle,
                coroutine
            });
    }

    int NodeCommandAPI::lua_resume_wrapper(
        const sol::table& lua_opt,
        int coroutineID) noexcept
    {
        const auto opt = readOptions(lua_opt);

        if (coroutineID < 0 && coroutineID >= m_coroutines.size()) {
            KI_WARN_OUT(fmt::format("resume: Invalid coroutine - node={}, coroutineID={}, opt={}", m_handle.str(), coroutineID, opt.str()));
            return 0;
        }

        //KI_INFO_OUT(fmt::format("resume: node={}, coroutineID={}, opt={}", m_nodeId, coroutineID, opt.str()));

        auto& coroutine = m_coroutines[coroutineID];

        return m_commandEngine->addCommand(
            opt.afterId,
            ResumeNode{
                m_handle,
                coroutine
            });
    }
}
