#include "PassForward.h"

#include "kigl/GLState.h"

#include "model/Node.h"

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

    void PassForward::updateRT(
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

        auto& state = ctx.m_state;
        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

        CollectionRender collectionRender;
        collectionRender.drawNodesImpl(
            ctx,
            [](const mesh::LodMesh& lodMesh) {
                return !lodMesh.m_drawOptions.isBlend() && !lodMesh.m_drawOptions.m_gbuffer
                    ? lodMesh.m_programId
                    : (ki::program_id)0;
            },
            [](ki::program_id programId) {},
            [&drawContext](const Node* node) {
                return node->m_typeFlags.useForward &&
                    drawContext.nodeSelector(node);
            },
            // NOTE KI no blended
            drawContext.kindBits & ~render::KIND_BLEND);

        auto flushedCount = ctx.m_batch->flush(ctx);
        if (flushedCount > 0) {
            // NOTE KI depth again if changes; FOG is broken without this
            //m_gBuffer.m_buffer->copy(
            //    m_gBuffer.m_depthTexture.get(),
            //    GBuffer::ATT_DEPTH_INDEX);

        }
        // NOTE KI need to reset possibly changed drawing modes
        // ex. selection volume changes to GL_LINE
        ctx.bindDefaults();
    }
}
