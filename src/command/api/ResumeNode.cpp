#include "ResumeNode.h"

#include "engine/UpdateContext.h"

#include "command/Coroutine.h"


ResumeNode::ResumeNode(
    int afterCommandId,
    int objectID,
    Coroutine* coroutine) noexcept
    : NodeCommand(afterCommandId, objectID, 0, false),
    m_coroutine(coroutine)
{
}

void ResumeNode::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_duration;
    if (m_finished) {
        (*(m_coroutine->m_coroutine))(m_id);
    }
}
