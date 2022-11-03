#include "command/WaitCommand.h"

#include "command/CommandEngine.h"

WaitCommand::WaitCommand(
    int afterCommandId,
    float finishTime) noexcept
    : Command(afterCommandId, finishTime)
{
}

void WaitCommand::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_finishTime;
}
