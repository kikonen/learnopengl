#pragma once

#include "FrameBuffer.h"


class ShadowBuffer final : public FrameBuffer
{
public:
	ShadowBuffer(const FrameBufferSpecification& spec);
	~ShadowBuffer() override = default;

public:
};

