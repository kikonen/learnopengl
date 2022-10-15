#include "command/CommandAPI.h"


#include "command/CommandEngine.h"

#include "command/CancelCommand.h"
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
    float initialDelay,
    float secs,
    int commandId)
{
    return m_commandEngine.addCommand(
        std::make_unique<CancelCommand>(
            afterCommandId,
            initialDelay,
            secs,
            commandId));
}

int CommandAPI::lua_moveTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float x, float y, float z)
{
    return m_commandEngine.addCommand(
        std::make_unique<MoveNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ x, y, z }));
}

int CommandAPI::lua_moveSplineTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float px, float py, float pz,
    float x, float y, float z)
{
    return m_commandEngine.addCommand(
        std::make_unique<MoveSplineNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ px, py, pz },
            glm::vec3{ x, y, z }));
}

int CommandAPI::lua_rotateTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float x, float y, float z)
{
    return m_commandEngine.addCommand(
        std::make_unique<RotateNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ x, y, z }));
}

int CommandAPI::lua_scaleTo(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float secs,
    bool relative,
    float x, float y, float z)
{
    return m_commandEngine.addCommand(
        std::make_unique<ScaleNode>(
            afterCommandId,
            objectID,
            initialDelay,
            secs,
            relative,
            glm::vec3{ x, y, z }));
}

int CommandAPI::lua_start(
    int afterCommandId,
    int objectID,
    float initialDelay,
    sol::function fn,
    sol::variadic_args va)
{
    auto task = std::make_unique<sol::coroutine>(m_runner.state(), fn);
    //auto r = (*task)(va);

    return m_commandEngine.addCommand(
        std::make_unique<StartNode>(
            afterCommandId,
            objectID,
            initialDelay,
            std::move(task),
            va));
}
