#pragma once

#include <vector>
#include <initializer_list>

#include "ki/GL.h"

#include "scene/RenderContext.h"

enum class FrameBufferAttachmentType {
	texture,
	depth_texture,
	rbo,
};

struct FrameBufferAttachment {
	FrameBufferAttachmentType type = FrameBufferAttachmentType::texture;

	GLenum attachment = GL_COLOR_ATTACHMENT0;

	int internalFormat = GL_RGBA8;
	int format = GL_RGBA;

	GLint minFilter = GL_LINEAR;
	GLint magFilter = GL_LINEAR;

	int textureWrap = GL_CLAMP_TO_EDGE;

	bool useMibMap = false;

	unsigned int textureID = -1;
	unsigned int RBO = -1;

	static FrameBufferAttachment getTexture() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::texture;
		spec.internalFormat = GL_RGBA8;
		spec.format = GL_RGBA;
		spec.attachment = GL_COLOR_ATTACHMENT0;

		return spec;
	}

	static FrameBufferAttachment getMousePick() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::texture;
		spec.internalFormat = GL_RGBA8;
		spec.format = GL_RGBA;
		spec.attachment = GL_COLOR_ATTACHMENT1;

		return spec;
	}

	// G buffer: position
	static FrameBufferAttachment getGBufferPosition() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::texture;
		spec.internalFormat = GL_RGB16F;
		spec.format = GL_RGB16F;
		spec.attachment = GL_COLOR_ATTACHMENT1;

		return spec;
	}

	// G buffer: normal
	static FrameBufferAttachment getGBufferNormal() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::texture;
		spec.internalFormat = GL_RGB16F;
		spec.format = GL_RGB16F;
		spec.attachment = GL_COLOR_ATTACHMENT2;

		return spec;
	}

	// G buffer: diffuse + specular
	static FrameBufferAttachment getGBufferAlbedo() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::texture;
		spec.internalFormat = GL_RGBA16F;
		spec.format = GL_RGBA16F;
		spec.attachment = GL_COLOR_ATTACHMENT3;

		return spec;
	}

	// G buffer: emission
	static FrameBufferAttachment getGBufferEmission() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::texture;
		spec.internalFormat = GL_RGB16F;
		spec.format = GL_RGB16F;
		spec.attachment = GL_COLOR_ATTACHMENT3;

		return spec;
	}

	static FrameBufferAttachment getDepthTexture() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::depth_texture;
		spec.internalFormat = GL_DEPTH_COMPONENT24;
		spec.format = GL_RGBA;
		spec.attachment = GL_DEPTH_ATTACHMENT;
		spec.minFilter = GL_LINEAR;
		spec.magFilter = GL_LINEAR;
		spec.textureWrap = GL_CLAMP_TO_EDGE;

		return spec;
	}

	static FrameBufferAttachment getRBODepthStencil() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::rbo;
		spec.internalFormat = GL_DEPTH24_STENCIL8;
		spec.attachment = GL_DEPTH_STENCIL_ATTACHMENT;

		return spec;
	}

	static FrameBufferAttachment getRBODepth() {
		FrameBufferAttachment spec;
		spec.type = FrameBufferAttachmentType::rbo;
		spec.internalFormat = GL_DEPTH_COMPONENT24;
		spec.attachment = GL_DEPTH_ATTACHMENT;

		return spec;
	}
};

struct FrameBufferSpecification {
	FrameBufferSpecification(int width, int height, std::initializer_list<FrameBufferAttachment>&& attachments) 
		: width(width), 
		height(height), 
		attachments(attachments) {
	}

	int width;
	int height;

	std::vector<FrameBufferAttachment> attachments;
};

class FrameBuffer
{
public:
	FrameBuffer(const FrameBufferSpecification& spec);
	virtual ~FrameBuffer();

	virtual void prepare();
	void bind(const RenderContext& ctx);
	void unbind(const RenderContext& ctx);

	void bindTexture(const RenderContext& ctx, int attachmentIndex, int unitIndex);

public:
	FrameBufferSpecification spec;

	unsigned int FBO = -1;

protected:
	bool prepared = false;
};

