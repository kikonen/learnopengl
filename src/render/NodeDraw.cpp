#include "NodeDraw.h"

#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "component/Camera.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/Batch.h"


void NodeDraw::prepare(
    const Assets& assets,
    Registry* registry)
{
    m_gBuffer.prepare(assets);
    m_oitBuffer.prepare(assets, &m_gBuffer);
    m_effectBuffer.prepare(assets, &m_gBuffer);

    m_plainQuad.prepare();
    m_textureQuad.prepare();

    m_deferredProgram = registry->m_programRegistry->getProgram(SHADER_DEFERRED_PASS);
    m_deferredProgram ->prepare(assets);

    m_oitProgram = registry->m_programRegistry->getProgram(SHADER_OIT_PASS);
    m_oitProgram->prepare(assets);

    m_blendOitProgram = registry->m_programRegistry->getProgram(SHADER_BLEND_OIT_PASS);
    m_blendOitProgram->prepare(assets);

    m_bloomProgram = registry->m_programRegistry->getProgram(SHADER_BLOOM_PASS);
    m_bloomProgram->prepare(assets);

    m_blendBloomProgram = registry->m_programRegistry->getProgram(SHADER_BLEND_BLOOM_PASS);
    m_blendBloomProgram->prepare(assets);

    m_emissionProgram = registry->m_programRegistry->getProgram(SHADER_EMISSION_PASS);
    m_emissionProgram->prepare(assets);

    m_fogProgram = registry->m_programRegistry->getProgram(SHADER_FOG_PASS);
    m_fogProgram->prepare(assets);

    m_hdrGammaProgram = registry->m_programRegistry->getProgram(SHADER_HDR_GAMMA_PASS);
    m_hdrGammaProgram->prepare(assets);

    m_timeElapsedQuery.create();
}

void NodeDraw::updateView(const RenderContext& ctx)
{
    m_gBuffer.updateView(ctx);
    m_oitBuffer.updateView(ctx);
    m_effectBuffer.updateView(ctx);
}

