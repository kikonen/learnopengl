#include "NormalRenderer.h"

#include "asset/ShaderBind.h"

NormalRenderer::NormalRenderer()
{
}

void NormalRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);

    m_normalShader = shaders.getShader(assets, TEX_NORMAL);
    m_normalShader->prepare(assets);
}

void NormalRenderer::render(
    const RenderContext& ctx)
{
    drawNodes(ctx);
}

void NormalRenderer::drawNodes(const RenderContext& ctx)
{
    ShaderBind bound(m_normalShader);

    auto renderTypes = [this, &ctx, &bound](const NodeTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto& type = *it.first;

            auto& batch = ctx.m_batch;

            type.bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            for (auto& node : it.second) {
                if (!node->m_allowNormals) continue;
                batch.draw(ctx, *node, bound.shader);
            }

            batch.flush(ctx, type);
            type.unbind(ctx);
        }
    };

    for (const auto& all : ctx.registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.registry.alphaNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.registry.blendedNodes) {
        renderTypes(all.second);
    }
}
