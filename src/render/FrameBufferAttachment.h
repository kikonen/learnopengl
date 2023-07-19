#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

enum class FrameBufferAttachmentType {
    shared,
    texture,
    depth_texture,
    rbo,
    shadow,
};

enum class ClearType {
    FLOAT,
    INT,
    UNSIGNED_INT,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL,
};

struct FrameBufferAttachment {
    FrameBufferAttachmentType type = FrameBufferAttachmentType::texture;

    GLenum attachment = GL_COLOR_ATTACHMENT0;
    bool useDrawBuffer = false;

    int internalFormat = GL_RGBA8;
    //int format = GL_RGBA;

    GLint minFilter = GL_NEAREST;
    GLint magFilter = GL_NEAREST;

    int textureWrapS = GL_CLAMP_TO_EDGE;
    int textureWrapT = GL_CLAMP_TO_EDGE;

    glm::vec4 borderColor{ 0.f, 0.f, 0.f, 0.f };
    glm::vec4 clearColor{ 0.f, 0.f, 0.f, 0.f };
    ClearType clearType{ ClearType::FLOAT };

    bool useMibMap = false;

    int drawBufferIndex = -1;
    unsigned int textureID = 0;
    unsigned int rbo = 0;

    FrameBufferAttachment* shared{ nullptr };

    void clearBuffer(int fbo) const;

    static FrameBufferAttachment getShared(FrameBufferAttachment* shared);

    static FrameBufferAttachment getTextureRGBA(GLenum attachment = GL_COLOR_ATTACHMENT0);

    static FrameBufferAttachment getTextureRGB(GLenum attachment = GL_COLOR_ATTACHMENT0);

    static FrameBufferAttachment getObjectId();

    // G buffer: diffuse
    static FrameBufferAttachment getGBufferAlbedo(GLenum attachment);

    // G buffer: specular
    static FrameBufferAttachment getGBufferSpecular(GLenum attachment);

    // G buffer: emission
    static FrameBufferAttachment getGBufferEmission(GLenum attachment);

    // G buffer: position
    static FrameBufferAttachment getGBufferPosition(GLenum attachment);

    // G buffer: normal
    static FrameBufferAttachment getGBufferNormal(GLenum attachment);

    static FrameBufferAttachment getRBODepthStencil();
    static FrameBufferAttachment getRBODepth();

    static FrameBufferAttachment getDepthTexture();
    static FrameBufferAttachment getShadow();

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
