#include "PassForward.h"

#include "kigl/GLState.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"

#include "shader/Shader.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"
#include "render/DrawableInfo.h"

namespace render
{
    PassForward::PassForward()
        : Pass("PassForward")
    {
    }

    PassForward::~PassForward() = default;

    void PassForward::prepare(const PrepareContext& ctx)
    {
    }

    void PassForward::updateView(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassForward::initRender(const RenderContext& ctx)
    {
        m_enabled = true;
    }

    PassContext PassForward::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        src.buffer->bind(ctx);

        passForward(ctx, drawContext);

        // NOTE KI need to reset possibly changed drawing modes
        // ex. selection volume changes to GL_LINE
        //ctx.bindDefaults();

        return src;
    }

    void PassForward::passForward(
        const RenderContext& ctx,
        const DrawContext& drawContext)
    {
        // pass 4 - non G-buffer solid nodes
        // => separate light calculations
        // => currently these *CANNOT* work correctly
        ctx.validateRender("non_gbuffer");

        auto& state = ctx.getGLState();
        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

        CollectionRender collectionRender;
        // NOTE KI the forward route's alpha bucket also contains BLEND (effect) drawables
        // (blend is a subset of alpha), and those are drawn by PassEffect / drawBlendedImpl.
        // Not requesting KIND_BLEND skips the blend *bucket*, but the alpha bucket still yields
        // them, so the selector must explicitly exclude blend here.
        collectionRender.drawProgram(
            ctx,
            [](const render::DrawableInfo& drawable) {
                if (drawable.drawOptions.isBlend()) return (ki::program_id)0;
                return drawable.programId;
            },
            // base selector folded into the cull (VISIBLE_SELECTED); no per-drawable call
            nullptr,
            drawContext.kindBits & ~render::KIND_BLEND,
            render::ROUTE_FORWARD,
            render::VISIBLE_ALL_SELECTED);

        auto flushedCount = ctx.m_batch->flush(ctx);
        if (flushedCount > 0) {
            // NOTE KI depth again if changes; FOG is broken without this
            //m_gBuffer.m_buffer->copy(
            //    m_gBuffer.m_depthTexture.get(),
            //    GBuffer::ATT_DEPTH_INDEX);

        }

        state.setStencil({});

        // NOTE KI need to reset possibly changed drawing modes
        // ex. selection volume changes to GL_LINE
        ctx.bindDefaults();
    }
}
