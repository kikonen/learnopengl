#include "NodeDraw.h"

#include "asset/Program.h"

#include "component/Camera.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"


void NodeDraw::prepare(const Assets& assets)
{
    m_gbuffer.prepare(assets);
}

void NodeDraw::update(const RenderContext& ctx)
{
    m_gbuffer.update(ctx);
}

void NodeDraw::drawNodes(
    const RenderContext& ctx,
    bool includeBlended,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
{
//    m_gbuffer.bind(ctx);

    // phase 1 - render to G-buffer
    // => nodes supporting G-buffer
    drawNodesImpl(ctx, includeBlended, typeSelector, nodeSelector);
    ctx.m_batch->flush(ctx);

    // phase 2 - render to targetBuffer

    // phase 3 - render to targetBuffer
    // => nodes NOT supporting G-buffer

    // phase 4 - render BLENDED to targetBuffer
}

void NodeDraw::drawBlended(
    const RenderContext& ctx,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
{
    drawBlendedImpl(ctx, typeSelector, nodeSelector);
    ctx.m_batch->flush(ctx);
}

void NodeDraw::drawNodesImpl(
    const RenderContext& ctx,
    bool includeBlended,
    std::function<bool(const MeshType*)> typeSelector,
    std::function<bool(const Node*)> nodeSelector)
{
    auto* nodeRegistry = ctx.m_registry->m_nodeRegistry.get();

    auto renderTypes = [this, &ctx, &typeSelector, &nodeSelector](const MeshTypeMap& typeMap) {
        auto program = typeMap.begin()->first.type->m_program;

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

    if (includeBlended)
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
