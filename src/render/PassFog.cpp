#include "PassFog.h"

#include "kigl/GLState.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

namespace {
    const std::string SHADER_FOG_PASS{ "screen_fog_pass" };
}

namespace render
{
    PassFog::PassFog()
        : Pass("PassFog")
    { }

    PassFog::~PassFog() = default;

    void PassFog::prepare(const PrepareContext& ctx)
    {
        m_program = Program::get(ProgramRegistry::get().getProgram(SHADER_FOG_PASS));
    }

    void PassFog::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;
    }

    void PassFog::initRender(const RenderContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

        m_enabled = !(ctx.m_forceSolid || !ctx.m_useScreenspaceEffects) &&
            ctx.m_useFog &&
            dbg.m_effectFogEnabled;
    }

    PassContext PassFog::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        startScreenPass(
            ctx,
            true,
            kigl::GLStencilMode::only(STENCIL_FOG, STENCIL_FOG),
            true,
            { GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        src.buffer->bind(ctx);
        m_program->bind();
        m_screenTri.draw();

        stopScreenPass(ctx);

        return src;
    }
}
