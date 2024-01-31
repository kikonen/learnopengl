#include "ParticleRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"


void ParticleRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    particleProgram = ProgramRegistry::get().getProgram(SHADER_PARTICLE);
    particleProgram->prepareRT();
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
}
