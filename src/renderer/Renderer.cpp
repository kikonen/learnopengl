#include "Renderer.h"

#include "scene/RenderContext.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    m_renderFrameStart = assets.renderFrameStart;
    m_renderFrameStep = assets.renderFrameStep;
}

void Renderer::update(const RenderContext& ctx)
{
}

void Renderer::setClosest(Node* closest, int tagIndex)
{
    if (m_lastClosest == closest) return;
    if (m_lastClosest && tagIndex != -1) {
        m_lastClosest->setTagMaterialIndex(-1);
    }
    if (closest && tagIndex != -1) {
        closest->setTagMaterialIndex(tagIndex);
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
