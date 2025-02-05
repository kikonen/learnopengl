#include "PassParticle.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

#include "renderer/ParticleRenderer.h"

namespace {
}

namespace render
{
    PassParticle::PassParticle()
        : Pass("PassParticle"),
        m_particleRenderer{ std::make_unique < ParticleRenderer>(true) }
    {
    }

    PassParticle::~PassParticle() = default;

    void PassParticle::prepare(const PrepareContext& ctx)
    {
        m_particleRenderer->prepareRT(ctx);
    }

    void PassParticle::updateRT(const UpdateViewContext& ctx)
    {
        if (!updateSize(ctx)) return;
    }

    void PassParticle::initRender(const RenderContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

        m_enabled = ctx.m_useParticles &&
            !(ctx.m_forceSolid) &&
            dbg.m_particleEnabled;
    }

    PassContext PassParticle::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;
        state.setStencil({});

        src.buffer->bind(ctx);
        m_particleRenderer->render(ctx);

        return src;
    }
}
