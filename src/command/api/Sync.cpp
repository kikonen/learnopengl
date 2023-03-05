#include "Sync.h"

#include "command/CommandEngine.h"

Sync::Sync(
    int afterCommandId,
    float finishTime,
    std::vector<int> commandIds) noexcept
    : Command(afterCommandId, finishTime),
    m_commandIds(commandIds)
{
}

void Sync::execute(
    const RenderContext& ctx) noexcept
{
    if (!m_syncedAll) {
        int count = 0;
        for (const auto& commandId : m_commandIds) {
            if (commandId == m_id) {
                count++;
            }
            else {
                if (!ctx.m_commandEngine->isAlive(commandId)) count++;
            }
        }
        m_syncedAll = m_commandIds.size() == count;
    }
    else {
        m_elapsedTime += ctx.m_clock.elapsedSecs;
        m_finished = m_elapsedTime >= m_finishTime;
    }
}
