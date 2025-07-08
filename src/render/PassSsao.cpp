#include "PassSsao.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

#include "SsaoBuffer.h"

namespace {
    const std::string SHADER_SSAO_PASS{ "screen_ssao_pass" };
    const std::string SHADER_SSAO_BLUR_PASS{ "screen_ssao_blur_pass" };
}

namespace render
{
    PassSsao::PassSsao()
        : Pass("PassSsao")
    {
    }

    PassSsao::~PassSsao() = default;

    void PassSsao::prepare(const PrepareContext& ctx)
    {
        m_ssaoProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_SSAO_PASS));
        m_ssaoBlurProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_SSAO_BLUR_PASS));

        m_ssaoBuffer.prepare();
    }

    void PassSsao::updateRT(
        const UpdateViewContext& ctx,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;

        m_ssaoBuffer.updateRT(ctx, bufferScale);
    }

    void PassSsao::cleanup(const RenderContext& ctx)
    {
        m_ssaoBuffer.invalidateAll();
    }

    void PassSsao::initRender(const RenderContext& ctx)
    {
        auto& state = ctx.m_state;
        const auto& dbg = *ctx.m_dbg;

        state.setStencil({});

        m_enabled = !(ctx.m_forceSolid || !ctx.m_useScreenspaceEffects) &&
            ctx.m_useSsao &&
            dbg.m_effectSsaoEnabled;

        m_ssaoBuffer.clearAll();
    }

    PassContext PassSsao::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;

        startScreenPass(
            ctx,
            false,
            {},
            false,
            {});

        m_ssaoBuffer.bind(ctx);

        m_ssaoProgram->bind();
        m_ssaoBuffer.m_buffer->setDrawBuffer(SsaoBuffer::ATT_SSAO_INDEX);
        m_screenTri.draw();

        m_ssaoBlurProgram->bind();
        m_ssaoBuffer.bindSsaoTexture(state);
        m_ssaoBuffer.m_buffer->setDrawBuffer(SsaoBuffer::ATT_SSAO_BLUR_INDEX);
        m_screenTri.draw();

        m_ssaoBuffer.bindSsaoBlurTexture(state);

        stopScreenPass(ctx);

        return src;
    }
}