void NodeDraw::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector,
    unsigned int kindBits,
    GLbitfield copyMask)
{
    // pass 1.1 - draw geometry
    // => nodes supporting G-buffer
    //if (false)
    {
        m_gBuffer.bind(ctx);
        m_gBuffer.clearAll();

        // NOTE KI no blend in G-buffer
        auto oldAllowBlend = ctx.setAllowBlend(false);

        drawNodesImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return type->m_flags.gbuffer && typeSelector(type); },
            nodeSelector,
            kindBits);

        ctx.m_batch->flush(ctx);

        ctx.setAllowBlend(oldAllowBlend);

        m_gBuffer.bindTexture(ctx);
    }

    // https://community.khronos.org/t/selectively-writing-to-buffers/71054
    auto* activeBuffer = m_effectBuffer.m_primary.get();

    // pass 2 => effectBuffer
    {
        activeBuffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

        activeBuffer->bind(ctx);
        m_effectBuffer.clearAll();

        activeBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);
        activeBuffer->bindTexture(ctx, EffectBuffer::ATT_BRIGHT_INDEX, UNIT_EFFECT_BRIGHT);
    }

    // pass 2.1 - light
    //if (false)
    {
        ctx.m_state.setEnabled(GL_DEPTH_TEST, false);

        m_deferredProgram->bind(ctx.m_state);
        m_textureQuad.draw(ctx.m_state);

        ctx.m_state.setEnabled(GL_DEPTH_TEST, true);

        // NOTE KI make depth available for "non gbuffer node" rendering
        if (false) {
            m_gBuffer.m_buffer->blit(
                activeBuffer,
                GL_DEPTH_BUFFER_BIT,
                { -1.f, 1.f },
                { 2.f, 2.f },
                GL_NEAREST);
        }
        else {
            // NOTE KI copy does not work with RBO
            m_gBuffer.m_buffer->copy(
                activeBuffer,
                GBuffer::ATT_DEPTH_INDEX,
                EffectBuffer::ATT_DEPTH_INDEX);
        }

        m_effectBuffer.m_primary->resetDrawBuffers(1);
    }

    // pass 3 - non G-buffer nodes
    // => for example, "skybox" (skybox is mostly via g_skybox now!)
    // => separate light calculations
    // => currently these *CANNOT* work correctly
    //if (false)
    {
        ctx.validateRender("non_gbuffer");

        bool rendered = drawNodesImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return !type->m_flags.gbuffer && typeSelector(type); },
            nodeSelector,
            kindBits);

        if (rendered) {
            ctx.m_batch->flush(ctx);

            if (false) {
                activeBuffer->blit(
                    m_gBuffer.m_buffer.get(),
                    GL_DEPTH_BUFFER_BIT,
                    { -1.f, 1.f },
                    { 2.f, 2.f },
                    GL_NEAREST);
            }
            else {
                // copy depth back
                activeBuffer->copy(
                    m_gBuffer.m_buffer.get(),
                    EffectBuffer::ATT_DEPTH_INDEX,
                    GBuffer::ATT_DEPTH_INDEX);
            }

            // NOTE KI need to reset possibly changed drawing modes
            // ex. selection volume changes to GL_LINE
            ctx.bindDefaults();
        }
    }

    // pass 1.2 - draw OIT
    // NOTE KI OIT after *forward* pass to allow using depth texture from them
    if (ctx.m_assets.effectOitEnabled)
    {
        m_oitBuffer.bind(ctx);
        m_oitBuffer.clearAll();

        // NOTE KI do NOT modify depth with blend
        auto oldDepthMask = ctx.m_state.setDepthMask(GL_FALSE);

        // NOTE KI different blend mode for each draw buffer
        glBlendFunci(0, GL_ONE, GL_ONE);
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glBlendEquation(GL_FUNC_ADD);

        // only "blend OIT" nodes
        drawProgram(
            ctx,
            m_oitProgram,
            nullptr,
            [&typeSelector](const MeshType* type) { return type->m_flags.blendOIT && typeSelector(type); },
            nodeSelector,
            NodeDraw::KIND_ALL);

        ctx.m_batch->flush(ctx);

        // NOTE KI *MUST* reset blend mode (especially for attachment 1)
        // ex. if not done OIT vs. bloom works strangely
        glBlendFunci(0, GL_ONE, GL_ONE);
        glBlendFunci(1, GL_ONE, GL_ONE);
        ctx.m_state.clearBlendMode();
        ctx.m_state.setDepthMask(oldDepthMask);

        m_oitBuffer.bindTexture(ctx);
    }

    activeBuffer->bind(ctx);

    // pass 4 - blend
    // => separate light calculations
    //if (false)
    if (ctx.m_allowBlend)
    {
        drawBlendedImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return !type->m_flags.blendOIT && typeSelector(type); },
            nodeSelector);
        ctx.m_batch->flush(ctx);
    }

    // pass 3 - blend screenspace effects
    if (ctx.m_allowBlend)
    {
        ctx.m_state.setEnabled(GL_DEPTH_TEST, false);
        // NOTE KI do NOT modify depth with blend (likely redundant)
        auto oldDepthMask = ctx.m_state.setDepthMask(GL_FALSE);

        {
            ctx.m_state.setEnabled(GL_BLEND, true);
            ctx.m_state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

            if (ctx.m_assets.effectFogEnabled) {
                m_fogProgram->bind(ctx.m_state);
                m_textureQuad.draw(ctx.m_state);
            }

            if (ctx.m_assets.effectOitEnabled) {
                activeBuffer->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

                m_blendOitProgram->bind(ctx.m_state);
                m_textureQuad.draw(ctx.m_state);
            }

            ctx.m_state.setEnabled(GL_BLEND, false);
        }

        if (ctx.m_assets.effectBloomEnabled)
        {
            //m_emissionProgram->bind(ctx.m_state);
            //m_textureQuad.draw(ctx);

            m_bloomProgram->bind(ctx.m_state);
            activeBuffer->bindTexture(ctx, EffectBuffer::ATT_BRIGHT_INDEX, UNIT_EFFECT_WORK);

            for (int i = 0; i < ctx.m_assets.effectBloomIterations; i++) {
                auto& buf = m_effectBuffer.m_buffers[i % 2];
                buf->bind(ctx);

                m_bloomProgram->u_effectBloomIteration->set(i);
                m_textureQuad.draw(ctx.m_state);

                buf->bindTexture(ctx, EffectBuffer::ATT_WORK_INDEX, UNIT_EFFECT_WORK);
            }

            activeBuffer = m_effectBuffer.m_secondary.get();
            activeBuffer->bind(ctx);

            m_blendBloomProgram->bind(ctx.m_state);
            m_textureQuad.draw(ctx.m_state);
        }
        else {
            activeBuffer->copy(
                m_effectBuffer.m_secondary.get(),
                EffectBuffer::ATT_ALBEDO_INDEX,
                EffectBuffer::ATT_ALBEDO_INDEX);

            activeBuffer = m_effectBuffer.m_secondary.get();
            activeBuffer->bind(ctx);
        }

        ctx.m_state.setDepthMask(oldDepthMask);
        ctx.m_state.setEnabled(GL_DEPTH_TEST, true);
    }

    // pass 5 - render to target
    {
        // NOTE KI binding target buffer should be 100% redundant here
        // i.e. blit does not need it
        // NOTE KI *Exception* CubeMapBuffer::bind logic
        //targetBuffer->bind(ctx);

        if (copyMask & GL_DEPTH_BUFFER_BIT) {
            m_gBuffer.m_buffer->blit(
                targetBuffer,
                GL_DEPTH_BUFFER_BIT,
                { -1.f, 1.f },
                { 2.f, 2.f },
                GL_NEAREST);
        }

        if (copyMask & GL_COLOR_BUFFER_BIT) {
            GLenum sourceFormat = activeBuffer->m_spec.attachments[EffectBuffer::ATT_ALBEDO_INDEX].internalFormat;
            GLenum targetFormat = -1;

            if (!targetBuffer->m_spec.attachments.empty()) {
                targetFormat = targetBuffer->m_spec.attachments[EffectBuffer::ATT_ALBEDO_INDEX].internalFormat;
            }

            const bool canCopy = !targetBuffer->m_spec.attachments.empty() &&
                targetBuffer->m_spec.width == activeBuffer->m_spec.width &&
                targetBuffer->m_spec.height == activeBuffer->m_spec.height &&
                targetFormat == sourceFormat;

            if (canCopy) {
                activeBuffer->copy(
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
                    activeBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);
                    m_textureQuad.draw(ctx.m_state);
                }
                else {
                    activeBuffer->blit(
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

        // pass 5 - debug info
        drawDebug(ctx, targetBuffer);
    }

    // cleanup
    {
        //ctx.m_state.bindFrameBuffer(0, false);

        if (ctx.m_assets.effectOitEnabled) {
            m_oitBuffer.invalidateAll();
        }
        m_effectBuffer.invalidateAll();
        m_gBuffer.invalidateAll();
    }
}

void NodeDraw::drawDebug(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!(ctx.m_allowDrawDebug && ctx.m_assets.drawDebug)) return;

    m_effectBuffer.m_primary->resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);

    constexpr float SZ1 = 0.25f;
    //constexpr float SZ2 = 0.5f;

    int count = 0;
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
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector)
{
    targetBuffer->bind(ctx);
    drawBlendedImpl(ctx, typeSelector, nodeSelector);
    ctx.m_batch->flush(ctx);
}

