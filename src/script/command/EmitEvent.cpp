#include "EmitEvent.h"

#include "engine/UpdateContext.h"

#include "script/ScriptSystem.h"

#include "registry/Registry.h"

namespace script
{
    EmitEvent::EmitEvent(
        pool::NodeHandle handle,
        int listenerId,
        int type,
        const std::string& data) noexcept
        : NodeCommand(handle, 0, false),
        m_listenerId{ listenerId },
        m_type{ type },
        m_data{ data }
    {
    }

    void EmitEvent::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            script::ScriptSystem::get().emitEvent(m_listenerId, m_type, m_data);
        }
    }
}
