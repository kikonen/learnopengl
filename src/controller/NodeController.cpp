#include "NodeController.h"

#include "model/Node.h"

#include "scene/RenderContext.h"

NodeController::NodeController()
{
}

void NodeController::prepare(
    const Assets& assets,
    EntityRegistry& entityRegistry,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;
}

bool NodeController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    return false;
}

void NodeController::setUpdateFrequency(float frequency)
{
    m_updateFrequency = frequency;
}

bool NodeController::needUpdate(const RenderContext& ctx)
{
    if (m_updateFrequency <= 0.f) return true;

    m_elapsedTime += ctx.m_clock.elapsedSecs;

    bool hit = m_elapsedTime >= m_updateFrequency
        && m_lastHitFrame + 1 != ctx.m_clock.frameCount;

    if (hit) {
        while (m_elapsedTime > 0) {
            m_elapsedTime -= m_updateFrequency;
        }
        if (m_elapsedTime < 0) {
            m_elapsedTime += m_updateFrequency;
        }

        m_lastHitFrame = ctx.m_clock.frameCount;
    }

    return hit;
}
