#include "GBuffer.h"

#include "asset/Shader.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"


void GBuffer::prepare(const Assets& assets)
{
}

void GBuffer::updateView(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;

    int w = ctx.m_assets.gbufferScale * res.x;
    int h = ctx.m_assets.gbufferScale * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_buffer || w != m_buffer->m_spec.width || h != m_buffer->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("G_BUFFER: update - w={}, h={}", w, h));

    {
        // NOTE KI alpha NOT needed
        auto buffer = new FrameBuffer(
            "gbuffer",
            {
                w, h,
                {
                    FrameBufferAttachment::getGBufferAlbedo(GL_COLOR_ATTACHMENT0),
                    FrameBufferAttachment::getGBufferSpecular(GL_COLOR_ATTACHMENT1),
                    FrameBufferAttachment::getGBufferEmission(GL_COLOR_ATTACHMENT2),
                    //FrameBufferAttachment::getGBufferAmbient(GL_COLOR_ATTACHMENT3),
                    FrameBufferAttachment::getGBufferPosition(GL_COLOR_ATTACHMENT3),
                    FrameBufferAttachment::getGBufferNormal(GL_COLOR_ATTACHMENT4),
                    // depth needed
                    FrameBufferAttachment::getRBODepth()
                }
            });

        m_buffer.reset(buffer);
        m_buffer->prepare(true);

        unbindTexture(ctx);
    }
}

void GBuffer::bind(const RenderContext& ctx)
{
    m_buffer->bind(ctx);
}

void GBuffer::bindTexture(const RenderContext& ctx)
{
    m_buffer->bindTexture(ctx, 0, UNIT_G_ALBEDO);
    m_buffer->bindTexture(ctx, 1, UNIT_G_SPECULAR);
    m_buffer->bindTexture(ctx, 2, UNIT_G_EMISSION);
    m_buffer->bindTexture(ctx, 3, UNIT_G_POSITION);
    m_buffer->bindTexture(ctx, 4, UNIT_G_NORMAL);
}

void GBuffer::unbindTexture(const RenderContext& ctx)
{
    m_buffer->unbindTexture(ctx, UNIT_G_ALBEDO);
    m_buffer->unbindTexture(ctx, UNIT_G_SPECULAR);
    m_buffer->unbindTexture(ctx, UNIT_G_EMISSION);
    m_buffer->unbindTexture(ctx, UNIT_G_POSITION);
    m_buffer->unbindTexture(ctx, UNIT_G_NORMAL);
}
