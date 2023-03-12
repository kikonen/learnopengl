#include "NodeDraw.h"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "component/Camera.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"


void NodeDraw::prepare(
    const Assets& assets,
    Registry* registry)
{
    m_gbuffer.prepare(assets);

    m_deferredProgram = registry->m_programRegistry->getProgram(SHADER_DEFERRED_PASS);
    m_deferredProgram ->prepare(assets);
}

void NodeDraw::update(const RenderContext& ctx)
{
    m_gbuffer.update(ctx);
    prepareQuad();
}

void NodeDraw::clear(const RenderContext& ctx, const glm::vec4& clearColor)
{
    m_gbuffer.bind(ctx);
    m_gbuffer.m_buffer->clear(ctx, clearColor);
}

void NodeDraw::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    bool includeBlended,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector,
    bool clearTarget,
    const glm::vec4& clearColor)
{
    // pass 1 - geometry
    // => nodes supporting G-buffer
    if (clearTarget) {
        m_gbuffer.bind(ctx);
        m_gbuffer.m_buffer->clear(ctx, clearColor);

        drawNodesImpl(ctx, includeBlended, true, typeSelector, nodeSelector);
        ctx.m_batch->flush(ctx);
    }

    // pass 2 - light
    {
        targetBuffer->bind(ctx);
        if (clearTarget) {
            targetBuffer->clear(ctx, clearColor);
        }
        m_deferredProgram->bind(ctx.state);
        m_gbuffer.bindTexture(ctx);
        drawQuad(ctx);
    }

    // pass 3 - non G-buffer nodes
    {
        m_gbuffer.m_buffer->blit(targetBuffer, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, { -1.f, 1.f }, { 2.f, 2.f });

        drawNodesImpl(ctx, includeBlended, false, typeSelector, nodeSelector);
        ctx.m_batch->flush(ctx);
    }

    // pass 4 - blend
    {
    }
}

void NodeDraw::drawBlended(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
{
    targetBuffer->bind(ctx);
    drawBlendedImpl(ctx, typeSelector, nodeSelector);
    ctx.m_batch->flush(ctx);
}

void NodeDraw::drawNodesImpl(
    const RenderContext& ctx,
    bool includeBlended,
    bool useGBuffer,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
{
    auto* nodeRegistry = ctx.m_registry->m_nodeRegistry.get();

    auto renderTypes = [this, &ctx, useGBuffer, &typeSelector, &nodeSelector](const MeshTypeMap& typeMap) {
        auto program = typeMap.begin()->first.type->m_program;

        for (const auto& it : typeMap) {
            auto* type = it.first.type;

            if (useGBuffer != type->m_flags.gbuffer) continue;
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

    if (!useGBuffer && includeBlended)
    {
        // NOTE KI do not try blend here; end result is worse than not doing blend at all (due to stencil)
        for (const auto& all : nodeRegistry->blendedNodes) {
            renderTypes(all.second);
        }
    }
}

void NodeDraw::drawBlendedImpl(
    const RenderContext& ctx,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
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
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
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

void NodeDraw::drawQuad(const RenderContext& ctx)
{
    ctx.state.bindVAO(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //glBindVertexArray(0);
}

void NodeDraw::prepareQuad()
{
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // setup plane VAO
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}
