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

    bool changed = !m_primary || w != m_primary->m_spec.width || h != m_primary->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("EFFECT_BUFFER: update - w={}, h={}", w, h));

    {
        // NOTE KI alpha NOT needed
        auto buffer = new FrameBuffer(
            "effect_primary",
            {
                w, h,
                {
                // src - diffuse from previous pass
                FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT0),
                // src - brighness from previous pass
                FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT1),
                // work - intermediate work buffer
                FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT2),
                //// dst - combine
                //FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT3),

                // NOTE KI *SHARE* depth with gbuffer
                // NOTE KI depth needed since there may be "non gbuffer" render steps
                FrameBufferAttachment::getShared(m_gBuffer->m_buffer->getDepthAttachment()),
            }
            });

        m_primary.reset(buffer);
        m_primary->prepare(true);

        m_buffers.clear();
        for (int i = 0; i < 2; i++) {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                "effect_work",
                {
                    w, h,
                    {
                    // src - diffuse from previous pass
                    FrameBufferAttachment::getEffectTexture(GL_COLOR_ATTACHMENT0),
                }
                });
            m_buffers.push_back(std::unique_ptr<FrameBuffer>(buffer));
        }

        for (auto& buf : m_buffers) {
            buf->prepare(true);
        }

        unbindTexture(ctx);
    }
}

void EffectBuffer::bind(const RenderContext& ctx)
{
    m_primary->bind(ctx);
}

void EffectBuffer::bindTexture(const RenderContext& ctx)
{
    m_primary->bindTexture(ctx, 0, UNIT_EFFECT_ALBEDO);
    m_primary->bindTexture(ctx, 1, UNIT_EFFECT_BRIGHT);
}

void EffectBuffer::unbindTexture(const RenderContext& ctx)
{
    m_primary->unbindTexture(ctx, UNIT_EFFECT_ALBEDO);
    m_primary->unbindTexture(ctx, UNIT_EFFECT_BRIGHT);
}

void EffectBuffer::clear()
{
    m_primary->clearAll();
    for (auto& buf : m_buffers) {
        buf->clearAll();
    }
}
