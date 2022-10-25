#include "command/CommandAPI.h"


#include "command/CommandEngine.h"

#include "command/CancelCommand.h"
#include "command/WaitCommand.h"
#include "command/MoveNode.h"
#include "command/MoveSplineNode.h"
#include "command/RotateNode.h"
#include "command/ScaleNode.h"
#include "command/StartNode.h"


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

int CommandAPI::lua_moveTo(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_pos)
{
    auto pos = glm::vec3{ lua_pos.get<float>(1) , lua_pos.get<float>(2) , lua_pos.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<MoveNode>(
            afterCommandId,
            objectID,
            secs,
            false,
            pos));
}

int CommandAPI::lua_moveRelative(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_pos)
{
    auto pos = glm::vec3{ lua_pos.get<float>(1) , lua_pos.get<float>(2) , lua_pos.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<MoveNode>(
            afterCommandId,
            objectID,
            secs,
            true,
            pos));
}

int CommandAPI::lua_moveSplineTo(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_p,
    sol::table lua_pos)
{
    auto p = glm::vec3{ lua_p.get<float>(1) , lua_p.get<float>(2) , lua_p.get<float>(3) };
    auto pos = glm::vec3{ lua_pos.get<float>(1) , lua_pos.get<float>(2) , lua_pos.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<MoveSplineNode>(
            afterCommandId,
            objectID,
            secs,
            false,
            p,
            pos));
}

int CommandAPI::lua_moveSplineRelative(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_p,
    sol::table lua_pos)
{
    auto p = glm::vec3{ lua_p.get<float>(1) , lua_p.get<float>(2) , lua_p.get<float>(3) };
    auto pos = glm::vec3{ lua_pos.get<float>(1) , lua_pos.get<float>(2) , lua_pos.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<MoveSplineNode>(
            afterCommandId,
            objectID,
            secs,
            true,
            p,
            pos));
}

int CommandAPI::lua_rotateTo(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_rot)
{
    auto rot = glm::vec3{ lua_rot.get<float>(1) , lua_rot.get<float>(2) , lua_rot.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<RotateNode>(
            afterCommandId,
            objectID,
            secs,
            false,
            rot));
}

int CommandAPI::lua_rotateRelative(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_rot)
{
    auto rot = glm::vec3{ lua_rot.get<float>(1) , lua_rot.get<float>(2) , lua_rot.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<RotateNode>(
            afterCommandId,
            objectID,
            secs,
            true,
            rot));
}

int CommandAPI::lua_scaleTo(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_scale)
{
    auto scale = glm::vec3{ lua_scale.get<float>(1) , lua_scale.get<float>(2) , lua_scale.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<ScaleNode>(
            afterCommandId,
            objectID,
            secs,
            false,
            scale));
}

int CommandAPI::lua_scaleRelative(
    int afterCommandId,
    int objectID,
    float secs,
    sol::table lua_scale)
{
    auto scale = glm::vec3{ lua_scale.get<float>(1) , lua_scale.get<float>(2) , lua_scale.get<float>(3) };

    return m_commandEngine.addCommand(
        std::make_unique<ScaleNode>(
            afterCommandId,
            objectID,
            secs,
            true,
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
