#include "NodeDraw.h"

#include "asset/Program.h"
#include "asset/ProgramUniforms.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "component/Camera.h"

#include "mesh/MeshType.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/Batch.h"

namespace {
    const NodeVector EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;
}

namespace render {
    // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    struct MeshTypeComparator {
        bool operator()(const mesh::MeshType* a, const mesh::MeshType* b) const {
            if (a->m_drawOptions < b->m_drawOptions) return true;
            else if (b->m_drawOptions < a->m_drawOptions) return false;
            return a->m_id < b->m_id;
        }
    };

    // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    MeshTypeKey::MeshTypeKey(const mesh::MeshType* type)
        : type(type)
    {}

    bool MeshTypeKey::operator<(const MeshTypeKey& o) const {
        const auto& a = type;
        const auto& b = o.type;
        if (a->m_drawOptions < b->m_drawOptions) return true;
        else if (b->m_drawOptions < a->m_drawOptions) return false;
        return a->m_id < b->m_id;
    }

    NodeDraw::NodeDraw()
    {}

    NodeDraw::~NodeDraw()
    {
        m_solidNodes.clear();
        m_blendedNodes.clear();
        m_invisibleNodes.clear();
    }

    void NodeDraw::prepareRT(
        const PrepareContext& ctx)
    {
        auto& registry = ctx.m_registry;
        auto& assets = ctx.m_assets;

        m_gBuffer.prepare(assets);
        m_oitBuffer.prepare(assets, &m_gBuffer);
        m_effectBuffer.prepare(assets, &m_gBuffer);

        m_plainQuad.prepare();
        m_textureQuad.prepare();

        m_deferredProgram = registry->m_programRegistry->getProgram(SHADER_DEFERRED_PASS);
        m_deferredProgram->prepareRT(assets);

        m_oitProgram = registry->m_programRegistry->getProgram(SHADER_OIT_PASS);
        m_oitProgram->prepareRT(assets);

        m_blendOitProgram = registry->m_programRegistry->getProgram(SHADER_BLEND_OIT_PASS);
        m_blendOitProgram->prepareRT(assets);

        m_bloomProgram = registry->m_programRegistry->getProgram(SHADER_BLOOM_PASS);
        m_bloomProgram->prepareRT(assets);

        m_blendBloomProgram = registry->m_programRegistry->getProgram(SHADER_BLEND_BLOOM_PASS);
        m_blendBloomProgram->prepareRT(assets);

        m_emissionProgram = registry->m_programRegistry->getProgram(SHADER_EMISSION_PASS);
        m_emissionProgram->prepareRT(assets);

        m_fogProgram = registry->m_programRegistry->getProgram(SHADER_FOG_PASS);
        m_fogProgram->prepareRT(assets);

        m_hdrGammaProgram = registry->m_programRegistry->getProgram(SHADER_HDR_GAMMA_PASS);
        m_hdrGammaProgram->prepareRT(assets);

        m_timeElapsedQuery.create();
    }

    void NodeDraw::updateRT(const UpdateViewContext& ctx)
    {
        m_gBuffer.updateRT(ctx);
        m_oitBuffer.updateRT(ctx);
        m_effectBuffer.updateRT(ctx);
    }

    void NodeDraw::handleNodeAdded(Node* node)
    {
        const auto* type = node->m_type;
        auto* program = type->m_program;

        if (type->m_entityType != mesh::EntityType::origo) {
            assert(program);
            if (!program) return;
        }

        {
            auto* map = &m_solidNodes;

            if (type->m_flags.alpha)
                map = &m_alphaNodes;

            if (type->m_flags.blend)
                map = &m_blendedNodes;

            if (type->m_entityType == mesh::EntityType::sprite)
                map = &m_spriteNodes;

            if (type->m_flags.invisible)
                map = &m_invisibleNodes;

            if (map) {
                // NOTE KI more optimal to not switch between culling mode (=> group by it)
                const ProgramKey programKey(
                    program ? program->m_id : NULL_PROGRAM_ID,
                    // NOTE KI *NEGATE* for std::tie
                    -type->m_priority,
                    type->getDrawOptions());

                const MeshTypeKey typeKey(type);

                auto& vTyped = (*map)[programKey][typeKey];
                insertNode(vTyped, node);
            }
        }
    }

    void NodeDraw::drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        unsigned int kindBits,
        GLbitfield copyMask)
    {
        //m_timeElapsedQuery.begin();

        // https://community.khronos.org/t/selectively-writing-to-buffers/71054
        auto* primaryBuffer = m_effectBuffer.m_primary.get();
        auto* secondaryBuffer = m_effectBuffer.m_secondary.get();

        // pass 1 - draw geometry
        // => nodes supporting G-buffer
        //if (false)
        {
            ctx.m_state.setStencil({});

            // NOTE KI intel requires FBO to be bound to clearing draw buffers
            // (nvidia seemingly does not)
            m_gBuffer.bind(ctx);
            m_gBuffer.clearAll();

            // NOTE KI no blend in G-buffer
            auto oldAllowBlend = ctx.setAllowBlend(false);

            // NOTE KI "pre pass depth" causes more artifacts than benefits
            if (ctx.m_assets.prepassDepthEnabled)
            {
                m_gBuffer.m_buffer->resetDrawBuffers(0);

                // NOTE KI only *solid* render in pre-pass
                {
                    ctx.m_nodeDraw->drawProgram(
                        ctx,
                        [this](const mesh::MeshType* type) { return type->m_preDepthProgram; },
                        [&typeSelector](const mesh::MeshType* type) {
                            return type->m_flags.gbuffer &&
                                type->m_flags.preDepth &&
                                typeSelector(type);
                        },
                        nodeSelector,
                        kindBits & NodeDraw::KIND_SOLID);
                }

                ctx.m_batch->flush(ctx);
                m_gBuffer.m_buffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
            }

            {
                ctx.m_state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));
                if (ctx.m_assets.prepassDepthEnabled) {
                    ctx.m_state.setDepthFunc(GL_LEQUAL);
                }

                drawNodesImpl(
                    ctx,
                    [&typeSelector](const mesh::MeshType* type) { return type->m_flags.gbuffer && typeSelector(type); },
                    nodeSelector,
                    kindBits);

                ctx.m_batch->flush(ctx);

                if (ctx.m_assets.prepassDepthEnabled) {
                    ctx.m_state.setDepthFunc(ctx.m_depthFunc);
                }
            }

            ctx.setAllowBlend(oldAllowBlend);

            m_gBuffer.m_buffer->copy(
                m_gBuffer.m_depthTexture.get(),
                GBuffer::ATT_DEPTH_INDEX);

            m_gBuffer.bindTexture(ctx);
        }

        // pass 2 - target effectBuffer
        {
            primaryBuffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

            primaryBuffer->bind(ctx);
            primaryBuffer->clearAll();
        }

        // pass 3 - light
        //if (false)
        {
            ctx.m_state.setStencil(kigl::GLStencilMode::only_non_zero());
            ctx.m_state.setEnabled(GL_DEPTH_TEST, false);

            primaryBuffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

            m_deferredProgram->bind(ctx.m_state);
            m_textureQuad.draw(ctx.m_state);

            primaryBuffer->resetDrawBuffers(1);

            ctx.m_state.setEnabled(GL_DEPTH_TEST, true);
        }

        // pass 4 - non G-buffer solid nodes
        // => separate light calculations
        // => currently these *CANNOT* work correctly
        //if (false)
        {
            ctx.validateRender("non_gbuffer");

            ctx.m_state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));

            bool rendered = drawNodesImpl(
                ctx,
                [&typeSelector](const mesh::MeshType* type) { return !type->m_flags.gbuffer && typeSelector(type); },
                nodeSelector,
                kindBits);

            if (rendered) {
                ctx.m_batch->flush(ctx);

                // NOTE KI need to reset possibly changed drawing modes
                // ex. selection volume changes to GL_LINE
                ctx.bindDefaults();
            }
        }

        // pass 5 - OIT
        // NOTE KI OIT after *forward* pass to allow using depth texture from them
        if (ctx.m_allowBlend)
        {
            if (ctx.m_assets.effectOitEnabled)
            {
                ctx.m_state.setStencil(kigl::GLStencilMode::fill(STENCIL_OIT | STENCIL_FOG));
                // NOTE KI do NOT modify depth with blend
                ctx.m_state.setDepthMask(GL_FALSE);

                ctx.m_state.setEnabled(GL_BLEND, true);

                m_oitBuffer.bind(ctx);
                m_oitBuffer.clearAll();

                // NOTE KI different blend mode for each draw buffer
                glBlendFunci(0, GL_ONE, GL_ONE);
                glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                glBlendEquation(GL_FUNC_ADD);

                // only "blend OIT" nodes
                drawProgram(
                    ctx,
                    [this](const mesh::MeshType* type) { return m_oitProgram; },
                    [&typeSelector](const mesh::MeshType* type) { return type->m_flags.blendOIT && typeSelector(type); },
                    nodeSelector,
                    NodeDraw::KIND_ALL);

                ctx.m_batch->flush(ctx);

                // NOTE KI *MUST* reset blend mode (especially for attachment 1)
                // ex. if not done OIT vs. bloom works strangely
                glBlendFunci(0, GL_ONE, GL_ONE);
                glBlendFunci(1, GL_ONE, GL_ONE);
                ctx.m_state.invalidateBlendMode();

                ctx.m_state.setEnabled(GL_BLEND, false);

                ctx.m_state.setDepthMask(GL_TRUE);
            }
        }

        {
            primaryBuffer->bind(ctx);
        }

        // pass 6 - skybox (*before* blend)
        {
            ctx.m_state.setStencil(kigl::GLStencilMode::fill(STENCIL_SKYBOX, STENCIL_SKYBOX, ~STENCIL_OIT));
            drawSkybox(ctx);
        }

        // pass 7 - blend effects
        // => separate light calculations
        //if (false)
        if (ctx.m_allowBlend)
        {
            ctx.m_state.setStencil({});

            drawBlendedImpl(
                ctx,
                [&typeSelector](const mesh::MeshType* type) {
                    return !type->m_flags.blendOIT &&
                        type->m_flags.blend &&
                        type->m_flags.effect &&
                        typeSelector(type);
                },
                nodeSelector);
            ctx.m_batch->flush(ctx);
        }

        // pass 8 - screenspace effects
        if (ctx.m_allowBlend)
        {
            ctx.m_state.setEnabled(GL_DEPTH_TEST, false);
            // NOTE KI do NOT modify depth with blend (likely redundant)
            ctx.m_state.setDepthMask(GL_FALSE);

            {
                ctx.m_state.setEnabled(GL_BLEND, true);
                ctx.m_state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

                if (ctx.m_assets.effectFogEnabled) {
                    ctx.m_state.setStencil(kigl::GLStencilMode::only(STENCIL_FOG, STENCIL_FOG));
                    m_fogProgram->bind(ctx.m_state);
                    m_textureQuad.draw(ctx.m_state);
                }

                if (ctx.m_assets.effectOitEnabled) {
                    ctx.m_state.setStencil(kigl::GLStencilMode::only_non_zero(STENCIL_OIT));

                    m_blendOitProgram->bind(ctx.m_state);
                    m_oitBuffer.bindTexture(ctx);

                    m_textureQuad.draw(ctx.m_state);
                }

                ctx.m_state.setEnabled(GL_BLEND, false);
            }

            if (ctx.m_assets.effectBloomEnabled)
            {
                ctx.m_state.setStencil({});

                primaryBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);
                primaryBuffer->bindTexture(ctx, EffectBuffer::ATT_BRIGHT_INDEX, UNIT_EFFECT_BRIGHT);

                //m_emissionProgram->bind(ctx.m_state);
                //m_textureQuad.draw(ctx);

                m_bloomProgram->bind(ctx.m_state);
                primaryBuffer->bindTexture(ctx, EffectBuffer::ATT_BRIGHT_INDEX, UNIT_EFFECT_WORK);

                bool cleared[2]{ false, false };

                for (int i = 0; i < ctx.m_assets.effectBloomIterations; i++) {
                    auto& buf = m_effectBuffer.m_buffers[i % 2];
                    buf->bind(ctx);

                    if (!cleared[i % 2]) {
                        cleared[i % 2] = true;
                        buf->clearAll();
                    }

                    m_bloomProgram->m_uniforms->u_effectBloomIteration.set(i);
                    m_textureQuad.draw(ctx.m_state);

                    buf->bindTexture(ctx, EffectBuffer::ATT_WORK_INDEX, UNIT_EFFECT_WORK);
                }

                secondaryBuffer->bind(ctx);
                secondaryBuffer->clearAll();

                m_blendBloomProgram->bind(ctx.m_state);
                m_textureQuad.draw(ctx.m_state);
            }
            else {
                primaryBuffer->copy(
                    secondaryBuffer,
                    EffectBuffer::ATT_ALBEDO_INDEX,
                    EffectBuffer::ATT_ALBEDO_INDEX);

                // NOTE KI no need to bind; no draw to buffer
                //secondaryBuffer->bind(ctx);
            }

            ctx.m_state.setDepthMask(GL_TRUE);
            ctx.m_state.setEnabled(GL_DEPTH_TEST, true);

            ctx.m_state.setStencil({});
        }

        // pass 11 - debug info
        {
            drawDebug(ctx, secondaryBuffer);
        }

        // pass 10 - render to target
        {
            // NOTE KI binding target buffer should be 100% redundant here
            // i.e. blit does not need it
            // NOTE KI *Exception* CubeMapBuffer::bind logic
            // => cubemap is *NOT* exception any longer
            //targetBuffer->bind(ctx);

            if (copyMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
                m_gBuffer.m_buffer->blit(
                    targetBuffer,
                    copyMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT),
                    { -1.f, 1.f },
                    { 2.f, 2.f },
                    GL_NEAREST);
            }

            if (copyMask & GL_COLOR_BUFFER_BIT) {
                GLenum sourceFormat = secondaryBuffer->m_spec.attachments[EffectBuffer::ATT_ALBEDO_INDEX].internalFormat;
                GLenum targetFormat = -1;

                if (!targetBuffer->m_spec.attachments.empty()) {
                    targetFormat = targetBuffer->m_spec.attachments[EffectBuffer::ATT_ALBEDO_INDEX].internalFormat;
                }

                const bool canCopy = !targetBuffer->m_spec.attachments.empty() &&
                    targetBuffer->m_spec.width == secondaryBuffer->m_spec.width &&
                    targetBuffer->m_spec.height == secondaryBuffer->m_spec.height &&
                    targetFormat == sourceFormat;

                if (canCopy) {
                    secondaryBuffer->copy(
                        targetBuffer,
                        EffectBuffer::ATT_ALBEDO_INDEX,
                        // NOTE KI assumption; buffer is at index 0
                        EffectBuffer::ATT_ALBEDO_INDEX);
                }
                else {
                    // NOTE KI hdri tone & gamma correct done at the viewport render
                    bool hdr = false;// targetFormat != GL_RGB16F && targetFormat != GL_RGBA16F;

                    if (hdr) {
                        targetBuffer->bind(ctx);
                        m_hdrGammaProgram->bind(ctx.m_state);
                        secondaryBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);
                        m_textureQuad.draw(ctx.m_state);
                    }
                    else {
                        secondaryBuffer->blit(
                            targetBuffer,
                            GL_COLOR_BUFFER_BIT,
                            GL_COLOR_ATTACHMENT0,
                            GL_COLOR_ATTACHMENT0,
                            { -1.f, 1.f },
                            { 2.f, 2.f },
                            GL_LINEAR);
                    }
                }
            }
        }

        // pass 12 - cleanup
        if (ctx.m_assets.glUseInvalidate)
        {
            //ctx.m_state.bindFrameBuffer(0, false);

            if (ctx.m_assets.effectOitEnabled) {
                m_oitBuffer.invalidateAll();
            }
            m_effectBuffer.invalidateAll();
            m_gBuffer.invalidateAll();
        }

        //m_timeElapsedQuery.end();

        //if (m_timeElapsedQuery.count() % 100 == 0) {
        //    //KI_INFO_OUT(fmt::format("AVG: {:.3} ms", m_timeElapsedQuery.avg(true) * 1.0e-6));
        //}
    }

    void NodeDraw::drawDebug(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer)
    {
        if (!(ctx.m_allowDrawDebug && ctx.m_assets.drawDebug)) return;

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

    void NodeDraw::drawBlended(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector)
    {
        targetBuffer->bind(ctx);
        drawBlendedImpl(ctx, typeSelector, nodeSelector);
        ctx.m_batch->flush(ctx);
    }

    bool NodeDraw::drawNodesImpl(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        unsigned int kindBits)
    {
        bool rendered{ false };

        auto* nodeRegistry = ctx.m_registry->m_nodeRegistry;

        auto renderTypes = [this, &ctx, &typeSelector, &nodeSelector, &rendered](const MeshTypeMap& typeMap) {
            auto* program = typeMap.begin()->first.type->m_program;

            for (const auto& it : typeMap) {
                auto* type = it.first.type;

                if (!type->isReady()) continue;
                if (!typeSelector(type)) continue;

                auto& batch = ctx.m_batch;

                for (auto& node : it.second) {
                    if (!nodeSelector(node)) continue;
                    rendered = true;
                    batch->draw(ctx, *node, program);
                }
            }
            };

        if (kindBits & NodeDraw::KIND_SOLID) {
            for (const auto& all : m_solidNodes) {
                renderTypes(all.second);
            }
        }

        if (kindBits & NodeDraw::KIND_SPRITE) {
            for (const auto& all : m_spriteNodes) {
                renderTypes(all.second);
            }
        }

        if (kindBits & NodeDraw::KIND_ALPHA) {
            for (const auto& all : m_alphaNodes) {
                renderTypes(all.second);
            }
        }

        return rendered;
    }

    void NodeDraw::drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector)
    {
        if (m_blendedNodes.empty()) return;

        const glm::vec3& viewPos = ctx.m_camera->getWorldPosition();

        // TODO KI discards nodes if *same* distance
        std::map<float, Node*> sorted;
        for (const auto& all : m_blendedNodes) {
            for (const auto& map : all.second) {
                auto* type = map.first.type;

                if (!type->isReady()) continue;
                if (!typeSelector(type)) continue;

                for (const auto& node : map.second) {
                    if (!nodeSelector(node)) continue;

                    const float distance = glm::length(viewPos - node->getSnapshot().getWorldPosition());
                    sorted[distance] = node;
                }
            }
        }

        // NOTE KI blending is *NOT* optimal program / nodetypw wise due to depth sorting
        // NOTE KI order = from furthest away to nearest
        for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            auto* node = it->second;
            auto* program = node->m_type->m_program;

            ctx.m_batch->draw(ctx, *node, program);
        }

        // TODO KI if no flush here then render order of blended nodes is incorrect
        //ctx.m_batch->flush(ctx);
    }

    void NodeDraw::drawProgram(
        const RenderContext& ctx,
        const std::function<Program* (const mesh::MeshType*)>& programSelector,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        unsigned int kindBits)
    {
        auto renderTypes = [this, &ctx, &programSelector, &typeSelector, &nodeSelector](const MeshTypeMap& typeMap) {
            for (const auto& it : typeMap) {
                auto* type = it.first.type;

                if (!type->isReady()) continue;
                if (!typeSelector(type)) continue;

                auto activeProgram = programSelector(type);
                if (!activeProgram) continue;

                auto& batch = ctx.m_batch;

                for (auto& node : it.second) {
                    if (!nodeSelector(node)) continue;

                    batch->draw(ctx, *node, activeProgram);
                }
            }
            };

        if (kindBits & NodeDraw::KIND_SOLID) {
            for (const auto& all : m_solidNodes) {
                renderTypes(all.second);
            }
        }

        if (kindBits & NodeDraw::KIND_SPRITE) {
            for (const auto& all : m_spriteNodes) {
                renderTypes(all.second);
            }
        }

        if (kindBits & NodeDraw::KIND_ALPHA) {
            for (const auto& all : m_alphaNodes) {
                renderTypes(all.second);
            }
        }

        if (kindBits & NodeDraw::KIND_BLEND) {
            for (const auto& all : m_blendedNodes) {
                renderTypes(all.second);
            }
        }
    }

    void NodeDraw::drawSkybox(
        const RenderContext& ctx)
    {
        Node* node = ctx.m_registry->m_nodeRegistry->m_skybox;
        if (!node) return;

        if (!node->m_type->isReady()) return;

        auto& batch = ctx.m_batch;
        auto* program = node->m_type->m_program;

        ctx.m_state.setDepthFunc(GL_LEQUAL);
        program->bind(ctx.m_state);
        m_textureQuad.draw(ctx.m_state);
        ctx.m_state.setDepthFunc(ctx.m_depthFunc);
    }

    void NodeDraw::insertNode(NodeVector& list, Node* node)
    {
        list.reserve(100);
        list.push_back(node);
    }
}
