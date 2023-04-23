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

// G buffer: ambient
FrameBufferAttachment FrameBufferAttachment::getGBufferAmbient(GLenum attachment)
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
    // https://stackoverflow.com/questions/22419682/glsl-sampler2dshadow-and-shadow2d-clarification
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::depth_texture;
    // NOTE KI need to have 24bit, 16bit is FAR TOO SMALL
    spec.internalFormat = GL_DEPTH_COMPONENT32F;
    spec.format = GL_FLOAT;
    spec.attachment = GL_DEPTH_ATTACHMENT;
    // NOTE KI linear slower, but *BETTER* results
    // CHECK KI does it actually matter for shadowmap?!?
    spec.minFilter = GL_NEAREST;
    spec.magFilter = GL_NEAREST;
    //spec.minFilter = GL_LINEAR;
    //spec.magFilter = GL_LINEAR;
    spec.textureWrapS = GL_CLAMP_TO_BORDER;
    spec.textureWrapT = GL_CLAMP_TO_BORDER;

    spec.borderColor = { 1.f, 1.f, 1.f, 1.f };

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
