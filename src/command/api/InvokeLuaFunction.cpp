#include "InvokeluaFunction.h"

#include "engine/UpdateContext.h"

#include "command/ScriptEngine.h"


InvokeLuaFunction::InvokeLuaFunction(
    int afterCommandId,
    int objectID,
    std::string_view functionName) noexcept
    : NodeCommand(afterCommandId, objectID, 0, false),
    m_functionName(functionName)
{
}

void InvokeLuaFunction::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_duration;
    if (m_finished) {
        ctx.m_scriptEngine->invokeFunction(m_node, m_functionName);
    }
}
