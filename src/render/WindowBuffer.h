#pragma once

#include "FrameBuffer.h"

class UpdateViewContext;

class WindowBuffer final : public FrameBuffer
{
public:
    WindowBuffer(bool forceBind) : WindowBuffer(0, forceBind) {}

    WindowBuffer(GLuint fbo, bool forceBind);

    virtual ~WindowBuffer() override {};

    void updateView(const UpdateViewContext& ctx);

private:
};
