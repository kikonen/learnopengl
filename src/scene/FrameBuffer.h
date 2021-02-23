#pragma once

#include "ki/GL.h"

class FrameBuffer
{
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();

	virtual void prepare() = 0;
	void bind();
	void unbind();

	void bindTexture(int unitID);

public:
	const int width;
	const int height;

	unsigned int FBO = -1;
	unsigned int textureID = -1;

protected:
	bool prepared = false;
};

