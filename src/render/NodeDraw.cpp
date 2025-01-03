#include "NodeDraw.h"

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/Uniform.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "pool/TypeHandle.h"

#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"

#include "renderer/ParticleRenderer.h"
#include "renderer/DecalRenderer.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "render/Camera.h"
#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/DebugContext.h"

#include "size.h"

namespace {
    const std::vector<pool::NodeHandle> EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;
}

namespace render {
    //// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    //struct MeshTypeComparator {
    //    bool operator()(const mesh::MeshType* a, const mesh::MeshType* b) const {
    //        return a->m_handle.m_id < b->m_handle.m_id;
    //    }
    //};

    bool MeshTypeKey::operator<(const MeshTypeKey& o) const {
        const auto& a = m_typeHandle.toType();
        const auto& b = o.m_typeHandle.toType();

        return a->m_handle.m_id < b->m_handle.m_id;
    }

    NodeDraw::NodeDraw()
        : m_textureQuad{ render::TextureQuad::get() },
        m_screenTri{ render::ScreenTri::get() },
        m_particleRenderer{ std::make_unique<ParticleRenderer>(true) },
        m_decalRenderer{ std::make_unique < DecalRenderer>(true) }
    {}

    NodeDraw::~NodeDraw()
    {
    }

    void NodeDraw::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

        m_gBuffer.prepare();
        m_oitBuffer.prepare(&m_gBuffer);
        m_effectBuffer.prepare(&m_gBuffer);

        m_textureQuad.prepare();

