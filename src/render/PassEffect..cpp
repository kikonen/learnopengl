#include "PassEffect.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "mesh/MeshType.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"
#include "render/CollectionRender.h"

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

    void PassEffect::updateRT(const UpdateViewContext& ctx)
    {
        if (!updateSize(ctx)) return;
    }

    void PassEffect::initRender(const RenderContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

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
        auto& state = ctx.m_state;

        state.setStencil({});
        state.setDepthMask(GL_FALSE);

        CollectionRender collectionRender;

        collectionRender.drawBlendedImpl(
            ctx,
            [&drawContext](const mesh::MeshType* type) {
                return
                    type->m_flags.anyBlend &&
                    type->m_flags.effect &&
                    drawContext.typeSelector(type);
            },
            drawContext.nodeSelector);

        ctx.m_batch->flush(ctx);

        state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_DEPTH_TEST, true);
    }
}
