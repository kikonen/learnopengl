#include "ViewportRenderer.h"

#include "kigl/GLState.h"

#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"

#include "render/FrameBuffer.h"

#include "registry/Registry.h"
#include "registry/ViewportRegistry.h"


void ViewportRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);
}

void ViewportRenderer::updateRT(const UpdateViewContext& ctx)
{
    for (auto& viewport : ctx.m_registry->m_viewportRegistry->getViewports()) {
        viewport->updateRT(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* destinationBuffer)
{
    auto& viewports = ctx.m_registry->m_viewportRegistry->getViewports();

    if (viewports.empty()) return;

    auto& state = kigl::GLState::get();

    bool forceWireframe = ctx.m_forceWireframe;
    ctx.m_forceWireframe = false;
    ctx.bindDefaults();

    state.setDepthFunc(GL_LEQUAL);

    // NOTE KI don't blend MAIN buffer
    bool blend = false;
    for (auto& viewport : viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        state.setEnabled(GL_BLEND, blend);
        viewport->draw(ctx);
        viewport->unbind(ctx);
        blend = true;
    }

    state.setEnabled(GL_BLEND, false);
    state.setDepthFunc(ctx.m_depthFunc);

    ctx.m_forceWireframe = forceWireframe;
    ctx.bindDefaults();
}
