#include "CommandAPI.h"

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_format.h"

#include "model/Node.h"

#include "ScriptEngine.h"

#include "script/CommandEngine.h"
#include "script/CommandEngine_impl.h"
#include "script/CommandEntry.h"

#include "api/CancelCommand.h"
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
        float duration = 0;
        float speed = 1;
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
    CommandAPI::CommandAPI(
        ScriptEngine* scriptEngine,
        CommandEngine* commandEngine,
        ki::node_id nodeId)
        :m_scriptEngine(scriptEngine),
        m_commandEngine(commandEngine),
        m_nodeId(nodeId)
    {}

    CommandAPI::~CommandAPI() = default;

    int CommandAPI::lua_cancel(
        const sol::table& lua_opt,
        int commandId) noexcept
    {
        const auto opt = readOptions(lua_opt);

        //KI_INFO_OUT(fmt::format("wait: command={}, opt={}", commandId, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            CancelCommand{
                opt.duration,
                static_cast<script::command_id>(commandId)
            });
    }

    int CommandAPI::lua_wait(
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

    int CommandAPI::lua_sync(
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

    int CommandAPI::lua_move(
        const sol::table& lua_opt,
        const sol::table& lua_pos) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto pos = readVec3(lua_pos);

        //KI_INFO_OUT(fmt::format("move: node={}, pos={}, opt={}", m_nodeId, pos, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            MoveNode{
                m_nodeId,
                opt.duration,
                opt.relative,
                pos
            });
    }

    int CommandAPI::lua_moveSpline(
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
                m_nodeId,
                opt.duration,
                opt.relative,
                p,
                pos
            });
    }

    int CommandAPI::lua_rotate(
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
                m_nodeId,
                opt.duration,
                opt.relative,
                dir,
                lua_degrees
            });
    }

    int CommandAPI::lua_scale(
        const sol::table& lua_opt,
        const sol::table& lua_scale) noexcept
    {
        const auto opt = readOptions(lua_opt);
        const auto scale = readVec3(lua_scale);

        //KI_INFO_OUT(fmt::format("scale: node={}, scale={}, opt={}", m_nodeId, scale, opt.str()));

        return m_commandEngine->addCommand(
            opt.afterId,
            ScaleNode{
                m_nodeId,
                opt.duration,
                opt.relative,
                scale
            });
    }

    int CommandAPI::lua_set_text(
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
                m_nodeId,
                opt.duration,
                text
            });
    }

    int CommandAPI::lua_audioPlay(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPlay{
                m_nodeId,
                opt.index,
                opt.sync
            });
    }

    int CommandAPI::lua_audioPause(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioPause{
                m_nodeId,
                opt.index
            });
    }

    int CommandAPI::lua_audioStop(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AudioStop{
                m_nodeId,
                opt.index
            });
    }

    int CommandAPI::lua_animationPlay(
        const sol::table& lua_opt) noexcept
    {
        const auto opt = readOptions(lua_opt);

        return m_commandEngine->addCommand(
            opt.afterId,
            AnimationPlay{
                m_nodeId,
                opt.name,
                opt.speed,
                opt.repeat
            });
    }

    int CommandAPI::lua_start(
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
                m_nodeId,
                coroutine
            });
    }

    int CommandAPI::lua_resume_wrapper(
        const sol::table& lua_opt,
        int coroutineID) noexcept
    {
        const auto opt = readOptions(lua_opt);

        if (coroutineID < 0 && coroutineID >= m_coroutines.size()) {
            KI_WARN_OUT(fmt::format("resume: Invalid coroutine - node={}, coroutineID={}, opt={}", m_nodeId, coroutineID, opt.str()));
            return 0;
        }

        //KI_INFO_OUT(fmt::format("resume: node={}, coroutineID={}, opt={}", m_nodeId, coroutineID, opt.str()));

        auto& coroutine = m_coroutines[coroutineID];

        return m_commandEngine->addCommand(
            opt.afterId,
            ResumeNode{
                m_nodeId,
                coroutine
            });
    }
}
