#include "ParticleRenderer.h"

#include "asset/Program.h"
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

    particleProgram = m_registry->m_programRegistry->getProgram(SHADER_PARTICLE);
    particleProgram->prepare(assets);
}

void ParticleRenderer::update(const RenderContext& ctx)
{
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
}
