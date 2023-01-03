#pragma once

#include "FrameBuffer.h"

class WindowBuffer final : public FrameBuffer
{
public:
    WindowBuffer();
    virtual ~WindowBuffer() override = default;

    virtual void bind(const RenderContext& ctx) override;

private:
};
