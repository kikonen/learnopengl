#include "command/CancelCommand.h"

#include "command/CommandEngine.h"

CancelCommand::CancelCommand(
    int afterCommandId,
    float initialDelay,
    float finishTime,
    int commandId)
    : Command(afterCommandId, initialDelay, finishTime),
    m_commandId(commandId)
{
}

void CancelCommand::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) {
        ctx.commandEngine.cancel(m_commandId);
    }
}
