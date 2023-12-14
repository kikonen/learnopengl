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

    m_start = m_node->getQuatRotation();

    const auto quat = util::degreesToQuat(m_degreesRotation);
    if (m_relative) {
        m_end = quat * m_start;
    }
    else {
        m_end = quat;
    }
}

void RotateNode::execute(
    const UpdateContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_duration;

    // NOTE KI keep steps relative to previous
    // => in case there is N concurrent commands
    // TODO KI needd to fix "relative" logic for quat
    // TODO KI rotating more tan 180 degrees with quat
    {
        const auto t = m_finished ? 1.f : (m_elapsedTime / m_duration);

        glm::quat p0{ m_start };
        glm::quat p1{ m_end };

        const auto rot = glm::normalize(glm::lerp(p0, p1, t));
        m_node->setQuatRotation(rot);
    }
}
