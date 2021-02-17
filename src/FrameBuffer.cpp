#include "FrameBuffer.h"

#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"

FrameBuffer::FrameBuffer(int width, int height)
	: width(width), height(height)
{
}

FrameBuffer::~FrameBuffer()
{
	if (FBO != -1) {
		glDeleteFramebuffers(1, &FBO);
	}
}

void FrameBuffer::bind()
{
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void FrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::bindTexture(int unitID)
{
	glActiveTexture(unitID);
	glBindTexture(GL_TEXTURE_2D, textureID);
}
