#include "ViewportRenderer.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/ScreenTri.h"
#include "render/FrameBuffer.h"
#include "render/FrameBufferAttachment.h"

#include "registry/Registry.h"
#include "registry/ViewportRegistry.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"


void ViewportRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    m_blitterId = ProgramRegistry::get().getProgram(SHADER_WINDOW_BLIT);
}

void ViewportRenderer::updateRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;

    for (auto& viewport : ViewportRegistry::get().getViewports()) {
        viewport->updateRT(ctx);
    }

    {
        const auto& res = ctx.m_resolution;

        int w = (int)(assets.gBufferScale * res.x);
        int h = (int)(assets.gBufferScale * res.y);
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
                            render::FrameBufferAttachment::getTextureRGBA(GL_COLOR_ATTACHMENT0),
                            render::FrameBufferAttachment::getDepthStencilTexture(),
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

    {
        m_buffer->bind(ctx);
        m_buffer->clear(
            ctx,
            GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
            { 0.f, 0.f, 0.f, 1.f });

        state.setDepthFunc(GL_LEQUAL);

        glBlendFunci(0, GL_ONE, GL_ONE);
        glBlendFunci(1, GL_ONE, GL_ONE);
        state.invalidateBlendMode();
        state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        for (auto& viewport : viewports) {
            viewport->setDestinationFrameBuffer(m_buffer.get());
            viewport->bind(ctx);
            state.setEnabled(GL_BLEND, viewport->isBlend());
            viewport->draw(ctx);
        }

        state.setEnabled(GL_BLEND, false);
        state.setDepthFunc(ctx.m_depthFunc);
    }

    {
        auto* program = Program::get(m_blitterId);
        state.bindTexture(UNIT_VIEWPORT, m_buffer->m_spec.attachments[0].textureID, true);

        state.setEnabled(GL_DEPTH_TEST, false);

        destinationBuffer->bind(ctx);
        program->bind();
        render::ScreenTri::get().draw();

        state.setEnabled(GL_DEPTH_TEST, true);
    }

    ctx.m_forceLineMode = forceLineMode;
    ctx.bindDefaults();
}
