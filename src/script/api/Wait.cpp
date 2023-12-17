#include "Wait.h"

#include "engine/UpdateContext.h"

namespace script
{
    Wait::Wait(
        script::command_id afterCommandId,
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
}
