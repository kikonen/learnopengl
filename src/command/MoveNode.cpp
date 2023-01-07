#include "command/MoveNode.h"

#include "scene/RenderContext.h"

MoveNode::MoveNode(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative,
    const glm::vec3& position) noexcept
    : NodeCommand(afterCommandId, objectID, finishTime, relative),
    m_end(position)
{
}

void MoveNode::bind(const RenderContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);
    m_begin = node->getPosition();
}

void MoveNode::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;

    const auto t = (m_elapsedTime / m_finishTime);
    glm::vec3 p0{ 0.f };
    const auto p1 = m_end;

    if (!m_relative) {
        p0 += m_begin;
    }

    glm::vec3 position = (1 - t) * p0 + t * p1;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) position = m_end;

    if (m_relative) {
        position += m_begin;
    }

    m_node->setPosition(position);
}
