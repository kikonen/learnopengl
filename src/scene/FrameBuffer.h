#pragma once

#include <vector>
#include <initializer_list>

#include <glm/glm.hpp>

#include "ki/GL.h"

enum class FrameBufferAttachmentType {
    texture,
    depth_texture,
    rbo,
};

struct FrameBufferAttachment {
    FrameBufferAttachmentType type = FrameBufferAttachmentType::texture;

    GLenum attachment = GL_COLOR_ATTACHMENT0;
    GLenum useDrawBuffer = false;

    int internalFormat = GL_RGBA8;
    int format = GL_RGBA;

    GLint minFilter = GL_NEAREST;
    GLint magFilter = GL_NEAREST;

    int textureWrapS = GL_CLAMP_TO_EDGE;
    int textureWrapT = GL_CLAMP_TO_EDGE;

    glm::vec4 borderColor = { 0.0, 0.0, 0.0, 0.0 };

    bool useMibMap = false;

    unsigned int textureID = 0;
    unsigned int rbo = 0;

    static FrameBufferAttachment getTextureRGBA(GLenum attachment = GL_COLOR_ATTACHMENT0) {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA8;
        spec.format = GL_RGBA;
        spec.attachment = attachment;

        return spec;
    }

    static FrameBufferAttachment getTextureRGB(GLenum attachment = GL_COLOR_ATTACHMENT0) {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB8;
        spec.format = GL_RGB;
        spec.attachment = attachment;

        return spec;
    }

    static FrameBufferAttachment getObjectId() {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA8;
        spec.format = GL_RGBA;
        spec.attachment = GL_COLOR_ATTACHMENT0;

        return spec;
    }

    // G buffer: position - ATTACHMENT0
    static FrameBufferAttachment getGBufferPosition() {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB16F;
        spec.format = GL_RGB16F;
        spec.attachment = GL_COLOR_ATTACHMENT0;
        spec.useDrawBuffer = true;

        return spec;
    }

    // G buffer: normal - ATTACHMENT1
    static FrameBufferAttachment getGBufferNormal() {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB16F;
        spec.format = GL_RGB16F;
        spec.attachment = GL_COLOR_ATTACHMENT1;
        spec.useDrawBuffer = true;

        return spec;
    }

    // G buffer: diffuse + specular - ATTACHMENT2
    static FrameBufferAttachment getGBufferAlbedo() {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA16F;
        spec.format = GL_RGBA16F;
        spec.attachment = GL_COLOR_ATTACHMENT2;
        spec.useDrawBuffer = true;

        return spec;
    }

    // G buffer: emission - ATTACHMENT3
    static FrameBufferAttachment getGBufferEmission() {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB16F;
        spec.format = GL_RGB16F;
        spec.attachment = GL_COLOR_ATTACHMENT3;
        spec.useDrawBuffer = true;

        return spec;
    }

    static FrameBufferAttachment getDepthTexture() {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::depth_texture;
        // NOTE KI need to have 24bit, 16bit is FAR TOO SMALL
        spec.internalFormat = GL_DEPTH_COMPONENT24;
        spec.format = GL_RGBA;
        spec.attachment = GL_DEPTH_ATTACHMENT;
        // NOTE KI linear slower, but *BETTER* results
        spec.minFilter = GL_LINEAR;
        spec.magFilter = GL_LINEAR;
        spec.textureWrapS = GL_CLAMP_TO_BORDER;
        spec.textureWrapT = GL_CLAMP_TO_BORDER;

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

class RenderContext;

class FrameBuffer
{
public:
    FrameBuffer(const FrameBufferSpecification& spec);
    virtual ~FrameBuffer();

    virtual void prepare(
        const bool clear,
        const glm::vec4& clearColor);

    virtual void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx, int attachmentIndex, int unitIndex);

public:
    FrameBufferSpecification m_spec;

    GLuint m_fbo = 0;

protected:
    bool m_prepared = false;
    std::vector<GLenum> m_drawBuffers;
};

