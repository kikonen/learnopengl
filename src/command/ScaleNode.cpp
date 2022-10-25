#include "command/ScaleNode.h"


ScaleNode::ScaleNode(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative,
    const glm::vec3& scale)
    : NodeCommand(afterCommandId, objectID, finishTime, relative),
    m_end(scale)
{
}

void ScaleNode::bind(const RenderContext& ctx, Node* node)
{
    NodeCommand::bind(ctx, node);
    m_begin = node->getScale();
}

void ScaleNode::execute(
    const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;

    const auto t = (m_elapsedTime / m_finishTime);
    glm::vec3 p0{ 0.f };
    const auto p1 = m_end;

    if (!m_relative) {
        p0 += m_begin;
    }

    glm::vec3 scale = (1 - t) * p0 + t * p1;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) scale = m_end;

    if (m_relative) {
        scale += m_begin;
    }

    m_node->setScale(scale);
}
