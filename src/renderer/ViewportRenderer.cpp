#include "ViewportRenderer.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "engine/UpdateViewContext.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/ScreenTri.h"
#include "render/FrameBuffer.h"
#include "render/FrameBufferAttachment.h"

#include "registry/Registry.h"
#include "registry/ViewportRegistry.h"

#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/Uniform.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

namespace {
    inline const std::string SHADER_WINDOW_BLIT{ "window_blit" };
}

ViewportRenderer::ViewportRenderer(bool useFrameStep) :
    Renderer("main", useFrameStep) {
}

ViewportRenderer::~ViewportRenderer() = default;

void ViewportRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    m_blitterId = ProgramRegistry::get().getProgram(SHADER_WINDOW_BLIT);

    setGammaCorrectEnabled(true);
    setHardwareGammaEnabled(true);
}

void ViewportRenderer::updateRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;
    auto& dbg = debug::DebugContext::get();

    for (auto& viewport : ViewportRegistry::get().getViewports()) {
        viewport->updateRT(ctx);
    }

    {
        const auto& res = ctx.m_resolution;
        const auto bufferScale = dbg.getGBufferScale();

        int w = (int)(bufferScale * res.x);
        int h = (int)(bufferScale * res.y);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_width || h != m_height;
        if (changed) {

            KI_INFO(fmt::format("FINAL_BUFFER: update - w={}, h={}", w, h));

            {
                // NOTE KI alpha NOT needed
                auto buffer = new render::FrameBuffer(
                    fmt::format("final_buffer_{}x{}", w, h),
                    {
                        w, h,
                        {
                            render::FrameBufferAttachment::getTextureRGBAHdr(GL_COLOR_ATTACHMENT0),
                            render::FrameBufferAttachment::getDepthRbo(),
                        }
                    });

                m_buffer.reset(buffer);
                m_buffer->prepare();
            }

            m_width = w;
            m_height = h;
        }
    }
}

void ViewportRenderer::render(
    const render::RenderContext& parentCtx,
    render::FrameBuffer* destinationBuffer)
{
    render::RenderContext localCtx(parentCtx);
    localCtx.m_forceLineMode = false;
    localCtx.bindDefaults();

    drawViewports(localCtx);
    blitWindow(localCtx, destinationBuffer);
}

void ViewportRenderer::drawViewports(
    const render::RenderContext& ctx)
{
    auto& state = ctx.m_state;

    auto& viewports = ViewportRegistry::get().getViewports();

    if (viewports.empty()) return;

    state.polygonFrontAndBack(GL_FILL);
    state.setDepthFunc(GL_LEQUAL);

    state.setStencil({});
    state.invalidateBlendMode();
    state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

    auto* buffer = m_buffer.get();

    buffer->bind(ctx);
    buffer->clear(
        ctx,
        GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
        { 0.f, 0.f, 0.f, 1.f });

    for (auto& viewport : viewports) {
        viewport->draw(ctx, buffer);
    }

    state.setEnabled(GL_BLEND, false);
    state.setDepthFunc(ctx.m_depthFunc);
}

void ViewportRenderer::blitWindow(
    const render::RenderContext& ctx,
    render::FrameBuffer* destinationBuffer)
{
    auto& state = ctx.m_state;

    state.polygonFrontAndBack(GL_FILL);
    state.setEnabled(GL_DEPTH_TEST, false);
    state.bindTexture(UNIT_VIEWPORT, m_buffer->m_spec.attachments[0].textureID, true);

    // NOTE KI this clears *window* buffer, not actual "main" buffer used for drawing
    // => Stencil is not supposed to exist here
    // => no need to clear this; ViewPort will do glBlitNamedFramebuffer
    // => *BUT* if glDraw is used instead then clear *IS* needed for depth
    //
    // NOTE KI *CLEAR* buffer
    // - https://stackoverflow.com/questions/37335281/is-glcleargl-color-buffer-bit-preferred-before-a-whole-frame-buffer-overwritte
    //
    destinationBuffer->bind(ctx);
    destinationBuffer->clear(
        ctx,
        GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
        { 0.f, 0.f, 0.f, 1.f });

    auto* program = Program::get(m_blitterId);
    {
        program->bind();

        auto* uniforms = program->m_uniforms.get();

        uniforms->u_hdrToneEnabled.set(m_hdrToneMappingEnabled);
        uniforms->u_gammaCorrectEnabled.set(m_hardwareGammaEnabled ? false : m_gammaCorrectEnabled);
    }

    if (m_gammaCorrectEnabled && m_hardwareGammaEnabled) {
        glEnable(GL_FRAMEBUFFER_SRGB);
        render::ScreenTri::get().draw();
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    else {
        render::ScreenTri::get().draw();
    }

    state.setEnabled(GL_DEPTH_TEST, true);
}
