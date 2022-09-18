#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
    : Renderer(assets)
{
}

void NormalRenderer::prepare(ShaderRegistry& shaders)
{
    normalShader = shaders.getShader(assets, TEX_NORMAL);
    normalShader->prepare();
}

void NormalRenderer::bind(const RenderContext& ctx)
{
}

void NormalRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
    drawNodes(ctx, registry);
}

void NormalRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
    auto shader = normalShader.get();
    for (auto& x : registry.nodes) {
        auto t = x.first;
        t->bind(ctx, shader);

        Batch& batch = t->batch;
        batch.bind(ctx, shader);

        for (auto& e : x.second) {
            if (!e->allowNormals) continue;
            batch.draw(ctx, e, shader);
        }

        batch.flush(ctx, t);
    }
}
