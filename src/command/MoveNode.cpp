#include "command/MoveNode.h"


MoveNode::MoveNode(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float finishTime,
    bool relative,
    const glm::vec3& position)
    : NodeCommand(afterCommandId, objectID, initialDelay, finishTime, relative),
    m_end(position)
{
}

void MoveNode::bind(const RenderContext& ctx, Node* node)
{
    NodeCommand::bind(ctx, node);
    m_begin = node->getPosition();
}

void MoveNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;
    glm::vec3 position = m_relative
        ? (m_end) * (m_elapsedTime / m_finishTime)
        : (m_end - m_begin) * (m_elapsedTime / m_finishTime) + m_begin;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) position = m_end;

    if (m_relative) {
        position += m_begin;
    }

    m_node->setPosition(position);
}
