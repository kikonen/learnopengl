#include "PhysicsRenderer.h"

#include "render/DebugContext.h"

namespace {
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
