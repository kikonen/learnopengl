#include "command/RotateNode.h"


RotateNode::RotateNode(
    int objectID,
    float finishTime,
    const glm::vec3& rotation)
    : Command(objectID, finishTime),
    m_end(rotation)
{
}

void RotateNode::bind(const RenderContext& ctx, Node* node)
{
    Command::bind(ctx, node);
    m_begin = node->getRotation();
}

void RotateNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;
    glm::vec3 rotation = (m_end - m_begin) * (m_elapsedTime / m_finishTime) + m_begin;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) rotation = m_end;

    m_node->setRotation(rotation);
}
