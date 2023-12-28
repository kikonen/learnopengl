#include "ViewportRenderer.h"

#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"

#include "render/FrameBuffer.h"

#include "registry/Registry.h"
#include "registry/ViewportRegistry.h"


void ViewportRenderer::prepareRT(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(assets, registry);
}

void ViewportRenderer::updateRT(const UpdateViewContext& ctx)
{
    for (auto& viewport : ctx.m_registry->m_viewportRegistry->getViewports()) {
        viewport->updateRT(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx,
    FrameBuffer* destinationBuffer)
{
    auto& viewports = ctx.m_registry->m_viewportRegistry->getViewports();

    if (viewports.empty()) return;

    bool forceWireframe = ctx.m_forceWireframe;
    ctx.m_forceWireframe = false;
    ctx.bindDefaults();

    ctx.m_state.setDepthFunc(GL_LEQUAL);

    // NOTE KI don't blend MAIN buffer
    bool blend = false;
    for (auto& viewport : viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        ctx.m_state.setEnabled(GL_BLEND, blend);
        viewport->draw(ctx);
        viewport->unbind(ctx);
        blend = true;
    }

    ctx.m_state.setEnabled(GL_BLEND, false);
    ctx.m_state.setDepthFunc(ctx.m_depthFunc);

    ctx.m_forceWireframe = forceWireframe;
    ctx.bindDefaults();
}
