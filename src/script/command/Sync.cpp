#include "Sync.h"

#include "engine/UpdateContext.h"

#include "script/CommandEngine.h"

#include "registry/Registry.h"

namespace script
{
    Sync::Sync(
        float duration,
        const std::vector<script::command_id>& commandIds) noexcept
        : Command(duration),
        m_commandIds(commandIds)
    {
    }

    void Sync::execute(
        const UpdateContext& ctx) noexcept
    {
        if (!m_syncedAll) {
            int count = 0;
            for (const auto& commandId : m_commandIds) {
                if (commandId == m_id) {
                    count++;
                }
                else {
                    if (!script::CommandEngine::get().isAlive(commandId)) count++;
                }
            }
            m_syncedAll = m_commandIds.size() == count;
        }
        else {
            m_elapsedTime += ctx.getClock().elapsedSecs;
            m_finished = m_elapsedTime >= m_duration;
        }
    }
}
