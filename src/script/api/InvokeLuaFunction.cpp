#include "InvokeluaFunction.h"

#include "engine/UpdateContext.h"

#include "script/ScriptEngine.h"

#include "registry/Registry.h"

namespace script
{
    InvokeLuaFunction::InvokeLuaFunction(
        pool::NodeHandle handle,
        const sol::function& fn,
        sol::table args) noexcept
        : NodeCommand(handle, 0, false),
        m_fn{ fn },
        m_args{ args }
    {
    }

    void InvokeLuaFunction::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            script::ScriptEngine::get().invokeNodeFunction(getNode(), m_fn, m_args);
        }
    }
}
