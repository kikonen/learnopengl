#pragma once

#include "FrameBuffer.h"

class WindowBuffer final : public FrameBuffer
{
public:
    WindowBuffer() : WindowBuffer(0) {}

    WindowBuffer(GLuint fbo);

    virtual ~WindowBuffer() override = default;

    void updateView(const RenderContext& ctx);

private:
};
