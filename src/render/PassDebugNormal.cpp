#include "PassDebugNormal.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/CollectionRender.h"
#include "render/Batch.h"

#include "renderer/NormalRenderer.h"

namespace {
}

namespace render
{
    PassDebugNormal::PassDebugNormal()
        : Pass("PassDebugNormal"),
        m_normalRenderer{ std::make_unique<NormalRenderer>(false) }
    {
    }

    PassDebugNormal::~PassDebugNormal() = default;

    void PassDebugNormal::prepare(const PrepareContext& ctx)
    {
        m_normalRenderer->prepareRT(ctx);
        m_normalRenderer->setEnabled(true);
    }

    void PassDebugNormal::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassDebugNormal::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.m_dbg;

        m_enabled = ctx.m_allowDrawDebug &&
            dbg.m_showNormals;
    }

    PassContext PassDebugNormal::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        src.buffer->bind(ctx);

        m_normalRenderer->render(ctx, src.buffer);

        return src;
    }
}
