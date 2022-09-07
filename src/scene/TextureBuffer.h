#pragma once

#include "FrameBuffer.h"

class TextureBuffer final : public FrameBuffer
{
public:
    TextureBuffer(const FrameBufferSpecification& spec);
    virtual ~TextureBuffer() override = default;

public:
};

