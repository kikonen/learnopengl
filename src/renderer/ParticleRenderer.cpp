#include "ParticleRenderer.h"

ParticleRenderer::ParticleRenderer()
{
}

void ParticleRenderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);

    particleShader = shaders.getShader(assets, TEX_PARTICLE);
    particleShader->prepare(assets);
}

void ParticleRenderer::update(const RenderContext& ctx)
{
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
}
