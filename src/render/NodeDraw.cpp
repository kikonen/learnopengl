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
    m_gbuffer.prepare(assets);
    m_oitbuffer.prepare(assets);
    m_quad.prepare();

    m_deferredProgram = registry->m_programRegistry->getProgram(SHADER_DEFERRED_PASS);
    m_deferredProgram ->prepare(assets);

    m_oitProgram = registry->m_programRegistry->getProgram(SHADER_OIT_PASS);
    m_oitProgram->prepare(assets);
}

void NodeDraw::updateView(const RenderContext& ctx)
{
    m_gbuffer.updateView(ctx);
    m_oitbuffer.updateView(ctx);
}

void NodeDraw::clear(
    const RenderContext& ctx,
    GLbitfield clearMask,
    const glm::vec4& clearColor)
{
    m_gbuffer.bind(ctx);
    m_gbuffer.m_buffer->clear(ctx, clearMask, clearColor);

    m_oitbuffer.bind(ctx);
    m_oitbuffer.m_buffer->clear(ctx, clearMask, clearColor);
}

void NodeDraw::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    const std::function<bool(const MeshType*)>& typeSelector,
    const std::function<bool(const Node*)>& nodeSelector,
    GLbitfield clearMask,
    const glm::vec4& clearColor)
{
    // NOTE KI no blend in G-buffer
    auto wasAllowBlend = ctx.pushAllowBlend(false);

    // pass 1 - geometry
    // => nodes supporting G-buffer
    {
        m_gbuffer.bind(ctx);
        m_gbuffer.m_buffer->clear(ctx, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, clearColor);

        drawNodesImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return type->m_flags.gbuffer && typeSelector(type); },
            nodeSelector);

        ctx.m_batch->flush(ctx);
    }

    // pass 1 - blend OIT
    {
        m_oitbuffer.bind(ctx);
        m_oitbuffer.m_buffer->clear(ctx, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, { 0, 0, 0, 0});

        // only "blend OIT" nodes
        drawProgram(
            ctx,
            m_oitProgram,
            nullptr,
            [&typeSelector](const MeshType* type) { return type->m_flags.blendOIT && typeSelector(type); },
            nodeSelector);

        ctx.m_batch->flush(ctx);
    }

    // pass 2 - light
    {
        targetBuffer->bind(ctx);
        targetBuffer->clear(ctx, clearMask, clearColor);

        m_deferredProgram->bind(ctx.m_state);
        m_gbuffer.bindTexture(ctx);
        m_quad.draw(ctx);
    }

    // pass 3 - non G-buffer nodes
    // => for example, "skybox"
    // => separate light calculations
    {
        m_gbuffer.m_buffer->blit(targetBuffer, GL_DEPTH_BUFFER_BIT, { -1.f, 1.f }, { 2.f, 2.f });

        drawNodesImpl(
            ctx,
            [&typeSelector](const MeshType* type) { return !type->m_flags.gbuffer && typeSelector(type); },
            nodeSelector);

        ctx.m_batch->flush(ctx);
    }

    ctx.pushAllowBlend(wasAllowBlend);

    // pass 4 - blend
    // => separate light calculations
    {
        targetBuffer->bind(ctx);
        drawBlendedImpl(ctx, typeSelector, nodeSelector);
        ctx.m_batch->flush(ctx);
    }

    {
        constexpr float SZ = 0.25f;

        for (int i = 0; i < m_oitbuffer.m_buffer->getDrawBufferCount(); i++) {
            m_oitbuffer.m_buffer->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0 + i,
                GL_COLOR_ATTACHMENT0,
                { -1.f, -0.75f + i * SZ }, { SZ, SZ });
        }

        for (int i = 0; i < m_gbuffer.m_buffer->getDrawBufferCount(); i++) {
            m_gbuffer.m_buffer->blit(
                targetBuffer,
                GL_COLOR_BUFFER_BIT,
                GL_COLOR_ATTACHMENT0 + i,
                GL_COLOR_ATTACHMENT0,
                { 0.75f, -0.75f + i * SZ }, { SZ, SZ });
        }
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
