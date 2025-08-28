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

void PhysicsRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();
    const auto& physicsDbg = dbg.m_physics;

    if (!physicsDbg.m_showObjects) return;

    const auto meshes = physicsDbg.m_meshesRT.load();
    if (!meshes || meshes->empty()) return;

    drawObjects(ctx, targetBuffer, *meshes);
}
