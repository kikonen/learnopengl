#include "BlurBuffer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "engine/UpdateViewContext.h"

#include "debug/DebugContext.h"
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

    void BlurBuffer::updateView(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        const auto& assets = ctx.getAssets();
        auto& dbg = debug::DebugContext::get();

        const auto& res = ctx.m_resolution;

        int w;
        int h;
        {
            w = (int)(bufferScale * res.x);
            h = (int)(bufferScale * res.y);
            if (w < 1) w = 1;
            if (h < 1) h = 1;

            bool changed = w != m_width || h != m_height;
            if (!changed) return;

            KI_INFO(fmt::format("BLUR_BUFFER: update - w={}, h={}", w, h));
        }

        int currW = w;
        int currH = h;

        m_frameBuffers.clear();

        for (int i = 0; i < BUFFER_COUNT; i++)
        {
            currW = static_cast<int>(std::round(currW * 0.5f));
            currH = static_cast<int>(std::round(currH * 0.5f));
            if (currW < 1) currW = 1;
            if (currH < 1) currH = 1;

            auto fbo = util::Ref<FrameBuffer>::create(
                fmt::format("{}_blur_buffer_{}x{}_{}", namePrefix, w, h, i),
                FrameBufferSpecification {
                    w, h,
                    {
                        FrameBufferAttachment::getGBufferEmissionHdr(ATT_COLOR_A_ENUM),
                        FrameBufferAttachment::getGBufferEmissionHdr(ATT_COLOR_B_ENUM),
                    }
                });

            m_frameBuffers.push_back(fbo);
        }

        for (auto& fbo : m_frameBuffers) {
            fbo->prepare();
        }

        m_width = w;
        m_height = h;
    }

    void BlurBuffer::bind(
        const RenderContext& ctx,
        int bufferIndex)
    {
        auto& fbo = m_frameBuffers[bufferIndex];
        fbo->bind(ctx);
    }

    void BlurBuffer::bindTexture(
        kigl::GLState& state,
        int bufferIndex,
        int attachmentIndex,
        int unitIndex)
    {
        auto& fbo = m_frameBuffers[bufferIndex];

        fbo->bindTexture(state, attachmentIndex, unitIndex);
    }

    void BlurBuffer::unbindTexture(
        kigl::GLState& state,
        int bufferIndex,
        int unitIndex)
    {
        auto& fbo = m_frameBuffers[bufferIndex];

        fbo->unbindTexture(state, unitIndex);
    }
}
