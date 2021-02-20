#pragma once

#include "FrameBuffer.h"

class TextureBuffer final : public FrameBuffer
{
public:
	TextureBuffer(int width, int height);
	~TextureBuffer();

	void prepare() override;

public:
	unsigned int RBO = -1;
};

