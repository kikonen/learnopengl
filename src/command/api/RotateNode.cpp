#include "RotateNode.h"

#include <fmt/format.h>

#include "util/glm_util.h"
#include "util/Log.h"
#include "util/glm_format.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

namespace {
    const glm::vec3 UP{ 0, 1, 0 };

    // https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
    glm::quat toQuat(const glm::vec3& v1, const glm::vec3& v2)
    {
        const glm::vec3 a = glm::cross(v1, v2);
        return glm::normalize(glm::quat{ glm::dot(v1, v2), a.x, a.y, a.z });
    }
}

RotateNode::RotateNode(
    ki::command_id afterCommandId,
    ki::object_id nodeId,
    float duration,
    bool relative,
    const glm::vec3& axis,
    const float degrees) noexcept
    : NodeCommand(afterCommandId, nodeId, duration, relative),
    m_axis(glm::normalize(axis)),
    m_degrees(degrees)
{
}

void RotateNode::bind(const UpdateContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);

    m_original = glm::normalize(m_node->getQuatRotation());

    m_start = util::axisDegreesToQuat(m_axis, 0);
    m_end = util::axisDegreesToQuat(m_axis, m_degrees);

    KI_INFO_OUT(fmt::format("orig_deg={} orig={}, start={}, end={}", m_node->getDegreesRotation(), m_original, m_start, m_end));
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

        const auto& p0 = m_start;
        const auto& p1 = m_end;

        const auto rot = glm::normalize(glm::slerp(p1, p0, t));
        m_node->setQuatRotation(rot * m_original);
        //m_node->adjustQuatRotation(rot);
    }
}
