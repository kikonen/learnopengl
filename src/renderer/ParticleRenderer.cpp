#include "ParticleRenderer.h"

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "engine/PrepareContext.h"

#include "mesh/ModelMesh.h"
#include "mesh/LodMesh.h"

#include "render/RenderContext.h"

#include "backend/gl/DrawElementsIndirectCommand.h"

#include "particle/ParticleSystem.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"

namespace {
}

void ParticleRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    const auto& assets = ctx.m_assets;
    m_enabled = assets.particleEnabled;

    Renderer::prepareRT(ctx);

    m_particleProgram = Program::get(ProgramRegistry::get().getProgram(
        SHADER_PARTICLE,
        { { DEF_USE_ALPHA, "1" },
          { DEF_USE_BLEND, "1" } }));

    m_particleProgram->prepareRT();
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
    if (!isEnabled()) return;

    auto& state = ctx.m_state;

    const auto instanceCount = particle::ParticleSystem::get().getActiveParticleCount();
    if (instanceCount == 0) return;

    state.setDepthMask(GL_FALSE);
    state.setEnabled(GL_BLEND, true);
    //state.setEnabled(GL_CULL_FACE, false);

    //state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

    // https://stackoverflow.com/questions/31850635/opengl-additive-blending-get-issue-when-no-background
    //state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_DST_ALPHA });
    state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ZERO, GL_DST_ALPHA });
    //glBlendFunc(GL_ONE, GL_ONE);

    m_particleProgram->bind();

    glDrawArraysInstanced(
        GL_POINTS,
        0,
        1,
        instanceCount);

    state.setDepthMask(GL_TRUE);
    state.setEnabled(GL_BLEND, false);
}
