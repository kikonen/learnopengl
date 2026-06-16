#include "PassDebugEnvironmentProbe.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

#include "renderer/EnvironmentProbeRenderer.h"

namespace {
}

namespace render
{
    PassDebugEnvironmentProbe::PassDebugEnvironmentProbe()
        : Pass("PassDebugEnvironmentProbe"),
        m_cubeMapRenderer{ std::make_unique<EnvironmentProbeRenderer>(true, false) },
        m_environmentProbeRenderer{ std::make_unique<EnvironmentProbeRenderer>(false, true) }
    {
    }

    PassDebugEnvironmentProbe::~PassDebugEnvironmentProbe() = default;

    void PassDebugEnvironmentProbe::prepare(const PrepareContext& ctx)
    {
        m_cubeMapRenderer->prepareRT(ctx);
        m_environmentProbeRenderer->prepareRT(ctx);
    }

    void PassDebugEnvironmentProbe::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebugEnvironmentProbe::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

        m_enabled = ctx.m_allowDrawDebug &&
            dbg.m_showEnvironmentProbe;
    }

    PassContext PassDebugEnvironmentProbe::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.getGLState();
        state.setStencil({});

        src.buffer->bind(ctx);

        m_cubeMapRenderer->render(ctx, src.buffer);
        m_environmentProbeRenderer->render(ctx, src.buffer);

        return src;
    }
}
