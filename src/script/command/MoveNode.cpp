#include "MoveNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

namespace script
{
    MoveNode::MoveNode(
        pool::NodeHandle handle,
        float duration,
        bool relative,
        const glm::vec3& position) noexcept
        : NodeCommand(handle, duration, relative),
        m_position(position)
    {}

    void MoveNode::bind(const UpdateContext& ctx) noexcept
    {
        NodeCommand::bind(ctx);

        m_end = m_position;
        if (!m_relative) {
            auto* node = getNode();
            if (!node) return;

            const auto& nodePosition = node->getState().getPosition();
            m_end -= nodePosition;
        }
    }

    void MoveNode::execute(
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

            glm::vec3 p0{ 0 };
            glm::vec3 p1{ m_end };

            position = (1 - t) * p0 + t * p1;
        }

        auto adjust = position - m_previous;

        auto* node = getNode();
        if (!node) return;

        node->modifyState().adjustPosition(adjust);
        node->updateModelMatrix();

        m_previous = position;
    }
}
