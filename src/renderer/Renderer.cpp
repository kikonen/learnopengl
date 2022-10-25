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

void Renderer::bind(const RenderContext& ctx)
{
}

bool Renderer::needRender(const RenderContext& ctx)
{
    if (m_renderFrequency <= 0.f) return true;

    m_elapsedTime += ctx.clock.elapsedSecs;
    bool hit = m_elapsedTime >= m_renderFrequency;
    if (hit) {
        while (m_elapsedTime > 0) {
            m_elapsedTime -= m_renderFrequency;
        }
        if (m_elapsedTime < 0) {
            m_elapsedTime += m_renderFrequency;
        }
    }
    return hit;
}
