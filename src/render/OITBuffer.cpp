#include "OITBuffer.h"

#include "asset/Shader.h"
#include "asset/Assets.h"

#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/GBuffer.h"
#include "render/FrameBuffer.h"

namespace render {
    void OITBuffer::prepare(
        GBuffer* gBuffer)
    {
        m_gBuffer = gBuffer;
    }

    void OITBuffer::updateRT(const UpdateViewContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        const auto& res = ctx.m_resolution;

        int w = (int)(assets.gBufferScale * res.x);
        int h = (int)(assets.gBufferScale * res.y);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_width || h != m_height;
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
                        FrameBufferAttachment::getShared(m_gBuffer->m_buffer->getDepthAttachment()),
                    }
                });

            m_buffer.reset(buffer);
            m_buffer->prepare();

            //unbindTexture(ctx);
        }

        m_width = w;
        m_height = h;
    }

    void OITBuffer::bind(const RenderContext& ctx)
    {
        m_buffer->bind(ctx);
    }

    void OITBuffer::bindTexture(const RenderContext& ctx)
    {
        m_buffer->bindTexture(ctx, ATT_ACCUMULATOR_INDEX, UNIT_OIT_ACCUMULATOR);
        m_buffer->bindTexture(ctx, ATT_REVEAL_INDEX, UNIT_OIT_REVEAL);
    }

    void OITBuffer::unbindTexture(const RenderContext& ctx)
    {
        m_buffer->unbindTexture(ctx, UNIT_OIT_ACCUMULATOR);
        m_buffer->unbindTexture(ctx, UNIT_OIT_REVEAL);
    }

    void OITBuffer::clearAll()
    {
        m_buffer->clearAll();
    }

    void OITBuffer::invalidateAll()
    {
        m_buffer->invalidateAll();
    }
}
