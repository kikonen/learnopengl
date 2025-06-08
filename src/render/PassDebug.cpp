#include "PassDebug.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

namespace {
}

namespace render
{
    PassDebug::PassDebug()
        : Pass("PassDebug")
    {
    }

    PassDebug::~PassDebug() = default;

    void PassDebug::prepare(const PrepareContext& ctx)
    {
    }

    void PassDebug::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebug::initRender(const RenderContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

        m_enabled = ctx.m_allowDrawDebug &&
            dbg.m_drawDebug;
    }

    PassContext PassDebug::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        src.buffer->bind(ctx);

        passDebug(ctx, drawContext);

        return src;
    }

    void PassDebug::passDebug(
        const RenderContext& ctx,
        const DrawContext& drawContext)
    {
        ////m_effectBuffer.m_primary->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

        //constexpr float SZ1 = 0.25f;
        ////constexpr float SZ2 = 0.5f;

        //size_t count = 0;
        //float padding = 0.f;
        //for (int i = 0; i < m_oitBuffer.m_buffer->getDrawBufferCount(); i++) {
        //    m_oitBuffer.m_buffer->blit(
        //        targetBuffer,
        //        GL_COLOR_BUFFER_BIT,
        //        GL_COLOR_ATTACHMENT0 + i,
        //        GL_COLOR_ATTACHMENT0,
        //        { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
        //        GL_NEAREST);
        //    padding = 0.f;
        //}
        //count += m_oitBuffer.m_buffer->getDrawBufferCount();

        //padding = 0.01f;
        //for (int i = 0; i < m_effectBuffer.m_primary->getDrawBufferCount(); i++) {
        //    m_effectBuffer.m_primary->blit(
        //        targetBuffer,
        //        GL_COLOR_BUFFER_BIT,
        //        GL_COLOR_ATTACHMENT0 + i,
        //        GL_COLOR_ATTACHMENT0,
        //        { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
        //        GL_NEAREST);
        //    padding = 0.f;
        //}
        //count += m_effectBuffer.m_primary->getDrawBufferCount();

        //padding = 0.01f;
        //for (int i = 0; i < m_effectBuffer.m_secondary->getDrawBufferCount(); i++) {
        //    m_effectBuffer.m_secondary->blit(
        //        targetBuffer,
        //        GL_COLOR_BUFFER_BIT,
        //        GL_COLOR_ATTACHMENT0 + i,
        //        GL_COLOR_ATTACHMENT0,
        //        { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
        //        GL_NEAREST);
        //    padding = 0.f;
        //}
        //count += m_effectBuffer.m_secondary->getDrawBufferCount();

        //padding = 0.02f;
        //for (int i = 0; i < m_effectBuffer.m_buffers.size(); i++) {
        //    auto& buf = m_effectBuffer.m_buffers[i];
        //    buf->blit(
        //        targetBuffer,
        //        GL_COLOR_BUFFER_BIT,
        //        GL_COLOR_ATTACHMENT0,
        //        GL_COLOR_ATTACHMENT0,
        //        { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
        //        GL_NEAREST);
        //    padding = 0.f;
        //}
        //count += m_effectBuffer.m_buffers.size();

        //count = 0;
        //padding = 0.f;
        //for (int i = 0; i < m_gBuffer.m_buffer->getDrawBufferCount(); i++) {
        //    m_gBuffer.m_buffer->blit(
        //        targetBuffer,
        //        GL_COLOR_BUFFER_BIT,
        //        GL_COLOR_ATTACHMENT0 + i,
        //        GL_COLOR_ATTACHMENT0,
        //        { 1 - SZ1, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
        //        GL_NEAREST);
        //    padding = 0.f;
        //}
    }
}
