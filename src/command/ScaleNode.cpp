#include "command/ScaleNode.h"


ScaleNode::ScaleNode(
    int objectID,
    float initialDelay,
    float finishTime,
    const glm::vec3& scale)
    : NodeCommand(objectID, initialDelay, finishTime),
    m_end(scale)
{
}

void ScaleNode::bind(const RenderContext& ctx, Node* node)
{
    NodeCommand::bind(ctx, node);
    m_begin = node->getScale();
}

void ScaleNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;
    glm::vec3 scale = (m_end - m_begin) * (m_elapsedTime / m_finishTime) + m_begin;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) scale = m_end;

    m_node->setScale(scale);
}
