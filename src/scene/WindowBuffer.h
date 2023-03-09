#pragma once

#include "FrameBuffer.h"

class WindowBuffer final : public FrameBuffer
{
public:
    WindowBuffer() : WindowBuffer(0) {}

    WindowBuffer(int fbo);

    virtual ~WindowBuffer() override = default;

    void update(const RenderContext& ctx);

private:
};
