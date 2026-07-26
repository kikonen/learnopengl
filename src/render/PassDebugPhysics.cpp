#include "PassDebugPhysics.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

#include "renderer/PhysicsRenderer.h"

#include "scene/Scene.h"

namespace {
}

namespace render
{
    PassDebugPhysics::PassDebugPhysics()
        : Pass("PassDebugPhysics")
    {
    }

    PassDebugPhysics::~PassDebugPhysics() = default;

    void PassDebugPhysics::prepare(const PrepareContext& ctx)
    {
    }

    void PassDebugPhysics::updateView(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebugPhysics::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.getDebug();
        const auto& physicsDbg = dbg.m_physics;

        m_enabled = ctx.m_allowDrawDebug &&
            physicsDbg.m_showObjects;
    }

    PassContext PassDebugPhysics::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.getGLState();
        state.setStencil({});

        src.buffer->bind(ctx);

        ctx.getScene()->getPhysicsRenderer()->draw(ctx, src.buffer);

        return src;
    }
}
