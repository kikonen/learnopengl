#pragma once

#include "FrameBuffer.h"

class WindowBuffer final : public FrameBuffer
{
public:
    WindowBuffer(bool forceBind) : WindowBuffer(0, forceBind) {}

    WindowBuffer(GLuint fbo, bool forceBind);

    virtual ~WindowBuffer() override = default;

    void updateView(const RenderContext& ctx);

private:
};
