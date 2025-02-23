#include "PassOit.h"

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
#include "render/PassDeferred.h"

namespace {
}

namespace render
{
    PassOit::PassOit()
        : Pass("PassOit")
    {
    }

    PassOit::~PassOit() = default;

    void PassOit::prepare(const PrepareContext& ctx)
    {
        m_oitProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_OIT_PASS));
        m_oitBlendProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLEND_OIT_PASS));

        m_oitBuffer.prepare();
    }

    void PassOit::updateRT(
        const UpdateViewContext& ctx,
        PassDeferred* passDeferred,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;

        m_oitBuffer.updateRT(ctx, passDeferred->getGBuffer(), bufferScale);
    }

    void PassOit::cleanup(const RenderContext& ctx)
    {
        m_oitBuffer.invalidateAll();
    }

    void PassOit::initRender(const RenderContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

        m_oitBuffer.clearAll();

        m_enabled = !(ctx.m_forceSolid) &&
            dbg.m_effectOitEnabled;

        m_flushedCount = 0;
    }

    PassContext PassOit::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        passOit(ctx, drawContext);

        return src;
    }

    PassContext PassOit::blend(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;
        if (m_flushedCount == 0) return src;

        startScreenPass(
            ctx,
            true,
            kigl::GLStencilMode::only_non_zero(STENCIL_OIT),
            true,
            { GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        m_oitBlendProgram->bind();
        m_oitBuffer.bindTexture(ctx.m_state);

        src.buffer->bind(ctx);

        m_screenTri.draw();

        stopScreenPass(ctx);

        return src;
    }

    void PassOit::passOit(
        const RenderContext& ctx,
        const DrawContext& drawContext)
    {
        auto& state = ctx.m_state;

        {
            state.setStencil(kigl::GLStencilMode::fill(STENCIL_OIT | STENCIL_FOG));

            // NOTE KI *use depth* but do NOT modify depth with blend
            state.setDepthMask(GL_FALSE);
            state.setDepthFunc(GL_LESS);

            state.polygonOffset({ 0.25f, 0.15f });
            state.setEnabled(GL_POLYGON_OFFSET_FILL, true);

            state.setEnabled(GL_BLEND, true);
            // NOTE KI different blend mode for each draw buffer
            state.setBlendMode(0, { GL_ONE, GL_ONE });
            state.setBlendMode(1, { GL_ZERO, GL_ONE_MINUS_SRC_COLOR });
            // NOTE KI no alpha for emission; override existing value
            state.setBlendMode(2, { GL_ONE, GL_ZERO });
        }

        m_oitBuffer.bind(ctx);

        // only "blend OIT" nodes
        CollectionRender collectionRender;
        collectionRender.drawNodesImpl(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (!lodMesh.m_drawOptions.m_gbuffer) return (ki::program_id)0;
                if (lodMesh.m_oitProgramId) return lodMesh.m_oitProgramId;
                return m_oitProgram->m_id;
            },
            [&drawContext](const mesh::MeshType* type) {
                return drawContext.typeSelector(type);
            },
            drawContext.nodeSelector,
            drawContext.kindBits & render::KIND_BLEND);

        m_flushedCount = ctx.m_batch->flush(ctx);

        {
            // NOTE KI *MUST* reset blend mode (especially for attachment 1)
            // ex. if not done OIT vs. bloom works strangely
            state.setBlendMode(0, {});
            state.setBlendMode(1, {});
            state.setBlendMode(2, {});
            state.invalidateBlendMode();

            state.setEnabled(GL_BLEND, false);
            state.setEnabled(GL_POLYGON_OFFSET_FILL, false);
            state.setDepthMask(GL_TRUE);
            state.setDepthFunc(ctx.m_depthFunc);
        }
    }
}
