#include "MoveSplineNode.h"

#include "model/Node.h"

#include "scene/RenderContext.h"


MoveSplineNode::MoveSplineNode(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative,
    const glm::vec3& controlPoint,
    const glm::vec3& position) noexcept
    : NodeCommand(afterCommandId, objectID, finishTime, relative),
    m_controlPoint(controlPoint),
    m_position(position)
{
}

void MoveSplineNode::bind(const RenderContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);

    m_end = m_position;
    if (!m_relative) {
        const auto& nodePosition = m_node->getPosition();
        m_end -= nodePosition;

        if (m_node->m_id == KI_UUID("65ce67c8-3efe-4b04-aaf9-fe384152c547"))
            KI_INFO_OUT(fmt::format(
                "BIND: pos=({}, {}, {}), target=({}, {}, {}), relative={}",
                nodePosition.x, nodePosition.y, nodePosition.z,
                m_end.x, m_end.y, m_end.z, m_relative));
    }
}

void MoveSplineNode::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_finishTime;

    const auto& nodePosition = m_node->getPosition();
    if (m_node->m_id == KI_UUID("65ce67c8-3efe-4b04-aaf9-fe384152c547"))
        KI_INFO_OUT(fmt::format("MOVE: {}, {}, {}", nodePosition.x, nodePosition.y, nodePosition.z));

    // NOTE KI keep steps relative to previous
    // => in case there is N concurrent commands
    glm::vec3 position;
    if (m_finished) {
        position = m_end;
    }
    else {
        const auto t = (m_elapsedTime / m_finishTime);

        glm::vec3 p0{ 0 };
        glm::vec3 p1{ m_controlPoint };
        glm::vec3 p2{ m_end };

        position = (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
    }

    auto adjust = position - m_previous;
    m_node->setPosition(nodePosition + adjust);
    m_previous = position;

    if (m_finished) {
        if (m_node->m_id == KI_UUID("65ce67c8-3efe-4b04-aaf9-fe384152c547"))
            const auto& nodePosition = m_node->getPosition();
            KI_INFO_OUT(fmt::format(
                "BIND: pos=({}, {}, {}), target=({}, {}, {}), relative={}",
                nodePosition.x, nodePosition.y, nodePosition.z,
                m_end.x, m_end.y, m_end.z, m_relative));
    }
}
