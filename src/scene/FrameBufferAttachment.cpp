#include "FrameBufferAttachment.h"

FrameBufferAttachment FrameBufferAttachment::getTextureRGBA(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.format = GL_RGBA;
    spec.attachment = attachment;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getTextureRGB(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGB8;
    spec.format = GL_RGB;
    spec.attachment = attachment;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getObjectId()
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.format = GL_RGBA;
    spec.attachment = GL_COLOR_ATTACHMENT0;

    return spec;
}

// G buffer: diffuse
FrameBufferAttachment FrameBufferAttachment::getGBufferAlbedo(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.format = GL_RGBA;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: specular
FrameBufferAttachment FrameBufferAttachment::getGBufferSpecular(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.format = GL_RGBA;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: emission
FrameBufferAttachment FrameBufferAttachment::getGBufferEmission(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.format = GL_RGBA;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: position
FrameBufferAttachment FrameBufferAttachment::getGBufferPosition(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA16F;
    spec.format = GL_RGBA;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: normal
FrameBufferAttachment FrameBufferAttachment::getGBufferNormal(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA16F;
    spec.format = GL_RGBA;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::FrameBufferAttachment::getDepthTexture()
{
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

FrameBufferAttachment FrameBufferAttachment::getRBODepthStencil()
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::rbo;
    spec.internalFormat = GL_DEPTH24_STENCIL8;
    spec.attachment = GL_DEPTH_STENCIL_ATTACHMENT;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getRBODepth()
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::rbo;
    spec.internalFormat = GL_DEPTH_COMPONENT24;
    spec.attachment = GL_DEPTH_ATTACHMENT;

    return spec;
}
