#include "GBuffer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/FrameBufferAttachment.h"

namespace {
}

namespace render {
    void GBuffer::prepare()
    {
    }

    void GBuffer::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        const auto& assets = ctx.m_assets;
        auto& dbg = render::DebugContext::get();

        const auto& res = ctx.m_resolution;

        int w = (int)(bufferScale * res.x);
        int h = (int)(bufferScale * res.y);
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
                        FrameBufferAttachment::getGBufferAlbedoHdr(ATT_ALBEDO_ENUM),
                        // NOTE KI no specular in metal workflow
                        //FrameBufferAttachment::getGBufferSpecularHdr(GL_COLOR_ATTACHMENT1),
                        FrameBufferAttachment::getGBufferEmissionHdr(ATT_EMISSION_ENUM),
                        //FrameBufferAttachment::getGBufferPosition(GL_COLOR_ATTACHMENT3),
                        FrameBufferAttachment::getGBufferNormal(ATT_NORMAL_ENUM),
                        FrameBufferAttachment::getGBufferMRA(ATT_MRAO_ENUM),
                        FrameBufferAttachment::getGBufferViewZ(ATT_VIEW_Z_ENUM),
                        // depth needed
                        FrameBufferAttachment::getDepthStencilTexture(),
                    }
                });

            m_buffer.reset(buffer);
            m_buffer->prepare();

            unbindTexture(ctx.m_state);
        }

        if (m_depthCopyEnabled)
        {
            m_depthTexture = std::make_unique< FrameBufferAttachment>();
            *m_depthTexture = FrameBufferAttachment::getDepthStencilTexture();
            m_depthTexture->create("gbuffer_ref_tex", w, h);
        }
        else {
            m_depthTexture.reset();
        }

        m_width = w;
        m_height = h;
    }

    void GBuffer::bind(const RenderContext& ctx)
    {
        m_buffer->bind(ctx);
    }

    void GBuffer::bindTexture(kigl::GLState& state)
    {
        m_buffer->bindTexture(state, ATT_ALBEDO_INDEX, UNIT_G_ALBEDO);
        //m_buffer->bindTexture(state, ATT_SPECULAR_INDEX, UNIT_G_SPECULAR);
        m_buffer->bindTexture(state, ATT_EMISSION_INDEX, UNIT_G_EMISSION);
        //m_buffer->bindTexture(state, ATT_POSITION_INDEX, UNIT_G_POSITION);
        m_buffer->bindTexture(state, ATT_NORMAL_INDEX, UNIT_G_NORMAL);
        m_buffer->bindTexture(state, ATT_MRAO_INDEX, UNIT_G_MRAO);
        m_buffer->bindTexture(state, ATT_VIEW_Z_INDEX, UNIT_G_VIEW_Z);
        m_buffer->bindTexture(state, ATT_DEPTH_INDEX, UNIT_G_DEPTH);

        if (m_depthCopyEnabled && m_depthTexture) {
            m_depthTexture->bindTexture(state, UNIT_G_DEPTH_COPY);
        }
    }

    void GBuffer::unbindTexture(kigl::GLState& state)
    {
        m_buffer->unbindTexture(state, UNIT_G_ALBEDO);
        //m_buffer->unbindTexture(state, UNIT_G_SPECULAR);
        m_buffer->unbindTexture(state, UNIT_G_EMISSION);
        //m_buffer->unbindTexture(state, UNIT_G_POSITION);
        m_buffer->unbindTexture(state, UNIT_G_MRAO);
        m_buffer->unbindTexture(state, UNIT_G_NORMAL);
        m_buffer->unbindTexture(state, UNIT_G_VIEW_Z);
        m_buffer->unbindTexture(state, UNIT_G_DEPTH);

        if (m_depthCopyEnabled && m_depthTexture) {
            //m_depthTexture->unbindTexture(state, UNIT_G_DEPTH);
        }
    }

    void GBuffer::bindDepthTexture(kigl::GLState& state)
    {
        m_buffer->bindTexture(state, ATT_DEPTH_INDEX, UNIT_G_DEPTH);
        m_depthTexture->bindTexture(state, UNIT_G_DEPTH_COPY);
    }

    void GBuffer::unbindDepthTexture(kigl::GLState& state)
    {
        m_buffer->unbindTexture(state, UNIT_G_DEPTH);
        //m_depthTexture->unbindTexture(state, UNIT_G_DEPTH);
    }

    void GBuffer::clearAll()
    {
        m_buffer->clearAll();
    }

    void GBuffer::invalidateAll()
    {
        m_buffer->invalidateAll();
    }

    void GBuffer::updateDepthCopy()
    {
        if (m_depthCopyEnabled && m_depthTexture) {
            m_buffer->copy(
                m_depthTexture.get(),
                GBuffer::ATT_DEPTH_INDEX);
        }
    }

    FrameBufferAttachment* GBuffer::getAttachment(int attachmentIndex)
    {
        return &m_buffer->m_spec.attachments[attachmentIndex];
    }
}
