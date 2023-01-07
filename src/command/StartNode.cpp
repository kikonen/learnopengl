#include "command/StartNode.h"

#include "scene/RenderContext.h"


StartNode::StartNode(
    int afterCommandId,
    int objectID,
    std::unique_ptr<sol::coroutine> coroutine,
    sol::variadic_args vargs) noexcept
    : NodeCommand(afterCommandId, objectID, 0, false),
    m_coroutine(std::move(coroutine)),
    m_vargs(vargs)
{
}

void StartNode::bind(const RenderContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);
}

void StartNode::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) {
        // TODO KI start...
        (*m_coroutine)(m_vargs);
    }
}
