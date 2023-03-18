#include "CancelCommand.h"

#include "command/CommandEngine.h"

#include "engine/UpdateContext.h"

CancelCommand::CancelCommand(
    int afterCommandId,
    float duration,
    int commandId) noexcept
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
        ctx.m_commandEngine->cancel(m_commandId);
    }
}
