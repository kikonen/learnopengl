#include "NormalRenderer.h"


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

void NormalRenderer::render(const RenderContext& ctx, const NodeRegistry& registry)
{
    drawNodes(ctx, registry);
}

void NormalRenderer::drawNodes(const RenderContext& ctx, const NodeRegistry& registry)
{
    auto shader = normalShader;
    shader->bind();
    for (const auto& x : registry.nodes) {
        auto& t = x.first;
        t->bind(ctx, shader);

        Batch& batch = t->batch;
        batch.bind(ctx, shader);

        for (auto& e : x.second) {
            if (!e->allowNormals) continue;
            batch.draw(ctx, e, shader);
        }

        batch.flush(ctx, t);
    }
    shader->unbind();
}
