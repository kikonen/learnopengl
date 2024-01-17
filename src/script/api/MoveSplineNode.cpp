#include "MoveSplineNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

namespace script
{
    MoveSplineNode::MoveSplineNode(
        script::command_id afterCommandId,
        ki::node_id nodeId,
        float duration,
        bool relative,
        const glm::vec3& controlPoint,
        const glm::vec3& position) noexcept
        : NodeCommand(afterCommandId, nodeId, duration, relative),
        m_controlPoint(controlPoint),
        m_position(position)
    {
    }

    void MoveSplineNode::bind(const UpdateContext& ctx, Node* node) noexcept
    {
        NodeCommand::bind(ctx, node);

        m_end = m_position;
        if (!m_relative) {
            const auto& nodePosition = m_node->getTransform().getPosition();
            m_end -= nodePosition;
        }
    }

    void MoveSplineNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;
        m_finished = m_elapsedTime >= m_duration;

        // NOTE KI keep steps relative to previous
        // => in case there is N concurrent commands
        glm::vec3 position;
        if (m_finished) {
            position = m_end;
        }
        else {
            const auto t = (m_elapsedTime / m_duration);

            glm::vec3 p0{ 0.f };
            glm::vec3 p1{ m_controlPoint };
            glm::vec3 p2{ m_end };

            position = (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
        }

        auto adjust = position - m_previous;
        m_node->modifyTransform().adjustPosition(adjust);
        m_previous = position;
    }
}
