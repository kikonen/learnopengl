#include "ParticleRenderer.h"

ParticleRenderer::ParticleRenderer(const Assets& assets)
    : Renderer(assets)
{
}

void ParticleRenderer::prepare(ShaderRegistry& shaders)
{
    Renderer::prepare(shaders);

    particleShader = shaders.getShader(assets, TEX_PARTICLE);
    particleShader->prepare();
}

void ParticleRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
}

void ParticleRenderer::bind(const RenderContext& ctx)
{
}

void ParticleRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
}
