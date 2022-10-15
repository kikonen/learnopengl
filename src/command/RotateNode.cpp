#include "command/RotateNode.h"


RotateNode::RotateNode(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float finishTime,
    bool relative,
    const glm::vec3& rotation)
    : NodeCommand(afterCommandId, objectID, initialDelay, finishTime, relative),
    m_end(rotation)
{
}

void RotateNode::bind(const RenderContext& ctx, Node* node)
{
    NodeCommand::bind(ctx, node);
    m_begin = node->getRotation();
}

void RotateNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;

    const auto t = (m_elapsedTime / m_finishTime);
    glm::vec3 p0{ 0.f };
    const auto p1 = m_end;

    if (!m_relative) {
        p0 += m_begin;
    }

    glm::vec3 rotation = (1 - t) * p0 + t * p1;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) rotation = m_end;

    if (m_relative) {
        rotation += m_begin;
    }

    m_node->setRotation(rotation);
}
