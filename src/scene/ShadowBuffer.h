#pragma once

#include "FrameBuffer.h"


class ShadowBuffer final : public FrameBuffer
{
public:
    ShadowBuffer(const FrameBufferSpecification& spec);
    virtual ~ShadowBuffer() override = default;

public:
};

