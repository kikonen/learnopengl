#include "StartNode.h"

#include <sol/sol.hpp>

#include <sol/sol.hpp>
#include "engine/UpdateContext.h"

#include "command/Coroutine.h"


StartNode::StartNode(
    ki::command_id afterCommandId,
    ki::object_id nodeId,
    Coroutine* coroutine) noexcept
    : NodeCommand(afterCommandId, nodeId, 0, false),
    m_coroutine(coroutine)
{
}

void StartNode::bind(const UpdateContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);
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
