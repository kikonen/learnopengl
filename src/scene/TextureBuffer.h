#pragma once

#include "FrameBuffer.h"

class TextureBuffer final : public FrameBuffer
{
public:
	TextureBuffer(const FrameBufferSpecification& spec);
	~TextureBuffer() override = default;

public:
};

