#include "PhysicsRenderer.h"

#include "render/DebugContext.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

namespace {
}

void PhysicsRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    m_programId = ProgramRegistry::get().getProgram("g_tex");
    Program::get(m_programId)->prepareRT();
}

void PhysicsRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = render::DebugContext::get();
    if (!dbg.m_physicsShowObjects) return;

    const auto meshes = dbg.m_physicsMeshes;
    if (!meshes || meshes->empty()) return;

    drawObjects(ctx, targetBuffer, *meshes);
}
