#include "ScaleNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

namespace script
{
    ScaleNode::ScaleNode(
        script::command_id afterCommandId,
        ki::object_id nodeId,
        float duration,
        bool relative,
        const glm::vec3& scale) noexcept
        : NodeCommand(afterCommandId, nodeId, duration, relative),
        m_scale(scale)
    {
    }

    void ScaleNode::bind(const UpdateContext& ctx, Node* node) noexcept
    {
        NodeCommand::bind(ctx, node);

        m_end = m_scale;
        if (!m_relative) {
            m_end -= m_node->getScale();
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
        m_node->adjustScale(adjust);
        m_previous = scale;
    }
}
