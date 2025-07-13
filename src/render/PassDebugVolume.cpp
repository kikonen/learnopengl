#include "PassDebugVolume.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

#include "renderer/VolumeRenderer.h"

namespace {
}

namespace render
{
    PassDebugVolume::PassDebugVolume()
        : Pass("PassDebugVolume"),
        m_volumeRenderer{ std::make_unique<VolumeRenderer>() }
    {
    }

    PassDebugVolume::~PassDebugVolume() = default;

    void PassDebugVolume::prepare(const PrepareContext& ctx)
    {
        m_volumeRenderer->prepareRT(ctx);
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
        const auto& dbg = *ctx.m_dbg;

        m_enabled = ctx.m_allowDrawDebug &&
            (dbg.m_showVolume || dbg.m_showSelectionVolume);
    }

    PassContext PassDebugVolume::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        src.buffer->bind(ctx);

        m_volumeRenderer->render(ctx, src.buffer);

        return src;
    }
}
