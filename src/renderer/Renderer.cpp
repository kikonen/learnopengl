#include "Renderer.h"

#include "scene/RenderContext.h"

#include "registry/Registry.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    m_registry = registry;

    m_renderFrameStart = assets.renderFrameStart;
    m_renderFrameStep = assets.renderFrameStep;
}

void Renderer::update(const RenderContext& ctx)
{
}

void Renderer::setClosest(Node* closest, int tagIndex)
{
    if (tagIndex != -1) {
        if (m_lastClosest) {
            m_lastClosest->setTagMaterialIndex(-1);
        }
        if (closest) {
            closest->setTagMaterialIndex(tagIndex);
        }
    }
    m_lastClosest = closest;
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