bool NodeDraw::drawNodesImpl(
    const RenderContext& ctx,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector,
    unsigned int kindBits)
{
    bool rendered{ false };

    auto* nodeRegistry = ctx.m_registry->m_nodeRegistry;

    auto renderTypes = [this, &ctx, &typeSelector, &nodeSelector, &rendered](const MeshTypeMap& typeMap) {
        auto* program = typeMap.begin()->first.type->m_program;

        for (const auto& it : typeMap) {
            auto* type = it.first.type;

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
        for (const auto& all : nodeRegistry->solidNodes) {
            renderTypes(all.second);
        }
    }

    if (kindBits & NodeDraw::KIND_SPRITE) {
        for (const auto& all : nodeRegistry->spriteNodes) {
            renderTypes(all.second);
        }
    }

    if (kindBits & NodeDraw::KIND_ALPHA) {
        for (const auto& all : nodeRegistry->alphaNodes) {
            renderTypes(all.second);
        }
    }

    return rendered;
}

void NodeDraw::drawBlendedImpl(
    const RenderContext& ctx,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector)
{
    if (ctx.m_registry->m_nodeRegistry->blendedNodes.empty()) return;

    const glm::vec3& viewPos = ctx.m_camera->getWorldPosition();

    // TODO KI discards nodes if *same* distance
    std::map<float, Node*> sorted;
    for (const auto& all : ctx.m_registry->m_nodeRegistry->blendedNodes) {
        for (const auto& map : all.second) {
            auto* type = map.first.type;

            if (!typeSelector(type)) continue;

            for (const auto& node : map.second) {
                if (!nodeSelector(node)) continue;

                const float distance = glm::length(viewPos - node->getWorldPosition());
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
    Program* program,
    Program* programPointSprite,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector,
    unsigned int kindBits)
{
    auto renderTypes = [this, &ctx, &program, &programPointSprite, &typeSelector, &nodeSelector](const MeshTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto* type = it.first.type;

            if (!typeSelector(type)) continue;

            auto& batch = ctx.m_batch;

            auto activeProgram = program;
            if (type->m_entityType == EntityType::point_sprite) {
                activeProgram = programPointSprite;
            }

            if (!activeProgram) continue;

            for (auto& node : it.second) {
                if (!nodeSelector(node)) continue;

                batch->draw(ctx, *node, activeProgram);
            }
        }
    };

    if (kindBits & NodeDraw::KIND_SOLID) {
        for (const auto& all : ctx.m_registry->m_nodeRegistry->solidNodes) {
            renderTypes(all.second);
        }
    }

    if (kindBits & NodeDraw::KIND_SPRITE) {
        for (const auto& all : ctx.m_registry->m_nodeRegistry->spriteNodes) {
            renderTypes(all.second);
        }
    }

    if (kindBits & NodeDraw::KIND_ALPHA) {
        for (const auto& all : ctx.m_registry->m_nodeRegistry->alphaNodes) {
            renderTypes(all.second);
        }
    }

    if (kindBits & NodeDraw::KIND_BLEND) {
        for (const auto& all : ctx.m_registry->m_nodeRegistry->blendedNodes) {
            renderTypes(all.second);
        }
    }
}
