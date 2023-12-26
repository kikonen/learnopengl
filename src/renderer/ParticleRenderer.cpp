#include "ParticleRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"


void ParticleRenderer::prepareView(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareView(assets, registry);

    particleProgram = m_registry->m_programRegistry->getProgram(SHADER_PARTICLE);
    particleProgram->prepareView(assets);
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
}
