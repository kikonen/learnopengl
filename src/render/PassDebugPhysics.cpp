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

namespace {
}

namespace render
{
    PassDebugPhysics::PassDebugPhysics()
        : Pass("PassDebugPhysics"),
        m_physicsRenderer{ std::make_unique<PhysicsRenderer>() }
    {
    }

    PassDebugPhysics::~PassDebugPhysics() = default;

    void PassDebugPhysics::prepare(const PrepareContext& ctx)
    {
        m_physicsRenderer->prepareRT(ctx);
    }

    void PassDebugPhysics::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebugPhysics::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.m_dbg;

        m_enabled = ctx.m_allowDrawDebug &&
            dbg.m_physicsShowObjects;
    }

    PassContext PassDebugPhysics::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        src.buffer->bind(ctx);

        m_physicsRenderer->render(ctx, src.buffer);

        return src;
    }
}
