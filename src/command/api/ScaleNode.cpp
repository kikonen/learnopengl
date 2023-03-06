#include "ScaleNode.h"

#include "model/Node.h"

#include "scene/RenderContext.h"


ScaleNode::ScaleNode(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative,
    const glm::vec3& scale) noexcept
    : NodeCommand(afterCommandId, objectID, finishTime, relative),
    m_scale(scale)
{
}

void ScaleNode::bind(const RenderContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);

    m_end = m_scale;
    if (!m_relative) {
        m_end -= m_node->getScale();
    }
}

void ScaleNode::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;
    m_finished = m_elapsedTime >= m_finishTime;

    const auto& nodescale = m_node->getScale();

    // NOTE KI keep steps relative to previous
    // => in case there is N concurrent commands
    glm::vec3 scale;
    if (m_finished) {
        scale = m_end;
    }
    else {
        const auto t = (m_elapsedTime / m_finishTime);

        glm::vec3 p0{ 0 };
        glm::vec3 p1{ m_end };

        scale = (1 - t) * p0 + t * p1;
    }

    auto adjust = scale - m_previous;
    m_node->setScale(nodescale + adjust);
    m_previous = scale;
}
