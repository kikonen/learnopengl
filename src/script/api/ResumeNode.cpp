#include "ResumeNode.h"

#include "engine/UpdateContext.h"

#include "script/Coroutine.h"

namespace script
{
    ResumeNode::ResumeNode(
        ki::node_id nodeId,
        Coroutine* coroutine) noexcept
        : NodeCommand(nodeId, 0, false),
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
}
