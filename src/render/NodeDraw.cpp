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
    GLbitfield copyMask)
{
    // pass 1.1 - draw geometry
    // => nodes supporting G-buffer
    //if (false)
    {
        m_gBuffer.bind(ctx);
        m_gBuffer.m_buffer->clearAll();

        // NOTE KI no blend in G-buffer
        auto oldAllowBlend = ctx.setAllowBlend(false);

        drawNodesImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return type->m_flags.gbuffer && typeSelector(type); },
            nodeSelector);

        ctx.m_batch->flush(ctx);

        ctx.setAllowBlend(oldAllowBlend);
    }

    // pass 1.2 - draw OIT
    //if (false)
    {
        m_oitBuffer.bind(ctx);

        m_oitBuffer.m_buffer->clearAttachment(0);
        m_oitBuffer.m_buffer->clearAttachment(1);

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
            nodeSelector);

        ctx.m_batch->flush(ctx);

        // NOTE KI *MUST* reset blend mode (messed caching earlier)
        ctx.m_state.clearBlendMode();
        ctx.m_state.setDepthMask(oldDepthMask);
    }

    // pass 2 => effectBuffer
    {
        m_effectBuffer.bind(ctx);
        m_effectBuffer.clear();

        m_gBuffer.bindTexture(ctx);
        m_oitBuffer.bindTexture(ctx);
        m_effectBuffer.bindTexture(ctx);
    }

    // pass 2.1 - light
    //if (false)
    {
        ctx.m_state.setEnabled(GL_DEPTH_TEST, false);

        m_deferredProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);

        ctx.m_state.setEnabled(GL_DEPTH_TEST, true);

        // NOTE KI make depth available for "non gbuffer node" rendering
        m_gBuffer.m_buffer->blit(
            m_effectBuffer.m_primary.get(),
            GL_DEPTH_BUFFER_BIT,
            { -1.f, 1.f },
            { 2.f, 2.f },
            GL_NEAREST);
    }

    // pass 3 - non G-buffer nodes
    // => for example, "skybox" (skybox is mostly via g_skybox now!)
    // => separate light calculations
    //if (false)
    {
        drawNodesImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return !type->m_flags.gbuffer && typeSelector(type); },
            nodeSelector);

        ctx.m_batch->flush(ctx);
    }

    // pass 4 - blend
    // => separate light calculations
    //if (false)
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

        //if (false)
        {
            //m_emissionProgram->bind(ctx.m_state);
            //m_plainQuad.draw(ctx);

            m_bloomProgram->bind(ctx.m_state);
            m_effectBuffer.m_primary->bindTexture(ctx, 1, UNIT_EFFECT_WORK);
            for (int i = 0; i < ctx.m_assets.effectBloomIterations; i++) {
                auto& buf = m_effectBuffer.m_buffers[i % 2];
                buf->bind(ctx);

                m_bloomProgram->u_effectBloomIteration->set(i);

                m_plainQuad.draw(ctx);
                buf->bindTexture(ctx, 0, UNIT_EFFECT_WORK);
            }

            m_effectBuffer.m_primary->bind(ctx);

            m_blendBloomProgram->bind(ctx.m_state);
            m_plainQuad.draw(ctx);
        }

        ctx.m_state.setEnabled(GL_BLEND, true);
        ctx.m_state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        m_blendOitProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);

        m_fogProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);

        ctx.m_state.setEnabled(GL_BLEND, false);
        ctx.m_state.setEnabled(GL_DEPTH_TEST, true);
    }

    // pass 5 - render to target
    {
        targetBuffer->bind(ctx);

        if ((copyMask & GL_DEPTH_BUFFER_BIT) != 0) {
            m_gBuffer.m_buffer->blit(
                targetBuffer,
                GL_DEPTH_BUFFER_BIT,
                { -1.f, 1.f },
                { 2.f, 2.f },
                GL_NEAREST);
        }

        if ((copyMask & GL_COLOR_BUFFER_BIT) != 0) {
            m_effectBuffer.m_primary->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT0,
                { -1.f, 1.f },
                { 2.f, 2.f },
                GL_LINEAR);
        }

        // pass 5 - debug info
        drawDebug(ctx, targetBuffer);
    }
}

