#include "command/StartNode.h"


StartNode::StartNode(
    int afterCommandId,
    int objectID,
    float initialDelay,
    sol::function& fn)
    : NodeCommand(afterCommandId, objectID, initialDelay, 0, false),
    m_fn(fn)
{
}

void StartNode::bind(const RenderContext& ctx, Node* node)
{
    NodeCommand::bind(ctx, node);
}

void StartNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) {
        // TODO KI start...
    }
}
