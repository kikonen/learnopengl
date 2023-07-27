#include "GBuffer.h"

#include "asset/Shader.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

namespace {
    static const int ATT_ALBEDO_INDEX = 0;
    static const int ATT_SPECULAR_INDEX = 1;
    static const int ATT_EMISSION_INDEX = 2;
    static const int ATT_NORMAL_INDEX = 3;
    static const int ATT_DEPTH_INDEX = 4;
}

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

    bool changed = w != m_width || h != m_height;
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
                    //FrameBufferAttachment::getGBufferPosition(GL_COLOR_ATTACHMENT3),
                    FrameBufferAttachment::getGBufferNormal(GL_COLOR_ATTACHMENT3),
                    // depth needed
                    FrameBufferAttachment::getDepthTexture()
                }
            });

        m_buffer.reset(buffer);
        m_buffer->prepare(true);

        unbindTexture(ctx);
    }

    m_width = w;
    m_height = h;
}

void GBuffer::bind(const RenderContext& ctx)
{
    m_buffer->bind(ctx);
}

void GBuffer::bindTexture(const RenderContext& ctx)
{
    m_buffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_G_ALBEDO);
    m_buffer->bindTexture(ctx, ATT_SPECULAR_INDEX, UNIT_G_SPECULAR);
    m_buffer->bindTexture(ctx, ATT_EMISSION_INDEX, UNIT_G_EMISSION);
    //m_buffer->bindTexture(ctx, ATT_POSITION_INDEX, UNIT_G_POSITION);
    m_buffer->bindTexture(ctx, ATT_NORMAL_INDEX, UNIT_G_NORMAL);
    m_buffer->bindTexture(ctx, ATT_DEPTH_INDEX, UNIT_G_DEPTH);
}

void GBuffer::unbindTexture(const RenderContext& ctx)
{
    m_buffer->unbindTexture(ctx, UNIT_G_ALBEDO);
    m_buffer->unbindTexture(ctx, UNIT_G_SPECULAR);
    m_buffer->unbindTexture(ctx, UNIT_G_EMISSION);
    //m_buffer->unbindTexture(ctx, UNIT_G_POSITION);
    m_buffer->unbindTexture(ctx, UNIT_G_NORMAL);
    m_buffer->unbindTexture(ctx, UNIT_G_DEPTH);
}

void GBuffer::bindDepthTexture(const RenderContext& ctx)
{
    m_buffer->bindTexture(ctx, 4, UNIT_G_DEPTH);
}

void GBuffer::unbindDepthTexture(const RenderContext& ctx)
{
    m_buffer->unbindTexture(ctx, UNIT_G_DEPTH);
}
