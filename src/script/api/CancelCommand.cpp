#include "CancelCommand.h"

#include "script/CommandEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"

namespace script
{
    CancelCommand::CancelCommand(
        script::command_id afterCommandId,
        float duration,
        script::command_id commandId) noexcept
        : Command(afterCommandId, duration),
        m_commandId(commandId)
    {
    }

    void CancelCommand::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            ctx.m_registry->m_commandEngine->cancel(m_commandId);
        }
    }
}
