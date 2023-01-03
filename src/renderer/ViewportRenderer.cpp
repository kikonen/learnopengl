#include "ViewportRenderer.h"

ViewportRenderer::ViewportRenderer()
{
}

void ViewportRenderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);
}

void ViewportRenderer::update(const RenderContext& ctx)
{
    for (auto& viewport : ctx.registry.viewports) {
        viewport->update(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx,
    FrameBuffer* destinationBuffer)
{
    if (ctx.registry.viewports.empty()) return;

    ctx.state.disable(GL_DEPTH_TEST);
    ctx.state.enable(GL_BLEND);

    bool useWireframe = ctx.m_useWireframe;
    ctx.m_useWireframe = false;
    ctx.bindDefaults();

    for (auto& viewport : ctx.registry.viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        viewport->draw(ctx);
        viewport->unbind(ctx);
    }

    ctx.state.disable(GL_BLEND);
    ctx.state.enable(GL_DEPTH_TEST);

    ctx.m_useWireframe = useWireframe;
    ctx.bindDefaults();
}
