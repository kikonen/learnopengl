#include "command/ScaleNode.h"


ScaleNode::ScaleNode(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float finishTime,
    bool relative,
    const glm::vec3& scale)
    : NodeCommand(afterCommandId, objectID, initialDelay, finishTime, relative),
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
    glm::vec3 scale = m_relative
        ? (m_end) * (m_elapsedTime / m_finishTime)
        : (m_end - m_begin) * (m_elapsedTime / m_finishTime) + m_begin;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) scale = m_end;

    if (m_relative) {
        scale += m_begin;
    }

    m_node->setScale(scale);
}
