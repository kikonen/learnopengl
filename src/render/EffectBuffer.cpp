#include "EffectBuffer.h"

#include "render/GBuffer.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"


void EffectBuffer::prepare(
    const Assets& assets,
    GBuffer* gBuffer)
{
    m_gBuffer = gBuffer;
}

void EffectBuffer::updateView(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;

    int w = ctx.m_assets.gBufferScale * res.x;
    int h = ctx.m_assets.gBufferScale * res.y;
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
                //FrameBufferAttachment::getRBODepth(),
                FrameBufferAttachment::getDepthStencilTexture()
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
        for (int i = 0; i < 2; i++) {
            // NOTE KI alpha NOT needed
            auto buffer = new FrameBuffer(
                "effect_work",
                {
                    w, h,
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
