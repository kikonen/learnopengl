#include "ViewportRenderer.h"

ViewportRenderer::ViewportRenderer()
{
}

void ViewportRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);
}

void ViewportRenderer::update(const RenderContext& ctx)
{
    for (auto& viewport : ctx.registry.viewports) {
        viewport->update(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx)
{
    if (ctx.registry.viewports.empty()) return;

    ctx.state.disable(GL_DEPTH_TEST);
    ctx.state.enable(GL_BLEND);

    bool useWireframe = ctx.m_useWireframe;
    ctx.m_useWireframe = false;
    ctx.bindGlobal();

    for (auto& viewport : ctx.registry.viewports) {
        viewport->bind(ctx);
        viewport->draw(ctx);
        viewport->unbind(ctx);
    }

    ctx.state.disable(GL_BLEND);
    ctx.state.enable(GL_DEPTH_TEST);

    ctx.m_useWireframe = useWireframe;
    ctx.bindGlobal();
}
