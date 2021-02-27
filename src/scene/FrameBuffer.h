#pragma once

#include "ki/GL.h"

#include "scene/RenderContext.h"


struct FrameBufferSpecification {
	int width;
	int height;

	bool useMibMap = false;
	bool useStencil = false;

	int internalFormat = GL_RGB8;
	int format = GL_RGB;

	int textureWrap = GL_CLAMP_TO_EDGE;
};

class FrameBuffer
{
public:
	FrameBuffer(const FrameBufferSpecification& spec);
	~FrameBuffer();

	virtual void prepare() = 0;
	void bind(const RenderContext& ctx);
	void unbind(const RenderContext& ctx);

	void bindTexture(const RenderContext& ctx, int unitID);

public:
	FrameBufferSpecification spec;

	unsigned int FBO = -1;
	unsigned int textureID = -1;

protected:
	bool prepared = false;
};

