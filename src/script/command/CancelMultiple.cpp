#include "CancelMultiple.h"

#include "ki/RenderClock.h"

#include "script/CommandEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"

namespace script
{
    CancelMultiple::CancelMultiple (
        float duration,
        std::vector<script::command_id> commandIds) noexcept
        : Command(duration),
        m_commandIds(commandIds)
    {
    }

    void CancelMultiple::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.getClock().elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            for (auto commandId : m_commandIds) {
                script::CommandEngine::get().cancel(commandId);
            }
        }
    }
}
