#include "Cancel.h"

#include "script/CommandEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"

namespace script
{
    Cancel::Cancel(
        float duration,
        script::command_id commandId) noexcept
        : Command(duration),
        m_commandId(commandId)
    {
    }

    void Cancel::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.getClock().elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            script::CommandEngine::get().cancel(m_commandId);
        }
    }
}
