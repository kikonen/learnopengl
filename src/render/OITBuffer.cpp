#include "OITBuffer.h"

#include "asset/Assets.h"

#include <fmt/format.h>

#include "shader/Shader.h"

#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/GBuffer.h"
#include "render/FrameBuffer.h"

namespace render {
    void OITBuffer::prepare()
    {
    }

    void OITBuffer::updateRT(
        const UpdateViewContext& ctx,
        GBuffer* gBuffer,
        const std::string& namePrefix,
        float bufferScale)
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
        KI_INFO(fmt::format("OIT_BUFFER: update - w={}, h={}", w, h));

        {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                fmt::format("{}_oit_buffer", namePrefix),
                {
                    w, h,
                    {
                        FrameBufferAttachment::getOITAccumulatorTexture(GL_COLOR_ATTACHMENT0),
                        FrameBufferAttachment::getOITRevealTexture(GL_COLOR_ATTACHMENT1),
                        FrameBufferAttachment::getShared(
                            gBuffer->getAttachment(GBuffer::ATT_EMISSION_INDEX),
                            GL_COLOR_ATTACHMENT2),
                        // NOTE KI *SHARE* depth with gbuffer
                        FrameBufferAttachment::getShared(gBuffer->m_buffer->getDepthAttachment()),
                    }
                });

            m_buffer.reset(buffer);
            m_buffer->prepare();

            unbindTexture(ctx.m_state);
        }

        m_width = w;
        m_height = h;
    }

    void OITBuffer::bind(const RenderContext& ctx)
    {
        m_buffer->bind(ctx);
    }

    void OITBuffer::bindTexture(kigl::GLState& state)
    {
        m_buffer->bindTexture(state, ATT_ACCUMULATOR_INDEX, UNIT_OIT_ACCUMULATOR);
        m_buffer->bindTexture(state, ATT_REVEAL_INDEX, UNIT_OIT_REVEAL);
    }

    void OITBuffer::unbindTexture(kigl::GLState& state)
    {
        m_buffer->unbindTexture(state, UNIT_OIT_ACCUMULATOR);
        m_buffer->unbindTexture(state, UNIT_OIT_REVEAL);
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
