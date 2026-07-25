#include "PassDebugVolume.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

#include "renderer/VolumeRenderer.h"

#include "scene/Scene.h"

namespace {
}

namespace render
{
    PassDebugVolume::PassDebugVolume()
        : Pass("PassDebugVolume")
    {
    }

    PassDebugVolume::~PassDebugVolume() = default;

    void PassDebugVolume::prepare(const PrepareContext& ctx)
    {
    }

    void PassDebugVolume::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebugVolume::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

        m_enabled = ctx.m_allowDrawDebug &&
            (dbg.m_showVolume || dbg.m_showSelectionVolume);
    }

    PassContext PassDebugVolume::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.getGLState();
        state.setStencil({});

        src.buffer->bind(ctx);

        ctx.getScene()->getVolumeRenderer()->draw(ctx, src.buffer);

        return src;
    }
}
