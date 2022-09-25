#include "ViewportRenderer.h"

ViewportRenderer::ViewportRenderer()
{
}

void ViewportRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    Renderer::prepare(assets, shaders);
}

void ViewportRenderer::bind(const RenderContext& ctx)
{
}

void ViewportRenderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
    for (auto& viewport : registry.viewports) {
        viewport->update(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    if (registry.viewports.empty()) return;

    ctx.state.disable(GL_DEPTH_TEST);
    ctx.state.enable(GL_BLEND);

    for (auto& viewport : registry.viewports) {
        viewport->bind(ctx);
        viewport->draw(ctx);
        viewport->unbind(ctx);
    }
    glBindVertexArray(0);

    ctx.state.disable(GL_BLEND);
    ctx.state.enable(GL_DEPTH_TEST);
}
