#pragma once

#include "FrameBuffer.h"

class WindowBuffer final : public FrameBuffer
{
public:
    WindowBuffer();
    virtual ~WindowBuffer() override = default;

    void update(const RenderContext& ctx);

private:
};
