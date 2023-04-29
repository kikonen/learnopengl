#pragma once

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
    //int format = GL_RGBA;

    GLint minFilter = GL_NEAREST;
    GLint magFilter = GL_NEAREST;

    int textureWrapS = GL_CLAMP_TO_EDGE;
    int textureWrapT = GL_CLAMP_TO_EDGE;

    glm::vec4 borderColor = { 0.0, 0.0, 0.0, 0.0 };

    bool useMibMap = false;

    unsigned int textureID = 0;
    unsigned int rbo = 0;

    static FrameBufferAttachment getTextureRGBA(GLenum attachment = GL_COLOR_ATTACHMENT0);

    static FrameBufferAttachment getTextureRGB(GLenum attachment = GL_COLOR_ATTACHMENT0);

    static FrameBufferAttachment getObjectId();

    // G buffer: diffuse
    static FrameBufferAttachment getGBufferAlbedo(GLenum attachment);

    // G buffer: specular
    static FrameBufferAttachment getGBufferSpecular(GLenum attachment);

    // G buffer: emission
    static FrameBufferAttachment getGBufferEmission(GLenum attachment);

    // G buffer: ambient
    static FrameBufferAttachment getGBufferAmbient(GLenum attachment);

    // G buffer: position
    static FrameBufferAttachment getGBufferPosition(GLenum attachment);

    // G buffer: normal
    static FrameBufferAttachment getGBufferNormal(GLenum attachment);

    static FrameBufferAttachment getDepthTexture();

    static FrameBufferAttachment getRBODepthStencil();

    static FrameBufferAttachment getRBODepth();

    static FrameBufferAttachment getOITAccumulatorTexture(GLenum attachment);
    static FrameBufferAttachment getOITRevealTexture(GLenum attachment);
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
