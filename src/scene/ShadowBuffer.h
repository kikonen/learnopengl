#pragma once

#include "FrameBuffer.h"


class ShadowBuffer final : public FrameBuffer
{
public:
	ShadowBuffer(int width, int height);
	~ShadowBuffer();

	void prepare() override;

public:
};

