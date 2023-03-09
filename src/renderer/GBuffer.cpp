#include "GBuffer.h"

#include "scene/RenderContext.h"


void GBuffer::prepare(const Assets& assets)
{
}

void GBuffer::update(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;
    int w = ctx.assets.resolutionScale.x * res.x;
    int h = ctx.assets.resolutionScale.y * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_buffer || w != m_buffer->m_spec.width || h != m_buffer->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("G_BUFFER: update - w={}, h={}", w, h));

    {
        // NOTE KI alpha NOT needed
        auto buffer = new FrameBuffer({
            w, h,
            {
                FrameBufferAttachment::getGBufferAlbedo(),
                FrameBufferAttachment::getGBufferEmission(),
                FrameBufferAttachment::getGBufferPosition(),
                FrameBufferAttachment::getGBufferNormal(),
            }
        });

        m_buffer.reset(buffer);
        m_buffer->prepare(true, { 0, 0, 0, 0.0 });
    }
}

void GBuffer::bind(const RenderContext& ctx)
{
    m_buffer->bind(ctx);
}
