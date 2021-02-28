#include "FrameBuffer.h"

#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"

FrameBuffer::FrameBuffer(const FrameBufferSpecification& spec)
	: spec(spec)
{
}

FrameBuffer::~FrameBuffer()
{
	if (FBO != -1) {
		glDeleteFramebuffers(1, &FBO);
	}
}

void FrameBuffer::bind(const RenderContext& ctx)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, spec.width, spec.height);
}

void FrameBuffer::unbind(const RenderContext& ctx)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, ctx.width, ctx.height);
}

void FrameBuffer::bindTexture(const RenderContext& ctx, int unitIndex)
{
	glBindTextures(unitIndex, 1, &textureID);
}
