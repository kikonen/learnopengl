#include "RotateNode.h"

#include "util/glm_util.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"


RotateNode::RotateNode(
    ki::command_id afterCommandId,
    ki::object_id nodeId,
    float duration,
    bool relative,
    const glm::vec3& degrees) noexcept
    : NodeCommand(afterCommandId, nodeId, duration, relative),
    m_degreesRotation(degrees)
{
}

void RotateNode::bind(const UpdateContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);

    m_end = util::degreesToQuat(m_degreesRotation);
    if (!m_relative) {
        m_end -= m_node->getQuatRotation();
    }
}

void RotateNode::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_duration;

    // NOTE KI keep steps relative to previous
    // => in case there is N concurrent commands
    glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };
    if (m_finished) {
        rotation = m_end;
    }
    else {
        const auto t = (m_elapsedTime / m_duration);

        glm::quat p0{ 1.f, 0.f, 0.f, 0.f };
        glm::quat p1{ m_end };

        rotation = (1 - t) * p0 + t * p1;
    }

    auto adjust = rotation - m_previous;
    m_node->adjustQuatRotation(adjust);
    m_previous = rotation;
}
