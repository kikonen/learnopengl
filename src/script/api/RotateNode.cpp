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

namespace script
{
    RotateNode::RotateNode(
        ki::node_id nodeId,
        float duration,
        bool relative,
        const glm::vec3& axis,
        const float degrees) noexcept
        : NodeCommand(nodeId, duration, relative),
        m_axis(glm::normalize(axis)),
        m_radians(glm::radians(degrees))
    {
    }

    void RotateNode::bind(const UpdateContext& ctx) noexcept
    {
        NodeCommand::bind(ctx);

        m_base = getNode()->getState().getQuatRotation();

        // NOTE KI relative means now "relative to axis of node"
        if (m_relative) {
            m_relativeAxis = glm::mat3(m_base) * m_axis;
        }
        else {
            m_relativeAxis = m_axis;
        }
    }

    void RotateNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;
        m_finished = m_elapsedTime >= m_duration;

        // NOTE KI trying to keep things relative to current context
        // i.e. allow multiple concurrent rotations
        {
            auto& state = getNode()->modifyState();
            const auto& base = state.getQuatRotation();
            const auto relativeAxis = m_relative ? glm::mat3(base) * m_axis : m_axis;

            const auto t = m_finished ? 1.f : (m_elapsedTime / m_duration);
            const auto radians = t * m_radians;

            const auto rot = util::axisRadiansToQuat(relativeAxis, radians - m_previousRadians);

            state.setQuatRotation(rot * base);

            m_previousRadians = radians;
        }
    }
}
