#include "WindowBuffer.h"

#include <iostream>

#include "glm/glm.hpp"

#include "scene/RenderContext.h"


WindowBuffer::WindowBuffer(GLuint fbo)
    : FrameBuffer({ 0, 0, {} })
{
    m_fbo = fbo;
}

void WindowBuffer::update(const RenderContext& ctx)
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
}
