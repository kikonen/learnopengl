#include "PassForward.h"

#include "kigl/GLState.h"

#include "mesh/LodMesh.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

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

    void PassForward::updateRT(const UpdateViewContext& ctx)
    {
        if (!updateSize(ctx)) return;
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

        auto& state = ctx.m_state;
        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

        CollectionRender collectionRender;
        collectionRender.drawNodesImpl(
            ctx,
            [](const mesh::LodMesh& lodMesh) {
                return !lodMesh.m_drawOptions.m_blend && !lodMesh.m_drawOptions.m_gbuffer
                    ? lodMesh.m_programId
                    : (ki::program_id)0;
            },
            [&drawContext](const mesh::MeshType* type) { return drawContext.typeSelector(type); },
            drawContext.nodeSelector,
            // NOTE KI no blended
            drawContext.kindBits & ~render::KIND_BLEND);

        ctx.m_batch->flush(ctx);
    }
}
