#include "command/CommandAPI.h"


#include "command/CommandEngine.h"

#include "api/CancelCommand.h"
#include "api/Wait.h"
#include "api/MoveNode.h"
#include "api/MoveSplineNode.h"
#include "api/RotateNode.h"
#include "api/ScaleNode.h"
#include "api/StartNode.h"


namespace {
    struct CommandOptions {
        int afterId = 0;
        int time = 0;
        bool relative = false;
        bool repeat = false;
    };

    CommandOptions readOptions(const sol::table& lua_opt) noexcept {
        CommandOptions opt;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "after") {
                opt.afterId = value.as<int>();
            }
            else if (k == "time") {
                opt.time = value.as<int>();
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
}


CommandAPI::CommandAPI(
    CommandEngine& commandEngine,
    sol::thread& runner)
  : m_commandEngine(commandEngine),
    m_runner(runner)
{}

int CommandAPI::lua_cancel(
    int afterCommandId,
    float secs,
    int commandId) noexcept
{
    return m_commandEngine.addCommand(
        std::make_unique<CancelCommand>(
            afterCommandId,
            secs,
            commandId));
}

int CommandAPI::lua_wait(
    int afterCommandId,
    float secs) noexcept
{
    return m_commandEngine.addCommand(
        std::make_unique<Wait>(
            afterCommandId,
            secs));
}

int CommandAPI::lua_move(
    int objectID,
    const sol::table& lua_opt,
    const sol::table& lua_pos) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto pos = readVec3(lua_pos);

    return m_commandEngine.addCommand(
        std::make_unique<MoveNode>(
            opt.afterId,
            objectID,
            opt.time,
            opt.relative,
            pos));
}

int CommandAPI::lua_moveSpline(
    int objectID,
    const sol::table& lua_opt,
    const sol::table& lua_p,
    const sol::table& lua_pos) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto p = readVec3(lua_p);
    const auto pos = readVec3(lua_pos);

    return m_commandEngine.addCommand(
        std::make_unique<MoveSplineNode>(
            opt.afterId,
            objectID,
            opt.time,
            opt.relative,
            p,
            pos));
}

int CommandAPI::lua_rotate(
    int objectID,
    const sol::table& lua_opt,
    const sol::table& lua_rot) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto rot = readVec3(lua_rot);

    return m_commandEngine.addCommand(
        std::make_unique<RotateNode>(
            opt.afterId,
            objectID,
            opt.time,
            opt.relative,
            rot));
}

int CommandAPI::lua_scale(
    int objectID,
    const sol::table& lua_opt,
    const sol::table& lua_scale) noexcept
{
    const auto opt = readOptions(lua_opt);
    const auto scale = readVec3(lua_scale);

    return m_commandEngine.addCommand(
        std::make_unique<ScaleNode>(
            opt.afterId,
            objectID,
            opt.time,
            opt.relative,
            scale));
}

int CommandAPI::lua_start(
    int afterCommandId,
    int objectID,
    sol::function fn,
    sol::variadic_args va) noexcept
{
    auto task = std::make_unique<sol::coroutine>(m_runner.state(), fn);
    //auto r = (*task)(va);

    return m_commandEngine.addCommand(
        std::make_unique<StartNode>(
            afterCommandId,
            objectID,
            std::move(task),
            va));
}
