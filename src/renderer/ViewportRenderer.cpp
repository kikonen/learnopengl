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
    for (auto& viewport : ViewportRegistry::get().getViewports()) {
        viewport->updateRT(ctx);
    }
}

void ViewportRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* destinationBuffer)
{
    auto& viewports = ViewportRegistry::get().getViewports();

    if (viewports.empty()) return;

    auto& state = ctx.m_state;

    bool forceLineMode = ctx.m_forceLineMode;
    ctx.m_forceLineMode = false;
    ctx.bindDefaults();

    state.setDepthFunc(GL_LEQUAL);

    for (auto& viewport : viewports) {
        viewport->setDestinationFrameBuffer(destinationBuffer);
        viewport->bind(ctx);
        state.setEnabled(GL_BLEND, viewport->isBlend());
        viewport->draw(ctx);
    }

    state.setEnabled(GL_BLEND, false);
    state.setDepthFunc(ctx.m_depthFunc);

    ctx.m_forceLineMode = forceLineMode;
    ctx.bindDefaults();
}
