#include "ViewportRenderer.h"

#include "scene/RenderContext.h"
#include "scene/FrameBuffer.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

ViewportRenderer::ViewportRenderer()
{
}

void ViewportRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);
}

void ViewportRenderer::update(const RenderContext& ctx)
{
    for (auto& viewport : ctx.m_registry->m_nodeRegistry->viewports) {
        viewport->update(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx,
    FrameBuffer* destinationBuffer)
{
    if (ctx.m_registry->m_nodeRegistry->viewports.empty()) return;

    ctx.state.disable(GL_DEPTH_TEST);
    ctx.state.enable(GL_BLEND);

    bool forceWireframe = ctx.m_forceWireframe;
    ctx.m_forceWireframe = false;
    ctx.bindDefaults();

    for (auto& viewport : ctx.m_registry->m_nodeRegistry->viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        viewport->draw(ctx);
        viewport->unbind(ctx);
    }

    ctx.state.disable(GL_BLEND);
    ctx.state.enable(GL_DEPTH_TEST);

    ctx.m_forceWireframe = forceWireframe;
    ctx.bindDefaults();
}
