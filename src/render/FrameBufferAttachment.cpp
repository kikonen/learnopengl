#include "FrameBufferAttachment.h"

#include "glm/ext.hpp"

namespace {
}

void FrameBufferAttachment::clearBuffer(int fbo) const
{
    switch (clearType) {
    case ClearType::FLOAT:
        if (drawBufferIndex >= 0) {
            glClearNamedFramebufferfv(fbo, GL_COLOR, drawBufferIndex, glm::value_ptr(clearColor));
        }
        break;
    case ClearType::INT:
        if (drawBufferIndex >= 0) {
            glClearNamedFramebufferiv(fbo, GL_COLOR, drawBufferIndex, glm::value_ptr(glm::ivec4(clearColor)));
        }
        break;
    case ClearType::UNSIGNED_INT:
        if (drawBufferIndex >= 0) {
            glClearNamedFramebufferuiv(fbo, GL_COLOR, drawBufferIndex, glm::value_ptr(glm::uvec4(clearColor)));
        }
        break;
    case ClearType::DEPTH_STENCIL:
        glClearNamedFramebufferfi(fbo, GL_DEPTH_STENCIL, 0, clearColor[0], clearColor[1]);
        break;
    }
}

FrameBufferAttachment FrameBufferAttachment::getShared(FrameBufferAttachment* shared)
{
    FrameBufferAttachment spec{ *shared };
    spec.type = FrameBufferAttachmentType::shared;
    spec.shared = shared;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getTextureRGBA(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getTextureRGB(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGB8;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getObjectId()
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
    spec.attachment = GL_COLOR_ATTACHMENT0;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: diffuse + IGNORE light flag
FrameBufferAttachment FrameBufferAttachment::getGBufferAlbedo(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA8;
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
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: position
FrameBufferAttachment FrameBufferAttachment::getGBufferPosition(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    // NOTE KI GL_RGB16F is *NOT* enough precision; causes very block light/shadows
    spec.internalFormat = GL_RGB32F;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    return spec;
}

// G buffer: normal
FrameBufferAttachment FrameBufferAttachment::getGBufferNormal(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_R11F_G11F_B10F;
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
    spec.internalFormat = GL_DEPTH_COMPONENT24;
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
    spec.clearColor = { 1.f, 0.f, 0.f, 0.f };
    spec.clearType = ClearType::DEPTH_STENCIL;

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getRBODepthStencil()
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::rbo;
    spec.internalFormat = GL_DEPTH24_STENCIL8;
    spec.attachment = GL_DEPTH_STENCIL_ATTACHMENT;
    spec.clearType = ClearType::DEPTH_STENCIL;

    // NOTE KI (depth, clear, _, _)
    spec.clearColor = { 1.f, 0.f, 0.f, 0.f };

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getRBODepth()
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::rbo;
    spec.internalFormat = GL_DEPTH_COMPONENT24;
    spec.attachment = GL_DEPTH_ATTACHMENT;
    spec.clearType = ClearType::DEPTH_STENCIL;

    // NOTE KI (depth, clear, _, _)
    spec.clearColor = { 1.f, 0.f, 0.f, 0.f };

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getOITAccumulatorTexture(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_RGBA16F;
    spec.minFilter = GL_NEAREST;
    spec.magFilter = GL_NEAREST;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    spec.clearColor = { 0.f, 0.f, 0.f, 0.f };
    spec.borderColor = { 0.f, 0.f, 0.f, 0.f };

    return spec;
}

FrameBufferAttachment FrameBufferAttachment::getOITRevealTexture(GLenum attachment)
{
    FrameBufferAttachment spec;
    spec.type = FrameBufferAttachmentType::texture;
    spec.internalFormat = GL_R8;
    spec.minFilter = GL_NEAREST;
    spec.magFilter = GL_NEAREST;
    spec.attachment = attachment;
    spec.useDrawBuffer = true;

    spec.clearColor = { 1.f, 1.f, 1.f, 1.f };
    spec.borderColor = { 1.f, 1.f, 1.f, 1.f };

    return spec;
}
