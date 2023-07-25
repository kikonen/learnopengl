#include "EffectBuffer.h"

#include "render/GBuffer.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"


void EffectBuffer::prepare(
    const Assets& assets,
    GBuffer* gBuffer)
{
    m_gBuffer = gBuffer;
}

void EffectBuffer::updateView(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;

    int w = ctx.m_assets.gbufferScale * res.x;
    int h = ctx.m_assets.gbufferScale * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_buffer || w != m_buffer->m_spec.width || h != m_buffer->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("EFFECT_BUFFER: update - w={}, h={}", w, h));

    {
        // NOTE KI alpha NOT needed
        auto buffer = new FrameBuffer(
            "effect",
            {
                w, h,
                {
                    // src - diffuse from previous pass
                    FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT0),
                    // src - brighness from previous pass
                    //FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT1),
                    //// work - intermediate work buffer
                    //FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT0),
                    //// dst - combined diffuse
                    //FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT0),

                    // NOTE KI *SHARE* depth with gbuffer
                    FrameBufferAttachment::getShared(m_gBuffer->m_buffer->getDepthAttachment()),
                }
            });

        m_buffer.reset(buffer);
        m_buffer->prepare(true);

        unbindTexture(ctx);
    }
}

void EffectBuffer::bind(const RenderContext& ctx)
{
    m_buffer->bind(ctx);
}

void EffectBuffer::bindTexture(const RenderContext& ctx)
{
    m_buffer->bindTexture(ctx, 0, UNIT_EFFECT_ALBEDO);
    //m_buffer->bindTexture(ctx, 1, UNIT_EFFECT_BRIGHT);
    //m_buffer->bindTexture(ctx, 2, UNIT_EFFECT_WORK);
}

void EffectBuffer::unbindTexture(const RenderContext& ctx)
{
    m_buffer->unbindTexture(ctx, UNIT_EFFECT_ALBEDO);
    //m_buffer->unbindTexture(ctx, UNIT_EFFECT_BRIGHT);
    //m_buffer->unbindTexture(ctx, UNIT_EFFECT_WORK);
}
