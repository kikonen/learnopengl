#include "DynamicCubeMap.h"

#include "scene/CubeMap.h"


DynamicCubeMap::DynamicCubeMap(int size)
	: size(size)
{
}

DynamicCubeMap::~DynamicCubeMap()
{
	glDeleteRenderbuffers(1, &depthBuffer);
	glDeleteFramebuffers(1, &FBO);
}

void DynamicCubeMap::bindTexture(const RenderContext& ctx, int unitID)
{
	glActiveTexture(unitID);
	KI_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));
}

void DynamicCubeMap::bind(const RenderContext& ctx)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, size, size);
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
	KI_GL_UNBIND(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

	// Reset viewport back
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, ctx.width, ctx.height);
	ctx.bindUBOs();
}

void DynamicCubeMap::prepare()
{
	{
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}

	{
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		KI_ERROR("FRAMEBUFFER:: Framebuffer is not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	textureID = CubeMap::createEmpty(size);
}
