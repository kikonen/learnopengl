#pragma once

#include "FrameBuffer.h"

class TextureBuffer final : public FrameBuffer
{
public:
	TextureBuffer(const FrameBufferSpecification& spec);
	~TextureBuffer();

	void prepare() override;

public:
	unsigned int RBO = -1;
};