        {
            std::map<std::string, std::string, std::less<>> definitions;

            size_t shadowCount = std::min(
                std::max(assets.shadowPlanes.size() - 1, static_cast<size_t>(1)),
                static_cast<size_t>(MAX_SHADOW_MAP_COUNT_ABS));

            definitions[DEF_MAX_SHADOW_MAP_COUNT] = std::to_string(shadowCount);

            m_deferredProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_DEFERRED_PASS, definitions));
            m_deferredProgram->prepareRT();
        }

        m_oitProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_OIT_PASS));
        m_oitBlendProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLEND_OIT_PASS));
        m_bloomInitProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLOOM_INIT_PASS));
        m_bloomBlurProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLOOM_BLUR_PASS));
        m_bloomFinalProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLOOM_FINAL_PASS));
        m_emissionProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_EMISSION_PASS));
        m_fogProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_FOG_PASS));
        // m_hdrGammaProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_HDR_GAMMA_PASS));

        m_timeElapsedQuery.create();

        m_particleRenderer->prepareRT(ctx);
        m_decalRenderer->prepareRT(ctx);

        m_effectBloomIterations = assets.effectBloomIterations;

        m_drawDebug = assets.drawDebug;
        m_glUseInvalidate = assets.glUseInvalidate;
        m_prepassDepthEnabled = assets.prepassDepthEnabled;
        m_effectOitEnabled = assets.effectOitEnabled;
        m_effectFogEnabled = assets.effectFogEnabled;
        m_effectBloomEnabled = assets.effectBloomEnabled;
    }

    void NodeDraw::updateRT(const UpdateViewContext& ctx)
    {
        m_gBuffer.updateRT(ctx);
        m_oitBuffer.updateRT(ctx);
        m_effectBuffer.updateRT(ctx);
    }

    void NodeDraw::drawNodes(
        const RenderContext& ctx,
        FrameBuffer* dstBuffer,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits,
        GLbitfield copyMask)
    {
        {
            const auto& dbg = *ctx.m_dbg;

            m_drawDebug = dbg.m_drawDebug;

            m_particleEnabled = dbg.m_particleEnabled;
            m_decalEnabled = dbg.m_decalEnabled;

            m_prepassDepthEnabled = dbg.m_prepassDepthEnabled;
            m_effectOitEnabled = dbg.m_effectOitEnabled;
            m_effectFogEnabled = dbg.m_effectFogEnabled;

            m_effectBloomEnabled = dbg.m_effectBloomEnabled;
            m_effectBloomIterations = dbg.m_effectBloomIterations;

            m_particleRenderer->setEnabled(m_particleEnabled);
            m_decalRenderer->setEnabled(m_decalEnabled);
        }

        // https://community.khronos.org/t/selectively-writing-to-buffers/71054
        auto* const targetBuffer = m_effectBuffer.m_primary.get();
        auto* const finalBuffer = m_effectBuffer.m_secondary.get();

        passDeferred(ctx, typeSelector, nodeSelector, kindBits, targetBuffer);
        passOit(ctx, typeSelector, nodeSelector, kindBits, targetBuffer);

        passForward(ctx, typeSelector, nodeSelector, kindBits, targetBuffer);
        passSkybox(ctx, targetBuffer);

        passDecal(ctx, targetBuffer);
        passParticle(ctx, targetBuffer);
        passEffect(ctx, typeSelector, nodeSelector, targetBuffer);

        passScreenspace(ctx, typeSelector, nodeSelector, kindBits, targetBuffer, finalBuffer);

        passCopy(ctx, finalBuffer, dstBuffer, copyMask);
        passDebug(ctx, dstBuffer);
        passCleanup(ctx);
    }

    void NodeDraw::passDeferred(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits,
        FrameBuffer* targetBuffer)
    {
        auto& state = ctx.m_state;

        // pass 1 - draw geometry
        // => nodes supporting G-buffer
        //if (false)
        state.setStencil({});

        m_gBuffer.m_buffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
        m_gBuffer.clearAll();
        m_gBuffer.bind(ctx);

        // NOTE KI no blend in G-buffer
        auto wasForceSolid = ctx.setForceSolid(true);

        passDeferredPreDepth(ctx, typeSelector, nodeSelector, kindBits);

        if (m_prepassDepthEnabled) {
            state.setDepthFunc(GL_LEQUAL);
        }

        passDeferredDraw(ctx, typeSelector, nodeSelector, kindBits);

        if (m_prepassDepthEnabled) {
            state.setDepthFunc(ctx.m_depthFunc);
        }

        ctx.setForceSolid(wasForceSolid);

        passDeferredCombine(ctx, targetBuffer);
    }

    void NodeDraw::passDeferredPreDepth(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits)
    {
        // NOTE KI "pre pass depth" causes more artifacts than benefits
        if (!m_prepassDepthEnabled) return;

        m_gBuffer.m_buffer->resetDrawBuffers(0);

        // NOTE KI only *solid* render in pre-pass
        {
            drawProgram(
                ctx,
                [this](const mesh::LodMesh& lodMesh) {
                    if (!lodMesh.m_flags.preDepth) return (ki::program_id)0;
                    return lodMesh.m_drawOptions.m_gbuffer ? lodMesh.m_preDepthProgramId : (ki::program_id)0;
                },
                [&typeSelector](const mesh::MeshType* type) {
                    return typeSelector(type);
                },
                nodeSelector,
                kindBits & render::KIND_SOLID);
        }

        auto flushedCount = ctx.m_batch->flush(ctx);
        if (flushedCount > 0) {
            //KI_INFO_OUT(fmt::format("PRE_PASS: count={}", flushedCount));
        }
        m_gBuffer.m_buffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
    }

    void NodeDraw::passDeferredDraw(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits)
    {
        auto& state = ctx.m_state;

        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

        drawNodesImpl(
            ctx,
            [](const mesh::LodMesh& lodMesh) {
                return lodMesh.m_drawOptions.m_gbuffer ? lodMesh.m_programId : (ki::program_id)0;
            },
            [&typeSelector](const mesh::MeshType* type) {
                return !type->m_flags.effect && typeSelector(type);
            },
            nodeSelector,
            kindBits);

        ctx.m_batch->flush(ctx);
    }

    void NodeDraw::passDeferredCombine(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        // pass 2 - target effectBuffer
        {
            targetBuffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
            targetBuffer->clearAll();
        }

        m_gBuffer.m_buffer->copy(
            m_gBuffer.m_depthTexture.get(),
            GBuffer::ATT_DEPTH_INDEX);

        m_gBuffer.bindTexture(ctx);

        auto& state = ctx.m_state;
        state.setStencil(kigl::GLStencilMode::only_non_zero());
        state.setEnabled(GL_DEPTH_TEST, false);
        state.polygonFrontAndBack(GL_FILL);
        state.frontFace(GL_CCW);

        targetBuffer->bind(ctx);

        m_deferredProgram->bind();
        m_screenTri.draw();

        targetBuffer->resetDrawBuffers(1);

        state.setEnabled(GL_DEPTH_TEST, true);
    }

    void NodeDraw::passOit(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits,
        FrameBuffer* targetBuffer)
    {
        // pass 5 - OIT
        // NOTE KI OIT after *forward* pass to allow using depth texture from them
        if (ctx.m_forceSolid) return;
        if (!m_effectOitEnabled) return;

        auto& state = ctx.m_state;

        state.setStencil(kigl::GLStencilMode::fill(STENCIL_OIT | STENCIL_FOG));
        // NOTE KI do NOT modify depth with blend
        state.setDepthMask(GL_FALSE);
        state.setEnabled(GL_BLEND, true);

        m_oitBuffer.clearAll();
        m_oitBuffer.bind(ctx);

        // NOTE KI different blend mode for each draw buffer
        glBlendFunci(0, GL_ONE, GL_ONE);
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glBlendEquation(GL_FUNC_ADD);

        // only "blend OIT" nodes
        drawNodesImpl(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (!lodMesh.m_drawOptions.m_gbuffer) return (ki::program_id)0;
                if (lodMesh.m_oitProgramId) return lodMesh.m_oitProgramId;
                return m_oitProgram->m_id;
            },
            [&typeSelector](const mesh::MeshType* type) {
                return typeSelector(type);
            },
            nodeSelector,
            kindBits& render::KIND_BLEND);

        ctx.m_batch->flush(ctx);

        {
            // NOTE KI *MUST* reset blend mode (especially for attachment 1)
            // ex. if not done OIT vs. bloom works strangely
            glBlendFunci(0, GL_ONE, GL_ONE);
            glBlendFunci(1, GL_ONE, GL_ONE);
            state.invalidateBlendMode();

            state.setEnabled(GL_BLEND, false);
            state.setDepthMask(GL_TRUE);
        }
    }

    void NodeDraw::passSkybox(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        auto& state = ctx.m_state;

        targetBuffer->bind(ctx);

        // pass 6 - skybox (*before* blend)
        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SKYBOX, STENCIL_SKYBOX, ~STENCIL_OIT));
        drawSkybox(ctx);
    }

    // pass 7 - blend effects
    // => separate light calculations
    void NodeDraw::passEffect(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        FrameBuffer* targetBuffer)
    {
        if (ctx.m_forceSolid) return;

        auto& state = ctx.m_state;

        state.setStencil({});
        state.setDepthMask(GL_FALSE);

        drawBlendedImpl(
            ctx,
            [&typeSelector](const mesh::MeshType* type) {
                return
                    type->m_flags.anyBlend &&
                    type->m_flags.effect &&
                    typeSelector(type);
            },
            nodeSelector);
        ctx.m_batch->flush(ctx);

        state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_DEPTH_TEST, true);
    }

    void NodeDraw::passDecal(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        if (ctx.m_forceSolid) return;
        if (!ctx.m_useDecals || !m_decalEnabled) return;

        auto& state = ctx.m_state;

        state.setStencil({});
        targetBuffer->bind(ctx);
        m_decalRenderer->renderBlend(ctx);
    }

    void NodeDraw::passParticle(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        if (ctx.m_forceSolid) return;
        if (!ctx.m_useParticles || !m_particleEnabled) return;

        auto& state = ctx.m_state;

        state.setStencil({});
        targetBuffer->bind(ctx);
        m_particleRenderer->render(ctx);
    }

    void NodeDraw::passForward(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits,
        FrameBuffer* targetBuffer)
    {
        // pass 4 - non G-buffer solid nodes
        // => separate light calculations
        // => currently these *CANNOT* work correctly
        ctx.validateRender("non_gbuffer");

        auto& state = ctx.m_state;
        state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

        targetBuffer->bind(ctx);

        drawNodesImpl(
            ctx,
            [](const mesh::LodMesh& lodMesh) {
                return !lodMesh.m_drawOptions.m_blend && !lodMesh.m_drawOptions.m_gbuffer
                    ? lodMesh.m_programId
                    : (ki::program_id)0;
            },
            [&typeSelector](const mesh::MeshType* type) { return typeSelector(type); },
            nodeSelector,
            // NOTE KI no blended
            kindBits & ~render::KIND_BLEND);

        auto flushedCount = ctx.m_batch->flush(ctx);
        if (flushedCount > 0) {
            // NOTE KI depth again if changes; FOG is broken without this
            m_gBuffer.m_buffer->copy(
                m_gBuffer.m_depthTexture.get(),
                GBuffer::ATT_DEPTH_INDEX);

            // NOTE KI need to reset possibly changed drawing modes
            // ex. selection volume changes to GL_LINE
            ctx.bindDefaults();
        }
    }

    // pass 8 - screenspace effects
    void NodeDraw::passScreenspace(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits,
        FrameBuffer* srcBuffer,
        FrameBuffer* finalBuffer)
    {
        if (ctx.m_forceSolid || !ctx.m_useScreenspaceEffects) {
            srcBuffer->copy(
                finalBuffer,
                EffectBuffer::ATT_ALBEDO_INDEX,
                EffectBuffer::ATT_ALBEDO_INDEX);
            return;
        }

        auto& state = ctx.m_state;

        state.setEnabled(GL_DEPTH_TEST, false);
        // NOTE KI do NOT modify depth with blend (likely redundant)
        state.setDepthMask(GL_FALSE);
        state.frontFace(GL_CCW);
        state.polygonFrontAndBack(GL_FILL);

        {
            state.setEnabled(GL_BLEND, true);

            passFogBlend(ctx, srcBuffer);
            passOitBlend(ctx, srcBuffer);

            glBlendFunci(0, GL_ONE, GL_ONE);
            glBlendFunci(1, GL_ONE, GL_ONE);
            state.invalidateBlendMode();

            state.setEnabled(GL_BLEND, false);
        }

        state.setStencil({});

        passBloom(ctx, srcBuffer, finalBuffer);

        state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_DEPTH_TEST, true);

        state.setStencil({});
    }

    void NodeDraw::passFogBlend(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        if (!m_effectFogEnabled) return;
        if (!ctx.m_useFog) return;

        auto& state = ctx.m_state;
        state.setStencil(kigl::GLStencilMode::only(STENCIL_FOG, STENCIL_FOG));
        state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        targetBuffer->bind(ctx);
        m_fogProgram->bind();
        m_screenTri.draw();
    }

    void NodeDraw::passOitBlend(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        if (!m_effectOitEnabled) return;

        auto& state = ctx.m_state;
        state.setStencil(kigl::GLStencilMode::only_non_zero(STENCIL_OIT));
        state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        targetBuffer->bind(ctx);
        m_oitBlendProgram->bind();
        m_oitBuffer.bindTexture(ctx);

        m_screenTri.draw();
    }

    void NodeDraw::passBloom(
        const RenderContext& ctx,
        FrameBuffer* srcBuffer,
        FrameBuffer* finalBuffer)
    {
        auto& state = ctx.m_state;

        if (!m_effectBloomEnabled || !ctx.m_useBloom)
        {
            srcBuffer->copy(
                finalBuffer,
                EffectBuffer::ATT_ALBEDO_INDEX,
                EffectBuffer::ATT_ALBEDO_INDEX);
            return;
        }

        for (auto& buffer : m_effectBuffer.m_buffers) {
            buffer->clearAll();
        }

        {
            srcBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);

            auto& buf = m_effectBuffer.m_buffers[1];
            buf->bind(ctx);
            buf->bindTexture(ctx, EffectBuffer::ATT_WORK_INDEX, UNIT_EFFECT_WORK);

            m_bloomInitProgram->bind();
            m_screenTri.draw();
        }

        {
            //srcBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);
            //srcBuffer->bindTexture(ctx, EffectBuffer::ATT_BRIGHT_INDEX, UNIT_EFFECT_BRIGHT);

            //m_emissionProgram->bind();
            //m_screenTri.draw(ctx);
        }

        m_bloomBlurProgram->bind();

        bool horizontal = false;
        for (int i = 0; i < m_effectBloomIterations; i++) {
            auto& buf = m_effectBuffer.m_buffers[i % 2];
            buf->bind(ctx);

            m_bloomBlurProgram->m_uniforms->u_effectBloomHorizontal.set(horizontal);
            m_screenTri.draw();

            buf->bindTexture(ctx, EffectBuffer::ATT_WORK_INDEX, UNIT_EFFECT_WORK);
            horizontal = !horizontal;
        }

        {
            finalBuffer->clearAll();
            finalBuffer->bind(ctx);

            srcBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);

            m_bloomFinalProgram->bind();
            m_screenTri.draw();
        }
    }

    void NodeDraw::passCopy(
        const RenderContext& ctx,
        FrameBuffer* srcBuffer,
        FrameBuffer* dstBuffer,
        GLbitfield copyMask)
    {
        // pass 10 - render to target
        // NOTE KI binding target buffer should be 100% redundant here
        // i.e. blit does not need it
        // NOTE KI *Exception* CubeMapBuffer::bind logic
        // => cubemap is *NOT* exception any longer
        //dstBuffer->bind(ctx);

        if (copyMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
            m_gBuffer.m_buffer->blit(
                dstBuffer,
                copyMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT),
                { -1.f, 1.f },
                { 2.f, 2.f },
                GL_NEAREST);
        }

        if (copyMask & GL_COLOR_BUFFER_BIT) {
            GLenum sourceFormat = srcBuffer->m_spec.attachments[EffectBuffer::ATT_ALBEDO_INDEX].internalFormat;
            GLenum targetFormat = -1;

            if (!dstBuffer->m_spec.attachments.empty()) {
                targetFormat = dstBuffer->m_spec.attachments[EffectBuffer::ATT_ALBEDO_INDEX].internalFormat;
            }

            const bool canCopy = !dstBuffer->m_spec.attachments.empty() &&
                dstBuffer->m_spec.width == srcBuffer->m_spec.width &&
                dstBuffer->m_spec.height == srcBuffer->m_spec.height &&
                targetFormat == sourceFormat;

            if (canCopy) {
                srcBuffer->copy(
                    dstBuffer,
                    EffectBuffer::ATT_ALBEDO_INDEX,
                    // NOTE KI assumption; buffer is at index 0
                    EffectBuffer::ATT_ALBEDO_INDEX);
            }
            else {
                srcBuffer->blit(
                    dstBuffer,
                    GL_COLOR_BUFFER_BIT,
                    GL_COLOR_ATTACHMENT0,
                    GL_COLOR_ATTACHMENT0,
                    { -1.f, 1.f },
                    { 2.f, 2.f },
                    GL_LINEAR);
            }
        }
    }

    void NodeDraw::passCleanup(
        const RenderContext& ctx)
    {
        // pass 12 - cleanup
        if (!m_glUseInvalidate) return;

        //kigl::GLState::get().bindFrameBuffer(0, false);

        if (m_effectOitEnabled) {
            m_oitBuffer.invalidateAll();
        }
        m_effectBuffer.invalidateAll();
        m_gBuffer.invalidateAll();
    }

    void NodeDraw::passDebug(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        // pass 11 - debug info
        if (!(ctx.m_allowDrawDebug && m_drawDebug)) return;

        //m_effectBuffer.m_primary->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

        constexpr float SZ1 = 0.25f;
        //constexpr float SZ2 = 0.5f;

        size_t count = 0;
        float padding = 0.f;
        for (int i = 0; i < m_oitBuffer.m_buffer->getDrawBufferCount(); i++) {
            m_oitBuffer.m_buffer->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0 + i,
                GL_COLOR_ATTACHMENT0,
                { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
                GL_NEAREST);
            padding = 0.f;
        }
        count += m_oitBuffer.m_buffer->getDrawBufferCount();

        padding = 0.01f;
        for (int i = 0; i < m_effectBuffer.m_primary->getDrawBufferCount(); i++) {
            m_effectBuffer.m_primary->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0 + i,
                GL_COLOR_ATTACHMENT0,
                { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
                GL_NEAREST);
            padding = 0.f;
        }
        count += m_effectBuffer.m_primary->getDrawBufferCount();

        padding = 0.01f;
        for (int i = 0; i < m_effectBuffer.m_secondary->getDrawBufferCount(); i++) {
            m_effectBuffer.m_secondary->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0 + i,
                GL_COLOR_ATTACHMENT0,
                { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
                GL_NEAREST);
            padding = 0.f;
        }
        count += m_effectBuffer.m_secondary->getDrawBufferCount();

        padding = 0.02f;
        for (int i = 0; i < m_effectBuffer.m_buffers.size(); i++) {
            auto& buf = m_effectBuffer.m_buffers[i];
            buf->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT0,
                { -1.f, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
                GL_NEAREST);
            padding = 0.f;
        }
        count += m_effectBuffer.m_buffers.size();

        count = 0;
        padding = 0.f;
        for (int i = 0; i < m_gBuffer.m_buffer->getDrawBufferCount(); i++) {
            m_gBuffer.m_buffer->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0 + i,
                GL_COLOR_ATTACHMENT0,
                { 1 - SZ1, -1 + padding + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
                GL_NEAREST);
            padding = 0.f;
        }
    }

    void NodeDraw::drawProgram(
        const RenderContext& ctx,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits)
    {
        drawNodesImpl(ctx, programSelector, typeSelector, nodeSelector, kindBits);
    }

    bool NodeDraw::drawNodesImpl(
        const RenderContext& ctx,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        const uint8_t kindBits)
    {
        bool rendered{ false };

        auto& collection = *ctx.m_collection;
        auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

        auto renderTypes = [this, &ctx, &programSelector, &typeSelector, &nodeSelector, &rendered](
            const MeshTypeMap& typeMap,
            unsigned int kind)
        {
            for (const auto& it : typeMap) {
                auto* type = it.first.m_typeHandle.toType();

                if (!type->isReady()) continue;
                if (type->m_layer != ctx.m_layer) continue;
                if (!typeSelector(type)) continue;

                auto& batch = ctx.m_batch;

                for (auto& handle : it.second) {
                    auto* node = handle.toNode();
                    if (!node || !nodeSelector(node)) continue;

                    rendered = true;
                    batch->draw(ctx, type, programSelector, kind, *node);
                }
            }
        };

        if (kindBits & render::KIND_SOLID) {
            renderTypes(collection.m_solidNodes, render::KIND_SOLID);
        }

        if (kindBits & render::KIND_ALPHA) {
            renderTypes(collection.m_alphaNodes, render::KIND_ALPHA);
        }

        if (kindBits & render::KIND_BLEND) {
            renderTypes(collection.m_blendedNodes, render::KIND_BLEND);
        }

        return rendered;
    }

    void NodeDraw::drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector)
    {
        auto& collection = *ctx.m_collection;

        if (collection.m_blendedNodes.empty()) return;

        const glm::vec3& eyePos = ctx.m_camera->getWorldPosition();

        // TODO KI discards nodes if *same* distance
        std::map<float, Node*> sorted;
        for (const auto& map : collection.m_blendedNodes) {
            auto* type = map.first.m_typeHandle.toType();

            if (!type->isReady()) continue;
            if (type->m_layer != ctx.m_layer) continue;
            if (!typeSelector(type)) continue;

            for (const auto& handle : map.second) {
                auto* node = handle.toNode();
                if (!node || !nodeSelector(node)) continue;

                const auto* snapshot = node->getSnapshotRT();
                if (!snapshot) continue;

                const auto& pos = snapshot->getWorldPosition();
                const float dist2 = glm::distance2(eyePos, pos);
                sorted[dist2] = node;
            }
        }

        if (!sorted.empty()) {
            //glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glFlush();
            //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
            //glFinish();
        }

        // NOTE KI blending is *NOT* optimal program / nodetypw wise due to depth sorting
        // NOTE KI order = from furthest away to nearest
        for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            auto* node = it->second;
            auto* type = node->m_typeHandle.toType();

            ctx.m_batch->draw(
                ctx,
                type,
                [this](const mesh::LodMesh& lodMesh) { return lodMesh.m_programId; },
                render::KIND_BLEND,
                *node);
        }

        // TODO KI if no flush here then render order of blended nodes is incorrect
        //ctx.m_batch->flush(ctx);
    }

    void NodeDraw::drawSkybox(
        const RenderContext& ctx)
    {
        auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
        auto* node = nodeRegistry.m_skybox.toNode();
        if (!node) return;

        auto* type = node->m_typeHandle.toType();

        if (!type->isReady()) return;
        if (type->m_layer != ctx.m_layer) return;

        auto& batch = ctx.m_batch;

        auto* lodMesh = type->getLodMesh(0);
        auto* program = Program::get(lodMesh->m_programId);

        auto& state = ctx.m_state;
        state.setDepthFunc(GL_LEQUAL);
        state.frontFace(GL_CCW);

        program->bind();
        m_textureQuad.draw();
        state.setDepthFunc(ctx.m_depthFunc);
    }
}
