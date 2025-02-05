#include "Pass.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "kigl/GLState.h"

#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"

namespace render
{
    class FrameBuffer;

    Pass::Pass(const std::string& name)
        : m_name{ name },
        m_textureQuad{ render::TextureQuad::get() },
        m_screenTri{ render::ScreenTri::get() }
    { }

    Pass::~Pass() = default;

    void Pass::prepare(const PrepareContext& ctx)
    {
    }

    void Pass::updateRT(const UpdateViewContext& ctx)
    {
    }

    void Pass::cleanup(const RenderContext& ctx)
    { }

    void Pass::initRender(const RenderContext& ctx)
    {}

    render::PassContext Pass::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        KI_INFO(fmt::format("PASS: name={}, w={}, h={}", m_name, m_width, m_height));
        return src;
    }

    bool Pass::updateSize(const UpdateViewContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

        const auto& res = ctx.m_resolution;
        const auto bufferScale = dbg.getGBufferScale();

        int w = (int)(bufferScale * res.x);
        int h = (int)(bufferScale * res.y);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        const bool changed = w != m_width || h != m_height;
        if (!changed) return false;

        KI_INFO(fmt::format("SIZE_CHANGED: name={}, w={}, h={}", m_name, w, h));

        m_width = w;
        m_height = h;

        return true;
    }

    void Pass::startScreenPass(
        const RenderContext& ctx,
        bool useStencil,
        const kigl::GLStencilMode& stencil,
        bool useBlend,
        const kigl::GLBlendMode& blend)
    {
        auto& state = ctx.m_state;

        // NOTE KI do NOT modify depth with screenpass
        state.setEnabled(GL_DEPTH_TEST, false);
        state.setDepthMask(GL_FALSE);

        // NOTE KI ensure "normal" draw mode is in use
        state.frontFace(GL_CCW);
        state.polygonFrontAndBack(GL_FILL);

        if (useStencil) {
            state.setStencil(stencil);
        }
        else {
            state.setStencil({});
        }

        state.setEnabled(GL_BLEND, useBlend);
        if (useBlend) {
            state.setBlendMode(blend);
        } else {
            state.setBlendMode({});
        }
    }

    void Pass::stopScreenPass(
        const RenderContext& ctx)
    {
        auto& state = ctx.m_state;

        state.setEnabled(GL_DEPTH_TEST, true);
        state.setDepthMask(GL_TRUE);

        state.setStencil({});
        state.setEnabled(GL_BLEND, false);
    }
}
