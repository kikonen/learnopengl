#include "NormalRenderer.h"

#include "asset/ShaderBind.h"

NormalRenderer::NormalRenderer()
{
}

void NormalRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    Renderer::prepare(assets, shaders);

    normalShader = shaders.getShader(assets, TEX_NORMAL);
    normalShader->prepare(assets);
}

void NormalRenderer::bind(const RenderContext& ctx)
{
}

void NormalRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    drawNodes(ctx, registry);
}

void NormalRenderer::drawNodes(const RenderContext& ctx, const NodeRegistry& registry)
{
    ShaderBind bound(normalShader);

    auto renderTypes = [this, &ctx, &bound](const NodeTypeMap& typeMap) {
        for (const auto& [type, nodes] : typeMap) {
            Batch& batch = type->batch;

            type->bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            for (auto& node : nodes) {
                if (!node->allowNormals) continue;
                batch.draw(ctx, node, bound.shader);
            }

            batch.flush(ctx, type);
            type->unbind(ctx);
        }
    };

    for (const auto& all : registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : registry.alphaNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : registry.blendedNodes) {
        renderTypes(all.second);
    }
}
