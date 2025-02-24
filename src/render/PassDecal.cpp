#include "PassDecal.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

#include "renderer/DecalRenderer.h"

namespace {
}

namespace render
{
    PassDecal::PassDecal()
        : Pass("PassDecal"),
        m_decalRenderer{ std::make_unique < DecalRenderer>(true) }
    {
    }

    PassDecal::~PassDecal() = default;

    void PassDecal::prepare(const PrepareContext& ctx)
    {
        m_decalRenderer->prepareRT(ctx);
    }

    void PassDecal::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDecal::initRender(const RenderContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

        m_enabled = ctx.m_useDecals &&
            dbg.m_decalEnabled;

        m_enabledBlend = m_enabled &&
            !ctx.m_forceSolid &&
            !ctx.m_forceLineMode;
    }

    PassContext PassDecal::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        src.buffer->bind(ctx);
        m_decalRenderer->renderSolid(ctx);

        return src;
    }

    PassContext PassDecal::renderOit(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabledBlend) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        src.buffer->bind(ctx);
        m_decalRenderer->renderBlend(ctx);

        return src;
    }
}
