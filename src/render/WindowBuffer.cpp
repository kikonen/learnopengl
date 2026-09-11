#include "WindowBuffer.h"

#include "glm/glm.hpp"

#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"

namespace render {
    WindowBuffer::WindowBuffer(
        GLuint fbo,
        bool forceBind,
        bool srgbEnabled)
        : FrameBuffer("window", { 0, 0, {} }),
        m_srgbEnabled{ srgbEnabled }
    {
        m_fbo = fbo;
        m_forceBind = forceBind;

        //{
        //    auto albedo = FrameBufferAttachment::getDrawBuffer(GL_COLOR_ATTACHMENT0);
        //    albedo.clearType = ClearType::FLOAT;
        //    albedo.externalDelete = true;

        //    m_spec.attachments.emplace_back(albedo);
        //}
    }

    void WindowBuffer::updateView(const UpdateViewContext& ctx)
    {
        const auto& res = ctx.m_resolution;
        int w = res.x;
        int h = res.y;
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_spec.width || h != m_spec.height;
        if (!changed) return;

        m_spec.width = w;
        m_spec.height = h;

        m_bufferInfo.u_bufferResolution = { m_spec.width, m_spec.height };
    }
}
