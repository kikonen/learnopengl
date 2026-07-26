#include "PassEffect.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "model/Node.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"
#include "render/DrawableInfo.h"

namespace {
}

namespace render
{
    PassEffect::PassEffect()
        : Pass("PassEffect")
    {
    }

    PassEffect::~PassEffect() = default;

    void PassEffect::prepare(const PrepareContext& ctx)
    {
    }

    void PassEffect::updateView(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassEffect::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

        m_enabled = !ctx.m_forceSolid;
    }

    PassContext PassEffect::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        src.buffer->bind(ctx);

        passEffect(ctx, drawContext);

        return src;
    }

    // pass 7 - blend effects
    // => separate light calculations
    void PassEffect::passEffect(
        const RenderContext& ctx,
        const DrawContext& drawContext)
    {
        auto& state = ctx.getGLState();

        state.setStencil({});
        state.setDepthMask(GL_FALSE);

        // base selector folded into the cull (VISIBLE_SELECTED); this pass only adds the
        // effect axis (a cheap inline flag read) — no nested std::function call.
        // NOTE KI drawBlendedImpl already restricts to KIND_BLEND
        const std::function<bool(const render::DrawableInfo&)> isEffect =
            [](const render::DrawableInfo& d) { return d.m_flags.effect; };

        CollectionRender collectionRender;

        collectionRender.drawBlendedImpl(
            ctx,
            &isEffect,
            render::VISIBLE_ALL_SELECTED);

        ctx.m_batch->flush(ctx);

        state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_DEPTH_TEST, true);
    }
}
