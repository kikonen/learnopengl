#include "InvokeFunction.h"

#include "engine/UpdateContext.h"

#include "script/ScriptSystem.h"

#include "registry/Registry.h"

namespace script
{
    InvokeFunction::InvokeFunction(
        pool::NodeHandle handle,
        bool self,
        const sol::function& fn,
        sol::table args) noexcept
        : NodeCommand(handle, 0, false),
        m_self{ self },
        m_fn{ fn },
        m_args{ args }
    {
    }

    void InvokeFunction::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            auto* node = getNode();
            if (!node) return;

            script::ScriptSystem::get().invokeNodeFunction(node, m_self, m_fn, m_args);
        }
    }
}
