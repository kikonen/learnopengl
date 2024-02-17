#include "ParticleRenderer.h"

#include "asset/Assets.h"
#include "asset/Program.h"
#include "asset/Shader.h"

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
#include "registry/ProgramRegistry.h"
#include "registry/ModelRegistry.h"

namespace {
    const std::string QUAD_MESH_NAME{ "quad" };
    mesh::LodMesh lodMesh;
    kigl::GLVertexArray* vao{ nullptr };
}

void ParticleRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    const auto& assets = ctx.m_assets;

    Renderer::prepareRT(ctx);

    particleProgram = ProgramRegistry::get().getProgram(SHADER_PARTICLE);
    particleProgram->prepareRT();

    auto future = ModelRegistry::get().getMesh(
        QUAD_MESH_NAME,
        assets.modelsDir);
    auto* mesh = future.get();
    vao = mesh->prepareRT(ctx);
    mesh->prepareLod(lodMesh);
}

void ParticleRenderer::render(
    const RenderContext& ctx)
{
    auto& state = ctx.m_state;

    const auto instanceCount = particle::ParticleSystem::get().getParticleCount();
    if (instanceCount == 0) return;

    state.setEnabled(GL_BLEND, true);
    state.setEnabled(GL_CULL_FACE, false);

    GLuint firstIndex = lodMesh.m_lod.m_baseIndex;
    GLuint baseVertex = lodMesh.m_lod.m_baseVertex;

    kigl::GLState::get().bindVAO(*vao);

    particleProgram->bind();

    glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES,
        lodMesh.m_lod.m_indexCount,
        GL_UNSIGNED_INT,
        (void*)(firstIndex * sizeof(GLuint)),
        instanceCount,
        baseVertex,
        0);

    ctx.bindDefaults();
}
