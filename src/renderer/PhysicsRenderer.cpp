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

    //m_programId = ProgramRegistry::get().getProgram("tex");
    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void PhysicsRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = render::DebugContext::get();
    if (!dbg.m_physicsShowObjects) return;

    const auto meshes = dbg.m_physicsMeshesRT;
    if (!meshes || meshes->empty()) return;

    drawObjects(ctx, targetBuffer, *meshes);
}
