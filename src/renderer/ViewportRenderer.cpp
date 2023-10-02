#include "ViewportRenderer.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/FrameBuffer.h"

#include "registry/Registry.h"
#include "registry/ViewportRegistry.h"


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
    for (auto& viewport : ctx.m_registry->m_viewportRegistry->getViewports()) {
        viewport->update(ctx);
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

    ctx.m_state.setEnabled(GL_DEPTH_TEST, false);
    //ctx.m_state.setDepthFunc(GL_LEQUAL);
    //ctx.m_state.setEnabled(GL_BLEND, true);

    for (auto& viewport : viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        viewport->draw(ctx);
        viewport->unbind(ctx);
    }

    //ctx.m_state.setEnabled(GL_BLEND, false);
    //ctx.m_state.setDepthFunc(ctx.m_depthFunc);
    ctx.m_state.setEnabled(GL_DEPTH_TEST, true);

    ctx.m_forceWireframe = forceWireframe;
    ctx.bindDefaults();
}
