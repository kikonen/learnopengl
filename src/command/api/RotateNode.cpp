#include "RotateNode.h"

#include "model/Node.h"

#include "scene/RenderContext.h"


RotateNode::RotateNode(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative,
    const glm::vec3& rotation) noexcept
    : NodeCommand(afterCommandId, objectID, finishTime, relative),
    m_rotation(rotation)
{
}

void RotateNode::bind(const RenderContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);

    m_end = m_rotation;
    if (!m_relative) {
        m_end -= m_node->getRotation();
    }
}

void RotateNode::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_finishTime;

    // NOTE KI keep steps relative to previous
    // => in case there is N concurrent commands
    glm::vec3 rotation;
    if (m_finished) {
        rotation = m_end;
    }
    else {
        const auto t = (m_elapsedTime / m_finishTime);

        glm::vec3 p0{ 0 };
        glm::vec3 p1{ m_end };

        rotation = (1 - t) * p0 + t * p1;
    }

    auto adjust = rotation - m_previous;
    m_node->adjustRotation(adjust);
    m_previous = rotation;
}
