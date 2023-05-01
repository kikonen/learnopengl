#include "OITBuffer.h"

#include "asset/Shader.h"

#include "render/GBuffer.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"


void OITBuffer::prepare(
    const Assets& assets,
    GBuffer* gbuffer)
{
    m_gbuffer = gbuffer;
}

void OITBuffer::updateView(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;

    int w = ctx.m_assets.gbufferScale * res.x;
    int h = ctx.m_assets.gbufferScale * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_buffer || w != m_buffer->m_spec.width || h != m_buffer->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("OIT_BUFFER: update - w={}, h={}", w, h));

    {
        // NOTE KI alpha NOT needed
        auto buffer = new FrameBuffer(
            "oit_buffer",
            {
                w, h,
                {
                    FrameBufferAttachment::getOITAccumulatorTexture(GL_COLOR_ATTACHMENT0),
                    FrameBufferAttachment::getOITRevealTexture(GL_COLOR_ATTACHMENT1),
                    // NOTE KI *SHARE* depth with gbuffer
                    FrameBufferAttachment::getShared(m_gbuffer->m_buffer->getDepthAttachment()),
                }
            });

        m_buffer.reset(buffer);
        m_buffer->prepare(true);

        unbindTexture(ctx);
    }
}

void OITBuffer::bind(const RenderContext& ctx)
{
    m_buffer->bind(ctx);
}

void OITBuffer::bindTexture(const RenderContext& ctx)
{
    m_buffer->bindTexture(ctx, 0, UNIT_OIT_ACCUMULATOR);
    m_buffer->bindTexture(ctx, 1, UNIT_OIT_REVEAL);
}

void OITBuffer::unbindTexture(const RenderContext& ctx)
{
    m_buffer->unbindTexture(ctx, UNIT_OIT_ACCUMULATOR);
    m_buffer->unbindTexture(ctx, UNIT_OIT_REVEAL);
}
