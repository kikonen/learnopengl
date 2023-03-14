#include "MoveNode.h"

#include "model/Node.h"

#include "scene/UpdateContext.h"


MoveNode::MoveNode(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative,
    const glm::vec3& position) noexcept
    : NodeCommand(afterCommandId, objectID, finishTime, relative),
    m_position(position)
{
}

void MoveNode::bind(const UpdateContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);

    m_end = m_position;
    if (!m_relative) {
        const auto& nodePosition = m_node->getPosition();
        m_end -= nodePosition;
    }
}

void MoveNode::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_finishTime;

    // NOTE KI keep steps relative to previous
    // => in case there is N concurrent commands
    glm::vec3 position;
    if (m_finished) {
        position = m_end;
    }
    else {
        const auto t = (m_elapsedTime / m_finishTime);

        glm::vec3 p0{ 0 };
        glm::vec3 p1{ m_end };

        position = (1 - t) * p0 + t * p1;
    }

    auto adjust = position - m_previous;
    m_node->adjustPosition(adjust);
    m_previous = position;
}
