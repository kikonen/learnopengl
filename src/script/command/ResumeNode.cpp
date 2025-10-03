#include "ResumeNode.h"

#include "engine/UpdateContext.h"

#include "script/Coroutine.h"

namespace script
{
    ResumeNode::ResumeNode(
        pool::NodeHandle handle,
        Coroutine* coroutine) noexcept
        : NodeCommand(handle, 0, false),
        m_coroutine(coroutine)
    {
    }

    void ResumeNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.getClock().elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            (*(m_coroutine->m_coroutine))(m_id);
        }
    }
}
