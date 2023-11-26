#include "command/CommandAPI.h"

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_format.h"

#include "model/Node.h"

#include "ScriptEngine.h"

#include "command/CommandEngine.h"

#include "api/CancelCommand.h"
#include "api/Wait.h"
#include "api/Sync.h"

#include "api/MoveNode.h"
#include "api/MoveSplineNode.h"
#include "api/RotateNode.h"
#include "api/ScaleNode.h"
#include "api/ResumeNode.h"
#include "api/StartNode.h"

namespace {
    struct CommandOptions {
        int afterId = 0;
        float duration = 0;
        bool relative = false;
        bool repeat = false;

        const std::string str() const noexcept {
            return fmt::format("<OPT: after={}, duration={}, relative={}, repeat={}>", afterId, duration, relative, repeat);
        }
    };

    CommandOptions readOptions(const sol::table& lua_opt) noexcept {
        CommandOptions opt;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "after") {
                opt.afterId = value.as<int>();
            }
            else if (k == "time") {
                opt.duration = value.as<float>();
            }
            else if (k == "duration") {
                opt.duration = value.as<float>();
            }
            else if (k == "relative") {
                opt.relative = value.as<bool>();
            }
            else if (k == "loop") {
                opt.repeat = value.as<bool>();
            }
        });
        return opt;
    }

    glm::vec3 readVec3(const sol::table& v) noexcept {
        return glm::vec3{ v.get<float>(1), v.get<float>(2), v.get<float>(3) };
    }

    std::vector<int> readIds(const sol::table& v) noexcept {
        std::vector<int> ids;
        v.for_each([&](sol::object const& key, sol::object const& value) {
            int id = value.as<int>();
            ids.push_back(id);
        });
        return ids;
    }
}


CommandAPI::CommandAPI(
    ScriptEngine* scriptEngine,
    CommandEngine* commandEngine,
    const int objectID)
    :m_scriptEngine(scriptEngine),
    m_commandEngine(commandEngine),
    m_objectID(objectID)
{}

int CommandAPI::lua_cancel(
    int afterCommandId,
    float duration,
    int commandId) noexcept
{
    KI_INFO_OUT(fmt::format("wait: after={}, duration={}, command={}", afterCommandId, duration, commandId));

    return m_commandEngine->addCommand(
        std::make_unique<CancelCommand>(
            afterCommandId,
            duration,
            commandId));
}

int CommandAPI::lua_wait(
    int afterCommandId,
    float duration) noexcept
{
    KI_INFO_OUT(fmt::format("wait: after={}, duration={}", afterCommandId, duration));

    return m_commandEngine->addCommand(
        std::make_unique<Wait>(
            afterCommandId,
            duration));
}

int CommandAPI::lua_sync(
    const sol::table& lua_opt,
    const sol::table& lua_ids) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto commandIds = readIds(lua_ids);

    KI_INFO_OUT(fmt::format("sync: commandIds={}, opt={}", commandIds[0], opt.str()));

    return m_commandEngine->addCommand(
        std::make_unique<Sync>(
            opt.afterId,
            opt.duration,
            commandIds));
}

int CommandAPI::lua_move(
    int _,
    const sol::table& lua_opt,
    const sol::table& lua_pos) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto pos = readVec3(lua_pos);

    KI_INFO_OUT(fmt::format("move: node={}, pos={}, opt={}", m_objectID, pos, opt.str()));

    return m_commandEngine->addCommand(
        std::make_unique<MoveNode>(
            opt.afterId,
            m_objectID,
            opt.duration,
            opt.relative,
            pos));
}

int CommandAPI::lua_moveSpline(
    int _,
    const sol::table& lua_opt,
    const sol::table& lua_p,
    const sol::table& lua_pos) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto p = readVec3(lua_p);
    const auto pos = readVec3(lua_pos);

    KI_INFO_OUT(fmt::format("move_spline: node={}, p={}, pos={}, opt={}", m_objectID, p, pos, opt.str()));

    return m_commandEngine->addCommand(
        std::make_unique<MoveSplineNode>(
            opt.afterId,
            m_objectID,
            opt.duration,
            opt.relative,
            p,
            pos));
}

int CommandAPI::lua_rotate(
    int _,
    const sol::table& lua_opt,
    const sol::table& lua_rot) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto rot = readVec3(lua_rot);

    KI_INFO_OUT(fmt::format("rotate: node={}, rot = {}, opt={}", m_objectID, rot, opt.str()));

    return m_commandEngine->addCommand(
        std::make_unique<RotateNode>(
            opt.afterId,
            m_objectID,
            opt.duration,
            opt.relative,
            rot));
}

int CommandAPI::lua_scale(
    int _,
    const sol::table& lua_opt,
    const sol::table& lua_scale) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto scale = readVec3(lua_scale);

    KI_INFO_OUT(fmt::format("scale: node={}, scale={}, opt={}", m_objectID, scale, opt.str()));

    return m_commandEngine->addCommand(
        std::make_unique<ScaleNode>(
            opt.afterId,
            m_objectID,
            opt.duration,
            opt.relative,
            scale));
}

int CommandAPI::lua_start(
    int _,
    const sol::table& lua_opt,
    sol::function fn) noexcept
{
    const auto opt = readOptions(lua_opt);

    {
        m_coroutines.emplace_back(new Coroutine(m_scriptEngine->getLua(), fn, m_coroutines.size()));
    }
    auto& coroutine = m_coroutines[m_coroutines.size() - 1];

    KI_INFO_OUT(fmt::format("start: node={}, opt={}, coroutine={}", m_objectID, opt.str(), coroutine->m_id));

    return m_commandEngine->addCommand(
        std::make_unique<StartNode>(
            opt.afterId,
            m_objectID,
            coroutine));    
}

int CommandAPI::lua_resume(
    int coroutineID,
    const sol::table& lua_opt,
    const std::string& callbackFn) noexcept
{
    const auto opt = readOptions(lua_opt);

    if (coroutineID < 0 && coroutineID >= m_coroutines.size()){
        KI_WARN_OUT(fmt::format("resume: Invalid coroutine - node={}, coroutineID={}, opt={}", m_objectID, coroutineID, opt.str()));
        return 0;
    }

    KI_INFO_OUT(fmt::format("resume: node={}, coroutineID={}, opt={}", m_objectID, coroutineID, opt.str()));

    auto& coroutine = m_coroutines[coroutineID];

    return m_commandEngine->addCommand(
        std::make_unique<ResumeNode>(
            opt.afterId,
            m_objectID,
            coroutine));
}
