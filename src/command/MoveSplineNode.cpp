#include "command/MoveSplineNode.h"


MoveSplineNode::MoveSplineNode(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float finishTime,
    bool relative,
    const glm::vec3& controlPoint,
    const glm::vec3& position)
    : NodeCommand(afterCommandId, objectID, initialDelay, finishTime, relative),
    m_controlPoint(controlPoint),
    m_end(position)
{
}

void MoveSplineNode::bind(const RenderContext& ctx, Node* node)
{
    NodeCommand::bind(ctx, node);
    m_begin = node->getPosition();
}

void MoveSplineNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;

    const auto t = (m_elapsedTime / m_finishTime);
    glm::vec3 p0{ 0.f };
    auto p1 = m_controlPoint;
    const auto p2 = m_end;

    if (!m_relative) {
        p0 += m_begin;
        p1 += m_begin;
    }

    glm::vec3 position = (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;

    m_finished = m_elapsedTime >= m_finishTime;

    if (m_finished) position = m_end;

    if (m_relative) {
        position += m_begin;
    }

    m_node->setPosition(position);
}
