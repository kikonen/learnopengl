#include "ParticleRenderer.h"

#include "asset/Shader.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

ParticleRenderer::ParticleRenderer()
{
}

void ParticleRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    particleShader = m_registry->m_shaderRegistry->getShader(TEX_PARTICLE);
    particleShader->prepare(assets);
}

void ParticleRenderer::update(const RenderContext& ctx)
{
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
}
