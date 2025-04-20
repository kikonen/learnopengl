#include "Cancel.h"

#include "script/CommandEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"

namespace script
{
    Cancel::Cancel(
        float duration,
        std::vector<script::command_id> commandIds) noexcept
        : Command(duration),
        m_commandIds(commandIds)
    {
    }

    void Cancel::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            for (auto commandId : m_commandIds) {
                script::CommandEngine::get().cancel(commandId);
            }
        }
    }
}
