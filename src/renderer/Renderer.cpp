#include "Renderer.h"


Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    m_renderFrequency = assets.renderFrequency;
}

void Renderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
}

bool Renderer::needRender(const RenderContext& ctx)
{
    if (m_renderFrequency <= 0.f) return true;

    m_elapsedTime += ctx.m_clock.elapsedSecs;

    bool hit = m_elapsedTime >= m_renderFrequency
        && m_lastHitFrame + 1 != ctx.m_clock.frameCount;

    if (hit) {
        while (m_elapsedTime > 0) {
            m_elapsedTime -= m_renderFrequency;
        }
        if (m_elapsedTime < 0) {
            m_elapsedTime += m_renderFrequency;
        }

        m_lastHitFrame = ctx.m_clock.frameCount;
    }

    return hit;
}
