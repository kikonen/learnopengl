#include "command/CancelCommand.h"

#include "command/CommandEngine.h"

CancelCommand::CancelCommand(
    int afterCommandId,
    float finishTime,
    int commandId) noexcept
    : Command(afterCommandId, finishTime),
    m_commandId(commandId)
{
}

void CancelCommand::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) {
        ctx.commandEngine.cancel(m_commandId);
    }
}
