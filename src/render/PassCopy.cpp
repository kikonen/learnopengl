#include "PassCopy.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

namespace {
}

namespace render
{
    PassCopy::PassCopy()
        : Pass("PassCopy")
    {
    }

    PassCopy::~PassCopy() = default;

    void PassCopy::prepare(const PrepareContext& ctx)
    {
    }

    void PassCopy::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassCopy::initRender(const RenderContext& ctx)
    {
        m_enabled = true;
    }

    PassContext PassCopy::copy(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src,
        FrameBuffer* dstBuffer)
    {
        if (!m_enabled) return src;

        src.buffer->bind(ctx);

        passCopy(ctx, src, dstBuffer, drawContext.copyMask);

        return src;
    }

    void PassCopy::passCopy(
        const RenderContext& ctx,
        const PassContext& src,
        FrameBuffer* dstBuffer,
        GLbitfield copyMask)
    {
        const auto* srcBuffer = src.buffer;
        const int dstAttachmentIndex = 0;

        if (copyMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
            srcBuffer->blit(
                dstBuffer,
                copyMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT),
                { -1.f, 1.f },
                { 2.f, 2.f },
                GL_NEAREST);
        }

        if (copyMask & GL_COLOR_BUFFER_BIT) {
            GLenum sourceFormat = srcBuffer->m_spec.attachments[src.attachmentIndex].internalFormat;
            GLenum targetFormat = -1;

            if (!dstBuffer->m_spec.attachments.empty()) {
                targetFormat = dstBuffer->m_spec.attachments[dstAttachmentIndex].internalFormat;
            }

            const bool canCopy = !dstBuffer->m_spec.attachments.empty() &&
                dstBuffer->m_spec.width == srcBuffer->m_spec.width &&
                dstBuffer->m_spec.height == srcBuffer->m_spec.height &&
                targetFormat == sourceFormat;

            if (canCopy) {
                srcBuffer->copy(
                    dstBuffer,
                    src.attachmentIndex,
                    // NOTE KI assumption; buffer is at index 0
                    0);
            }
            else {
                srcBuffer->blit(
                    dstBuffer,
                    GL_COLOR_BUFFER_BIT,
                    GL_COLOR_ATTACHMENT0,
                    GL_COLOR_ATTACHMENT0,
                    { -1.f, 1.f },
                    { 2.f, 2.f },
                    GL_LINEAR);
            }
        }
    }
}
