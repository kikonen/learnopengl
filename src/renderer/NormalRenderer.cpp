#include "NormalRenderer.h"


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
    auto shader = m_normalShader;

    auto renderTypes = [this, &ctx, &shader](const MeshTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto& type = *it.first;
            auto& batch = ctx.m_batch;

            for (auto& node : it.second) {
                if (!node->m_allowNormals) continue;
                batch.draw(ctx, *node, shader);
            }
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

    ctx.m_batch.flush(ctx);
}