void NodeDraw::drawDebug(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!(ctx.m_allowDrawDebug && ctx.m_assets.drawDebug)) return;

    constexpr float SZ1 = 0.25f;
    //constexpr float SZ2 = 0.5f;

    int count = 0;
    for (int i = 0; i < m_oitBuffer.m_buffer->getDrawBufferCount(); i++) {
        m_oitBuffer.m_buffer->blit(
            targetBuffer,
            GL_COLOR_BUFFER_BIT,
            GL_COLOR_ATTACHMENT0 + i,
            GL_COLOR_ATTACHMENT0,
            { -1.f, -1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
            GL_NEAREST);
    }
    count += m_oitBuffer.m_buffer->getDrawBufferCount();

    for (int i = 0; i < m_effectBuffer.m_primary->getDrawBufferCount(); i++) {
        m_effectBuffer.m_primary->blit(
            targetBuffer,
            GL_COLOR_BUFFER_BIT,
            GL_COLOR_ATTACHMENT0 + i,
            GL_COLOR_ATTACHMENT0,
            { -1.f, -1 + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
            GL_NEAREST);
    }
    count += m_oitBuffer.m_buffer->getDrawBufferCount();

    for (int i = 0; i < m_effectBuffer.m_buffers.size(); i++) {
        auto& buf = m_effectBuffer.m_buffers[i];
        buf->blit(
            targetBuffer,
            GL_COLOR_BUFFER_BIT,
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT0,
            { -1.f, -1 + count * SZ1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
            GL_NEAREST);
    }
    count += m_effectBuffer.m_buffers.size();

    for (int i = 0; i < m_gBuffer.m_buffer->getDrawBufferCount(); i++) {
        m_gBuffer.m_buffer->blit(
            targetBuffer,
            GL_COLOR_BUFFER_BIT,
            GL_COLOR_ATTACHMENT0 + i,
            GL_COLOR_ATTACHMENT0,
            { 1 - SZ1, -1 + SZ1 + i * SZ1 }, { SZ1, SZ1 },
            GL_NEAREST);
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

void NodeDraw::drawNodesImpl(
    const RenderContext& ctx,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector)
{
    auto* nodeRegistry = ctx.m_registry->m_nodeRegistry;

    auto renderTypes = [this, &ctx, &typeSelector, &nodeSelector](const MeshTypeMap& typeMap) {
        auto* program = typeMap.begin()->first.type->m_program;

        for (const auto& it : typeMap) {
            auto* type = it.first.type;

            if (!typeSelector(type)) continue;

            auto& batch = ctx.m_batch;

            for (auto& node : it.second) {
                if (!nodeSelector(node)) continue;
                batch->draw(ctx, *node, program);
            }
        }
    };

    for (const auto& all : nodeRegistry->solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : nodeRegistry->alphaNodes) {
        renderTypes(all.second);
    }
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
    Program* programSprite,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector)
{
    auto renderTypes = [this, &ctx, &program, &programSprite, &typeSelector, &nodeSelector](const MeshTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto* type = it.first.type;

            if (!typeSelector(type)) continue;

            auto& batch = ctx.m_batch;

            auto activeProgram = program;
            if (type->m_entityType == EntityType::sprite) {
                activeProgram = programSprite;
            }

            if (!activeProgram) continue;

            for (auto& node : it.second) {
                if (!nodeSelector(node)) continue;

                batch->draw(ctx, *node, activeProgram);
            }
        }
    };

    for (const auto& all : ctx.m_registry->m_nodeRegistry->solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.m_registry->m_nodeRegistry->alphaNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.m_registry->m_nodeRegistry->blendedNodes) {
        renderTypes(all.second);
    }
}
