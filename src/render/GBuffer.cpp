#include "GBuffer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/FrameBufferAttachment.h"

namespace {
}

namespace render {
    void GBuffer::prepare()
    {
    }

    void GBuffer::updateRT(const UpdateViewContext& ctx)
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
        KI_INFO(fmt::format("G_BUFFER: update - w={}, h={}", w, h));

        {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                "gbuffer",
                {
                    w, h,
                    {
                        FrameBufferAttachment::getGBufferAlbedoHdr(GL_COLOR_ATTACHMENT0),
                        // NOTE KI no specular in metal workflow
                        //FrameBufferAttachment::getGBufferSpecularHdr(GL_COLOR_ATTACHMENT1),
                        FrameBufferAttachment::getGBufferEmission(GL_COLOR_ATTACHMENT1),
                        //FrameBufferAttachment::getGBufferPosition(GL_COLOR_ATTACHMENT3),
                        FrameBufferAttachment::getGBufferNormal(GL_COLOR_ATTACHMENT2),
                        FrameBufferAttachment::getGBufferMetal(GL_COLOR_ATTACHMENT3),
                        FrameBufferAttachment::getGBufferViewZ(GL_COLOR_ATTACHMENT4),
                        // depth needed
                        FrameBufferAttachment::getDepthStencilTexture(),
                    }
                });

            m_buffer.reset(buffer);
            m_buffer->prepare();

            //unbindTexture(ctx);
        }

        {
            m_depthTexture = std::make_unique< FrameBufferAttachment>();
            *m_depthTexture = FrameBufferAttachment::getDepthStencilTexture();
            m_depthTexture->create("gbuffer_ref_tex", w, h);
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
        //m_buffer->bindTexture(ctx, ATT_SPECULAR_INDEX, UNIT_G_SPECULAR);
        m_buffer->bindTexture(ctx, ATT_EMISSION_INDEX, UNIT_G_EMISSION);
        //m_buffer->bindTexture(ctx, ATT_POSITION_INDEX, UNIT_G_POSITION);
        m_buffer->bindTexture(ctx, ATT_NORMAL_INDEX, UNIT_G_NORMAL);
        m_buffer->bindTexture(ctx, ATT_METAL_INDEX, UNIT_G_METAL);
        m_buffer->bindTexture(ctx, ATT_VIEW_Z_INDEX, UNIT_G_VIEW_Z);
        //m_buffer->bindTexture(ctx, ATT_DEPTH_INDEX, UNIT_G_DEPTH);

        m_depthTexture->bindTexture(ctx, UNIT_G_DEPTH);
    }

    void GBuffer::unbindTexture(const RenderContext& ctx)
    {
        m_buffer->unbindTexture(ctx, UNIT_G_ALBEDO);
        //m_buffer->unbindTexture(ctx, UNIT_G_SPECULAR);
        m_buffer->unbindTexture(ctx, UNIT_G_EMISSION);
        //m_buffer->unbindTexture(ctx, UNIT_G_POSITION);
        m_buffer->unbindTexture(ctx, UNIT_G_METAL);
        m_buffer->unbindTexture(ctx, UNIT_G_NORMAL);
        m_buffer->unbindTexture(ctx, UNIT_G_VIEW_Z);
        m_buffer->unbindTexture(ctx, UNIT_G_DEPTH);

        m_depthTexture->unbindTexture(ctx, UNIT_G_DEPTH);
    }

    void GBuffer::bindDepthTexture(const RenderContext& ctx)
    {
        m_depthTexture->bindTexture(ctx, UNIT_G_DEPTH);
    }

    void GBuffer::unbindDepthTexture(const RenderContext& ctx)
    {
        m_depthTexture->unbindTexture(ctx, UNIT_G_DEPTH);
    }

    void GBuffer::clearAll()
    {
        m_buffer->clearAll();
    }

    void GBuffer::invalidateAll()
    {
        m_buffer->invalidateAll();
    }
}
