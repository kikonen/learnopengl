#include "BlurBuffer.h"

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
    BlurBuffer::BlurBuffer() = default;
    BlurBuffer::~BlurBuffer() = default;

    void BlurBuffer::prepare()
    {
    }

    void BlurBuffer::updateRT(const UpdateViewContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& dbg = render::DebugContext::get();

        const auto& res = ctx.m_resolution;
        const auto bufferScale = dbg.getGBufferScale();

        int w = (int)(bufferScale * res.x);
        int h = (int)(bufferScale * res.y);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_width || h != m_height;
        if (!changed) return;

        KI_INFO(fmt::format("BLUR_BUFFER: update - w={}, h={}", w, h));

        int currW = w;
        int currH = h;

        m_buffers.clear();

        for (int i = 0; i < BUFFER_COUNT; i++)
        {
            auto fb = new FrameBuffer(
                fmt::format("blur_buffer_{}", i),
                {
                    w, h,
                    {
                        FrameBufferAttachment::getGBufferEmissionHdr(ATT_COLOR_A),
                        FrameBufferAttachment::getGBufferEmissionHdr(ATT_COLOR_B),
                    }
                });

            m_buffers.push_back(std::unique_ptr<FrameBuffer>(fb));

            currW = std::round(currW * 0.5f);
            currH = std::round(currH * 0.5f);
            if (currW < 1) currW = 1;
            if (currH < 1) currH = 1;
        }

        for (auto& buf : m_buffers) {
            buf->prepare();
        }

        m_width = w;
        m_height = h;
    }

    void BlurBuffer::bind(
        const RenderContext& ctx,
        int bufferIndex)
    {
        auto& buffer = m_buffers[bufferIndex];
        buffer->bind(ctx);
    }

    void BlurBuffer::bindTexture(
        const RenderContext& ctx,
        int bufferIndex,
        int attachmentIndex,
        int unitIndex)
    {
        auto& buffer = m_buffers[bufferIndex];

        buffer->bindTexture(ctx, attachmentIndex, unitIndex);
    }

    void BlurBuffer::unbindTexture(
        const RenderContext& ctx,
        int bufferIndex,
        int unitIndex)
    {
        auto& buffer = m_buffers[bufferIndex];

        buffer->unbindTexture(ctx, unitIndex);
    }
}
