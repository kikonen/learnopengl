#include "SsaoBuffer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

namespace render {
    SsaoBuffer::SsaoBuffer()
    { }

    SsaoBuffer::~SsaoBuffer() = default;

    void SsaoBuffer::prepare()
    { }

    void SsaoBuffer::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        const auto& assets = ctx.m_assets;
        auto& dbg = render::DebugContext::get();

        const auto& res = ctx.m_resolution;
        const float ssaoScale = 0.5f;

        // NOTE KI use smaller size buffer for SSAO to save GPU time
        int w = (int)(bufferScale * res.x * ssaoScale);
        int h = (int)(bufferScale * res.y * ssaoScale);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_width || h != m_height;
        if (!changed) return;

        //if (m_mainBuffer) return;
        KI_INFO(fmt::format("SSAO_BUFFER: update - w={}, h={}", w, h));

        {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                fmt::format("{}_ssao_buffer_{}x{}", namePrefix, w, h),
                {
                    w, h,
                    {
                        FrameBufferAttachment::getSsaoTexture(GL_COLOR_ATTACHMENT0),
                        FrameBufferAttachment::getSsaoTexture(GL_COLOR_ATTACHMENT1),
                    }
                 });

            m_buffer.reset(buffer);
            m_buffer->prepare();

            unbindSsaoTexture(ctx.m_state);
            unbindSsaoBlurTexture(ctx.m_state);
        }

        m_width = w;
        m_height = h;
    }

    void SsaoBuffer::bind(const RenderContext& ctx)
    {
        m_buffer->bind(ctx);
    }

    void SsaoBuffer::bindSsaoTexture(kigl::GLState& state)
    {
        m_buffer->bindTexture(state, ATT_SSAO_INDEX, UNIT_SSAO);
    }

    void SsaoBuffer::bindSsaoBlurTexture(kigl::GLState& state)
    {
        m_buffer->bindTexture(state, ATT_SSAO_BLUR_INDEX, UNIT_SSAO_BLUR);
    }

    void SsaoBuffer::unbindSsaoTexture(kigl::GLState& state)
    {
        m_buffer->unbindTexture(state, UNIT_SSAO);
    }

    void SsaoBuffer::unbindSsaoBlurTexture(kigl::GLState& state)
    {
        m_buffer->unbindTexture(state, UNIT_SSAO_BLUR);
    }

    void SsaoBuffer::clearAll()
    {
        m_buffer->clearAll();
    }

    void SsaoBuffer::invalidateAll()
    {
        m_buffer->invalidateAll();
    }
}
