#include "FrameBuffer.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

FrameBuffer::FrameBuffer(int w, int h)
	: w(w), h(h)
{
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::prepare()
{
	if (prepared) {
		return;
	}
	prepared = true;

	glGenFramebuffers(1, &FBO);

	// depth map
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::bind()
{
	glViewport(0, 0, w, h);
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
