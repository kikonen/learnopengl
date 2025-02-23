#include "ViewportRenderer.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
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
    auto& dbg = render::DebugContext::get();

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
                    "final_buffer",
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
    const RenderContext& ctx,
    render::FrameBuffer* destinationBuffer)
{
    auto& viewports = ViewportRegistry::get().getViewports();

    if (viewports.empty()) return;

    auto& state = ctx.m_state;

    bool forceLineMode = ctx.m_forceLineMode;
    ctx.m_forceLineMode = false;
    ctx.bindDefaults();

    state.polygonFrontAndBack(GL_FILL);

    {
        auto* buffer = m_buffer.get();

        buffer->bind(ctx);
        buffer->clear(
            ctx,
            GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
            { 0.f, 0.f, 0.f, 1.f });

        state.setDepthFunc(GL_LEQUAL);

        state.invalidateBlendMode();
        state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        for (auto& viewport : viewports) {
            if (!viewport->isEnabled()) continue;

            viewport->bind(ctx);
            state.setEnabled(GL_BLEND, viewport->isBlend());
            viewport->draw(ctx, buffer);
        }

        state.setEnabled(GL_BLEND, false);
        state.setDepthFunc(ctx.m_depthFunc);
    }

    {
        auto* program = Program::get(m_blitterId);
        state.bindTexture(UNIT_VIEWPORT, m_buffer->m_spec.attachments[0].textureID, true);

        state.setEnabled(GL_DEPTH_TEST, false);

        destinationBuffer->bind(ctx);

        // NOTE KI this clears *window* buffer, not actual "main" buffer used for drawing
        // => Stencil is not supposed to exist here
        // => no need to clear this; ViewPort will do glBlitNamedFramebuffer
        // => *BUT* if glDraw is used instead then clear *IS* needed for depth
        //
        // NOTE KI *CLEAR* buffer
        // - https://stackoverflow.com/questions/37335281/is-glcleargl-color-buffer-bit-preferred-before-a-whole-frame-buffer-overwritte
        //
        destinationBuffer->clear(
            ctx,
            GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
            { 0.f, 0.f, 0.f, 1.f });

        program->bind();

        auto* uniforms = program->m_uniforms.get();

        uniforms->u_hdrToneEnabled.set(m_hdrToneMappingEnabled);
        uniforms->u_gammaCorrectEnabled.set(m_hardwareGammaEnabled ? false : m_gammaCorrectEnabled);

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

    ctx.m_forceLineMode = forceLineMode;
    ctx.bindDefaults();
}
