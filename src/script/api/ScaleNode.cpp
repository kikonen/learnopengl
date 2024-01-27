#include "ScaleNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

namespace script
{
    ScaleNode::ScaleNode(
        ki::node_id nodeId,
        float duration,
        bool relative,
        const glm::vec3& scale) noexcept
        : NodeCommand(nodeId, duration, relative),
        m_scale(scale)
    {
    }

    void ScaleNode::bind(const UpdateContext& ctx) noexcept
    {
        NodeCommand::bind(ctx);

        m_end = m_scale;
        if (!m_relative) {
            m_end -= getNode()->getTransform().getScale();
        }
    }

    void ScaleNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;
        m_finished = m_elapsedTime >= m_duration;

        // NOTE KI keep steps relative to previous
        // => in case there is N concurrent commands
        glm::vec3 scale;
        if (m_finished) {
            scale = m_end;
        }
        else {
            const auto t = (m_elapsedTime / m_duration);

            glm::vec3 p0{ 0 };
            glm::vec3 p1{ m_end };

            scale = (1 - t) * p0 + t * p1;
        }

        auto adjust = scale - m_previous;
        getNode()->modifyTransform().adjustScale(adjust);
        m_previous = scale;
    }
}
