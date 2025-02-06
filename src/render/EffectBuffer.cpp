#include "EffectBuffer.h"

#include "asset/Assets.h"

#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/GBuffer.h"
#include "render/FrameBuffer.h"


namespace render {
    void EffectBuffer::prepare(
        GBuffer* gBuffer)
    {
        m_gBuffer = gBuffer;
    }

    void EffectBuffer::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        const auto& assets = Assets::get();
        auto& dbg = render::DebugContext::get();

        const auto& res = ctx.m_resolution;

        int w = (int)(bufferScale * res.x);
        int h = (int)(bufferScale * res.y);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_width || h != m_height;
        if (!changed) return;

        //if (m_mainBuffer) return;
        KI_INFO(fmt::format("EFFECT_BUFFER: update - w={}, h={}", w, h));

        // primary
        {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                "effect_primary",
                {
                    w, h,
                    {
                    // diffuse
                    FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                    // diffuse bright
                    FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT1),

                    // NOTE KI sharing depth buffer with gbuffer IS NOT VALID
                    // => reading and "writing" same depth buffer in shader is undefined operation
                    // NOTE KI depth needed since there may be "non gbuffer" render steps
                    // NOTE DepthTexture instead of RBODepth to allow *copy* instead of *blit*
                    // NOTE KI *SHARE* depth with gbuffer
                    FrameBufferAttachment::getShared(m_gBuffer->m_buffer->getDepthAttachment()),
                }
                });

            m_primary.reset(buffer);
            m_primary->prepare();
        }

        // secondary
        {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                "effect_secondary",
                {
                    w, h,
                    {
                    // diffuse
                    FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),

                    //// NOTE KI *SHARE* depth with gbuffer
                    //FrameBufferAttachment::getShared(m_gBuffer->m_buffer->getDepthAttachment()),
                }
                });

            m_secondary.reset(buffer);
            m_secondary->prepare();
        }

        // work buffers
        {
            m_buffers.clear();

            int workW = static_cast<int>(w * 0.5f);
            int workH = static_cast<int>(h * 0.5f);
            if (workW < 1) workW = 1;
            if (workH < 1) workH = 1;

            for (int i = 0; i < 2; i++) {
                // NOTE KI alpha NOT needed
                auto buffer = new FrameBuffer(
                    fmt::format("effect_worker_{}", i),
                    {
                        workW, workH,
                        {
                        // src - diffuse from previous pass
                        FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                    }
                    });
                m_buffers.push_back(std::unique_ptr<FrameBuffer>(buffer));
            }

            for (auto& buf : m_buffers) {
                buf->prepare();
            }
        }

        m_width = w;
        m_height = h;
    }

    void EffectBuffer::clearAll()
    {
        m_primary->clearAll();
        m_secondary->clearAll();
        for (auto& buf : m_buffers) {
            buf->clearAll();
        }
    }

    void EffectBuffer::invalidateAll()
    {
        m_primary->invalidateAll();
        m_secondary->invalidateAll();
        for (auto& buf : m_buffers) {
            buf->invalidateAll();
        }
    }

    void EffectBuffer::unbindTexture(const RenderContext& ctx)
    {
        m_primary->unbindTexture(ctx, UNIT_EFFECT_ALBEDO);
        m_primary->unbindTexture(ctx, UNIT_EFFECT_BRIGHT);

        m_secondary->unbindTexture(ctx, UNIT_EFFECT_ALBEDO);
        m_secondary->unbindTexture(ctx, UNIT_EFFECT_BRIGHT);

        for (auto& buf : m_buffers) {
            buf->unbindTexture(ctx, UNIT_EFFECT_WORK);
        }
    }
}
