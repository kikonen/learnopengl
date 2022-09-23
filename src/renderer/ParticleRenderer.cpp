#include "ParticleRenderer.h"

ParticleRenderer::ParticleRenderer()
{
}

void ParticleRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    Renderer::prepare(assets, shaders);

    particleShader = shaders.getShader(assets, TEX_PARTICLE);
    particleShader->prepare(assets);
}

void ParticleRenderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
}

void ParticleRenderer::bind(const RenderContext& ctx)
{
}

void ParticleRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
}
