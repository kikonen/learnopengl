#include "command/CommandAPI.h"


#include "command/CommandEngine.h"

#include "command/CancelCommand.h"
#include "command/WaitCommand.h"
#include "command/MoveNode.h"
#include "command/MoveSplineNode.h"
#include "command/RotateNode.h"
#include "command/ScaleNode.h"
#include "command/StartNode.h"


namespace {
    struct CommandOptions {
        int afterId = 0;
        int time = 0;
        bool relative = false;
        bool repeat = false;
    };

    CommandOptions readOptions(const sol::table& lua_opt) {
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

    glm::vec3 readVec3(const sol::table& v) {
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
    int commandId)
{
    return m_commandEngine.addCommand(
        std::make_unique<CancelCommand>(
            afterCommandId,
            secs,
            commandId));
}

int CommandAPI::lua_wait(
    int afterCommandId,
    float secs)
{
    return m_commandEngine.addCommand(
        std::make_unique<WaitCommand>(
            afterCommandId,
            secs));
}

int CommandAPI::lua_move(
    int objectID,
    const sol::table& lua_opt,
    const sol::table& lua_pos)
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
    const sol::table& lua_pos)
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
    const sol::table& lua_rot)
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
    const sol::table& lua_scale)
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
    sol::variadic_args va)
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
