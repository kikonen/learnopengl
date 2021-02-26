#pragma once

#include "ki/GL.h"

#include "scene/RenderContext.h"

class FrameBuffer
{
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();

	virtual void prepare() = 0;
	void bind(const RenderContext& ctx);
	void unbind(const RenderContext& ctx);

	void bindTexture(int unitID);

public:
	const int width;
	const int height;

	unsigned int FBO = -1;
	unsigned int textureID = -1;

protected:
	bool prepared = false;
};

