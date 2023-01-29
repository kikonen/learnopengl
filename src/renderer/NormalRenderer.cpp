#include "NormalRenderer.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

NormalRenderer::NormalRenderer()
{
}

void NormalRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_normalShader = m_registry->m_shaderRegistry->getShader(TEX_NORMAL);
    m_normalShader->prepare(assets);
}

void NormalRenderer::render(
    const RenderContext& ctx)
{
    drawNodes(ctx);
}

void NormalRenderer::drawNodes(const RenderContext& ctx)
{
    auto shader = m_normalShader;

    auto renderTypes = [this, &ctx, &shader](const MeshTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto& type = *it.first.type;
            auto& batch = ctx.m_batch;

            for (auto& node : it.second) {
                if (!node->m_allowNormals) continue;
                batch->draw(ctx, *node, shader);
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

    ctx.m_batch->flush(ctx);
}
