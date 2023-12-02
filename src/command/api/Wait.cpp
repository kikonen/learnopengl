#include "Wait.h"

#include "command/CommandEngine.h"

#include "engine/UpdateContext.h"

Wait::Wait(
    ki::command_id afterCommandId,
    float duration) noexcept
    : Command(afterCommandId, duration)
{
}

void Wait::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_duration;
}
