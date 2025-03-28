#include "StartNode.h"

#include "engine/UpdateContext.h"

#include "script/lua_binding.h"
#include "script/Coroutine.h"

namespace script
{
    StartNode::StartNode(
        pool::NodeHandle handle,
        Coroutine* coroutine) noexcept
        : NodeCommand(handle, 0, false),
        m_coroutine(coroutine)
    {
    }

    void StartNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            // NOTE KI pass unique coroutine ID to allow multiple coroutines per node
            (*(m_coroutine->m_coroutine))(m_coroutine->m_id);
        }
    }
}
