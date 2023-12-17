#include "InvokeluaFunction.h"

#include "engine/UpdateContext.h"

#include "script/ScriptEngine.h"

#include "registry/Registry.h"

namespace script
{
    InvokeLuaFunction::InvokeLuaFunction(
        script::command_id afterCommandId,
        ki::object_id nodeId,
        std::string_view functionName) noexcept
        : NodeCommand(afterCommandId, nodeId, 0, false),
        m_functionName(functionName)
    {
    }

    void InvokeLuaFunction::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            ctx.m_registry->m_scriptEngine->invokeFunction(m_node, m_functionName);
        }
    }
}
