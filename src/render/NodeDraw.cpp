#include "NodeDraw.h"

#include "asset/Program.h"
#include "asset/Shader.h"

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

    m_plainQuad.prepare();
    m_textureQuad.prepare();

    m_deferredProgram = registry->m_programRegistry->getProgram(SHADER_DEFERRED_PASS);
    m_deferredProgram ->prepare(assets);

    m_oitProgram = registry->m_programRegistry->getProgram(SHADER_OIT_PASS);
    m_oitProgram->prepare(assets);

    m_blendOitProgram = registry->m_programRegistry->getProgram(SHADER_BLEND_OIT_PASS);
    m_blendOitProgram->prepare(assets);

    m_emissionProgram = registry->m_programRegistry->getProgram(SHADER_EMISSION_PASS);
    m_emissionProgram->prepare(assets);

    m_fogProgram = registry->m_programRegistry->getProgram(SHADER_FOG_PASS);
    m_fogProgram->prepare(assets);
}

void NodeDraw::updateView(const RenderContext& ctx)
{
    m_gBuffer.updateView(ctx);
    m_oitBuffer.updateView(ctx);
}

void NodeDraw::clear(
    const RenderContext& ctx,
    GLbitfield clearMask,
    const glm::vec4& clearColor)
{
    //m_gbuffer.bind(ctx);
    //m_gbuffer.m_buffer->clear(ctx, clearMask, clearColor);

    //m_oitbuffer.bind(ctx);
    //m_oitbuffer.m_buffer->clear(ctx, clearMask, clearColor);
}

void NodeDraw::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector,
    GLbitfield clearMask,
    const glm::vec4& clearColor)
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

    // pass 2 => targetBuffer
    {
        targetBuffer->bind(ctx);
        targetBuffer->clear(ctx, clearMask, { 0.f , 0.f, 0.f, 0.f });

        m_gBuffer.bindTexture(ctx);
        m_oitBuffer.bindTexture(ctx);
    }

    // pass 2.1 - light
    //if (false)
    {
        m_deferredProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);
    }

    // pass 3 - non G-buffer nodes
    // => for example, "skybox" (skybox is mostly via g_skybox now!)
    // => separate light calculations
    //if (false)
    {
        // NOTE KI *wrong* blit
        // TODO KI broken depth blit
        m_gBuffer.m_buffer->blit(targetBuffer, GL_DEPTH_BUFFER_BIT, { -1.f, 1.f }, { 2.f, 2.f });

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
        ctx.m_state.setEnabled(GL_BLEND, true);
        ctx.m_state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        m_emissionProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);

        m_blendOitProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);

        m_fogProgram->bind(ctx.m_state);
        m_plainQuad.draw(ctx);

        ctx.m_state.setEnabled(GL_BLEND, false);
        ctx.m_state.setEnabled(GL_DEPTH_TEST, true);
    }

    // pass 5 - debug info
    drawDebug(ctx, targetBuffer);
}

void NodeDraw::drawDebug(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!(ctx.m_allowDrawDebug && ctx.m_assets.drawDebug)) return;

    constexpr float SZ1 = 0.25f;
    //constexpr float SZ2 = 0.5f;

    for (int i = 0; i < m_oitBuffer.m_buffer->getDrawBufferCount(); i++) {
        m_oitBuffer.m_buffer->blit(
            targetBuffer,
            GL_COLOR_BUFFER_BIT,
            GL_COLOR_ATTACHMENT0 + i,
            GL_COLOR_ATTACHMENT0,
            { -1.f, -1 + SZ1 + i * SZ1 }, { SZ1, SZ1 });
    }

    for (int i = 0; i < m_gBuffer.m_buffer->getDrawBufferCount(); i++) {
        m_gBuffer.m_buffer->blit(
            targetBuffer,
            GL_COLOR_BUFFER_BIT,
            GL_COLOR_ATTACHMENT0 + i,
            GL_COLOR_ATTACHMENT0,
            { 1 - SZ1, -1 + SZ1 + i * SZ1 }, { SZ1, SZ1 });
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
