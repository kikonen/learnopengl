#include "Wait.h"

#include "command/CommandEngine.h"

#include "engine/UpdateContext.h"

Wait::Wait(
    int afterCommandId,
    float finishTime) noexcept
    : Command(afterCommandId, finishTime)
{
}

void Wait::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_finishTime;
}
