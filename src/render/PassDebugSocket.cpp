#include "PassDebugSocket.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

#include "renderer/SocketRenderer.h"

namespace {
}

namespace render
{
    PassDebugSocket::PassDebugSocket()
        : Pass("PassDebugSocket"),
        m_socketRenderer{ std::make_unique<SocketRenderer>() }
    {
    }

    PassDebugSocket::~PassDebugSocket() = default;

    void PassDebugSocket::prepare(const PrepareContext& ctx)
    {
        m_socketRenderer->prepareRT(ctx);
    }

    void PassDebugSocket::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebugSocket::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

        m_enabled = ctx.m_allowDrawDebug &&
            dbg.m_animation.m_showSockets;
    }

    PassContext PassDebugSocket::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.getGLState();
        state.setStencil({});

        src.buffer->bind(ctx);

        m_socketRenderer->render(ctx, src.buffer);

        return src;
    }
}
