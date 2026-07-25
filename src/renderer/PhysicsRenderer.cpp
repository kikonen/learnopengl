#include "PhysicsRenderer.h"

#include "debug/DebugContext.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

namespace {
}

void PhysicsRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    //m_programId = ProgramRegistry::get().getProgram("tex");
    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void PhysicsRenderer::updateImpl(
    const render::RenderContext& ctx)
{
    const auto& dbg = debug::DebugContext::get();
    const auto& physicsDbg = dbg.m_physics;

    if (!physicsDbg.m_showObjects) return;

    const auto meshes = physicsDbg.m_meshesRT.load();
    if (!meshes || meshes->empty()) return;

    updateMeshes(ctx, *meshes);
}
