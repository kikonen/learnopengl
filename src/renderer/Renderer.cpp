#include "Renderer.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"


void Renderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    m_registry = registry;

    m_renderFrameStart = assets.renderFrameStart;
    m_renderFrameStep = assets.renderFrameStep;
}

bool Renderer::setClosest(Node* closest, int tagIndex)
{
    const bool changed = m_lastClosest != closest;
    if (tagIndex != -1) {
        if (m_lastClosest) {
            m_lastClosest->setTagMaterialIndex(-1);
        }
        if (closest) {
            closest->setTagMaterialIndex(tagIndex);
        }
    }
    m_lastClosest = closest;
    return changed;
}

bool Renderer::needRender(const RenderContext& ctx)
{
    if (m_renderFrameStep <= 0) return true;

    bool hit = ((ctx.m_clock.frameCount + m_renderFrameStart) % m_renderFrameStep) == 0;

    if (hit) {
        m_elapsedSecs = m_elapsedSecs == -1 ? 0 : m_lastHitTime - ctx.m_clock.ts;
        m_lastHitTime = ctx.m_clock.ts;
        m_lastHitFrame = ctx.m_clock.frameCount;
    }

    return hit;
}
