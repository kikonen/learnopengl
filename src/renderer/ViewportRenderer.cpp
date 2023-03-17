#include "ViewportRenderer.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/FrameBuffer.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


void ViewportRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);
}

void ViewportRenderer::update(const UpdateContext& ctx)
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

    ctx.m_state.disable(GL_DEPTH_TEST);
    ctx.m_state.enable(GL_BLEND);

    bool forceWireframe = ctx.m_forceWireframe;
    ctx.m_forceWireframe = false;
    ctx.bindDefaults();

    for (auto& viewport : ctx.m_registry->m_nodeRegistry->viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        viewport->draw(ctx);
        viewport->unbind(ctx);
    }

    ctx.m_state.disable(GL_BLEND);
    ctx.m_state.enable(GL_DEPTH_TEST);

    ctx.m_forceWireframe = forceWireframe;
    ctx.bindDefaults();
}
