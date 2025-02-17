#include "Renderer.h"

#include "asset/Assets.h"

#include "render/RenderContext.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"


Renderer::~Renderer()
{}

void Renderer::prepareRT(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.m_assets;

    m_registry = ctx.m_registry;

    m_renderFrameStart = assets.renderFrameStart;
    m_renderFrameStep = assets.renderFrameStep;
}

bool Renderer::setClosest(
    const RenderContext& ctx,
    Node* closest,
    int tagIndex)
{
    const bool changed = m_lastClosest != closest;
    if (changed) {
        if (m_lastClosest) {
            ctx.m_registry->m_selectionRegistry->untagNode(m_lastClosest->toHandle(), tagIndex);
        }
        if (closest) {
            ctx.m_registry->m_selectionRegistry->tagNode(closest->toHandle(), tagIndex);
        }
        m_lastClosest = closest;
    }
    return changed;
}

void Renderer::clearClosest(
    const RenderContext& ctx,
    int tagIndex)
{
    if (m_lastClosest) {
        ctx.m_registry->m_selectionRegistry->untagNode(m_lastClosest->toHandle(), tagIndex);
        m_lastClosest = nullptr;
    }
}

bool Renderer::needRender(const RenderContext& ctx)
{
    if (!m_enabled) return false;

    if (!m_useFrameStep) return true;
    if (m_renderFrameStep <= 0) return true;

    bool hit = ((ctx.m_clock.frameCount + m_renderFrameStart) % m_renderFrameStep) == 0;

    if (hit) {
        m_elapsedSecs = m_elapsedSecs <= -1.f ? 0.f : static_cast<float>(m_lastHitTime - ctx.m_clock.ts);
        m_lastHitTime = ctx.m_clock.ts;
        m_lastHitFrame = ctx.m_clock.frameCount;
    }

    return hit;
}
