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
	if (!prepared) return;

	glDeleteFramebuffers(1, &FBO);

	for (auto& att : spec.attachments) {
		if (att.textureID != -1) {
			glDeleteTextures(1, &att.textureID);
		}
		if (att.RBO != -1) {
			glDeleteRenderbuffers(1, &att.RBO);
		}
	}

}

void FrameBuffer::prepare()
{
	if (prepared) return;
	prepared = true;

	{
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}

	for (auto& att : spec.attachments) {
		if (att.type == FrameBufferAttachmentType::texture) {
			glGenTextures(1, &att.textureID);
			glBindTexture(GL_TEXTURE_2D, att.textureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, att.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, att.magFilter);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, att.textureWrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, att.textureWrap);

			glTexStorage2D(GL_TEXTURE_2D, 1, att.internalFormat, spec.width, spec.height);

			glFramebufferTexture2D(GL_FRAMEBUFFER, att.attachment, GL_TEXTURE_2D, att.textureID, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else if (att.type == FrameBufferAttachmentType::rbo) {
			glGenRenderbuffers(1, &att.RBO);
			glBindRenderbuffer(GL_RENDERBUFFER, att.RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, att.internalFormat, spec.width, spec.height);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, att.attachment, GL_RENDERBUFFER, att.RBO);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		else if (att.type == FrameBufferAttachmentType::depth_texture) {
			glGenTextures(1, &att.textureID);
			glBindTexture(GL_TEXTURE_2D, att.textureID);

			glTexStorage2D(GL_TEXTURE_2D, 1, att.internalFormat, spec.width, spec.height);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, att.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, att.magFilter);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, att.textureWrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, att.textureWrap);

			glm::vec4 borderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));

			glBindTexture(GL_TEXTURE_2D, 0);

			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, att.attachment, GL_TEXTURE_2D, att.textureID, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
		}
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		KI_ERROR_SB("FRAMEBUFFER:: Framebuffer is not complete!");
		KI_BREAK();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void FrameBuffer::bindTexture(const RenderContext& ctx, int attachmentIndex, int unitIndex)
{
	glBindTextures(unitIndex, 1, &spec.attachments[attachmentIndex].textureID);
}
