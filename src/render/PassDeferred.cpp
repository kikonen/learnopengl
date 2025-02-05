#include "PassDeferred.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "mesh/MeshType.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/NodeDraw.h"
#include "render/Batch.h"
#include "render/CollectionRender.h"

namespace {
    const std::string SHADER_DEFERRED_PASS{ "g_deferred_pass" };
}

namespace render
{
    PassDeferred::PassDeferred()
        : Pass("PassDeferred")
    {
    }

    PassDeferred::~PassDeferred() = default;

    void PassDeferred::prepare(const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;

        m_gBuffer.prepare();

        {
            std::map<std::string, std::string, std::less<>> definitions;

            size_t shadowCount = std::min(
                std::max(assets.shadowPlanes.size() - 1, static_cast<size_t>(1)),
                static_cast<size_t>(MAX_SHADOW_MAP_COUNT_ABS));

            definitions[DEF_MAX_SHADOW_MAP_COUNT] = std::to_string(shadowCount);

            m_combineProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_DEFERRED_PASS, definitions));
            m_combineProgram->prepareRT();
        }
    }

    void PassDeferred::updateRT(const UpdateViewContext& ctx)
    {
        if (!updateSize(ctx)) return;

        m_gBuffer.updateRT(ctx);

        // NOTE KI this is start of all whole chain (i.e. after GBuffer pass)
        {
            auto buffer = new FrameBuffer(
                m_name,
                {
                    m_width, m_height,
                    {
                        FrameBufferAttachment::getEffectTextureHdr(ATT_ALBEDO_ENUM),
                        FrameBufferAttachment::getShared(m_gBuffer.m_buffer->getDepthAttachment()),
                    }
                });

            m_buffer.reset(buffer);
            m_buffer->prepare();
        }
    }

    void PassDeferred::cleanup(const RenderContext& ctx)
    {
        m_gBuffer.invalidateAll();
    }

    void PassDeferred::initRender(
        const RenderContext& ctx)
    {
        auto& state = ctx.m_state;
        const auto& dbg = *ctx.m_dbg;

        state.setStencil({});

        m_gBuffer.m_buffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
        m_gBuffer.clearAll();

        m_buffer->clearAll();

        m_preDepthEnabled = dbg.m_prepassDepthEnabled;
    }

    PassContext PassDeferred::preDepth(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_preDepthEnabled) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        {
            auto wasForceSolid = ctx.setForceSolid(true);
            passPreDepth(ctx, drawContext);
            ctx.setForceSolid(wasForceSolid);
        }

        return src;
    }

    PassContext PassDeferred::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        auto& state = ctx.m_state;
        state.setStencil({});

        if (m_preDepthEnabled) {
            state.setDepthFunc(GL_LEQUAL);
        }

        {
            auto wasForceSolid = ctx.setForceSolid(true);
            passDraw(ctx, drawContext);
            ctx.setForceSolid(wasForceSolid);
        }

        if (m_preDepthEnabled) {
            state.setDepthFunc(ctx.m_depthFunc);
        }

        return src;
    }

    void PassDeferred::passPreDepth(
        const RenderContext& ctx,
        const DrawContext& drawContext)
    {
        m_gBuffer.m_buffer->resetDrawBuffers(0);
        m_gBuffer.bind(ctx);

        // NOTE KI only *solid* render in pre-pass
        CollectionRender collectionRender;
        collectionRender.drawProgram(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (!lodMesh.m_flags.preDepth) return (ki::program_id)0;
                return lodMesh.m_drawOptions.m_gbuffer ? lodMesh.m_preDepthProgramId : (ki::program_id)0;
            },
            [&drawContext](const mesh::MeshType* type) {
                return drawContext.typeSelector(type);
            },
            drawContext.nodeSelector,
            drawContext.kindBits & render::KIND_SOLID);

        //auto flushedCount = ctx.m_batch->flush(ctx);
        //if (flushedCount > 0) {
        //    //KI_INFO_OUT(fmt::format("PRE_PASS: count={}", flushedCount));
        //}
    }

    void PassDeferred::passDraw(
        const RenderContext& ctx,
        const DrawContext& drawContext)
    {
        auto& state = ctx.m_state;

        m_gBuffer.m_buffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
        m_gBuffer.bind(ctx);

        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

        CollectionRender collectionRender;
        collectionRender.drawNodesImpl(
            ctx,
            [](const mesh::LodMesh& lodMesh) {
                return lodMesh.m_drawOptions.m_gbuffer ? lodMesh.m_programId : (ki::program_id)0;
            },
            [&drawContext](const mesh::MeshType* type) {
                return !type->m_flags.effect && drawContext.typeSelector(type);
            },
            drawContext.nodeSelector,
            drawContext.kindBits);

        ctx.m_batch->flush(ctx);
    }

    PassContext PassDeferred::combine(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        const auto& dbg = *ctx.m_dbg;

        {
            m_gBuffer.bindTexture(ctx);
            m_gBuffer.m_buffer->copy(
                m_gBuffer.m_depthTexture.get(),
                GBuffer::ATT_DEPTH_INDEX);
        }

        startScreenPass(
            ctx,
            true,
            kigl::GLStencilMode::only_non_zero(),
            false,
            {});

        m_buffer->bind(ctx);
        m_combineProgram->bind();
        m_screenTri.draw();

        stopScreenPass(ctx);


        return { m_buffer.get(), ATT_ALBEDO_INDEX };
    }
}
