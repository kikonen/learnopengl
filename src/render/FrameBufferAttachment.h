#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

class RenderContext;

namespace render {
    enum class FrameBufferAttachmentType : std::underlying_type_t<std::byte> {
        shared,
        draw_buffer,
        texture,
        depth_texture,
        depth_stencil_texture,
        rbo,
        shadow,
    };

    enum class ClearType : std::underlying_type_t<std::byte> {
        NONE,
        FLOAT,
        INT,
        UNSIGNED_INT,
        DEPTH,
        STENCIL,
        DEPTH_STENCIL,
    };

    struct FrameBufferAttachment {
        FrameBufferAttachmentType type = FrameBufferAttachmentType::texture;

        int index = -1;

        GLenum attachment = GL_COLOR_ATTACHMENT0;
        bool useDrawBuffer = false;

        int internalFormat = GL_RGBA8;
        //int format = GL_RGBA;

        GLint minFilter = GL_LINEAR_MIPMAP_NEAREST;
        GLint magFilter = GL_LINEAR;

        int textureWrapS = GL_CLAMP_TO_EDGE;
        int textureWrapT = GL_CLAMP_TO_EDGE;
        int textureWrapR = GL_CLAMP_TO_EDGE;

        glm::vec4 borderColor{ 0.f, 0.f, 0.f, 0.f };
        glm::vec4 clearColor{ 0.f, 0.f, 0.f, 0.f };
        GLint stencilClear{ 0 };
        ClearType clearType{ ClearType::FLOAT };

        bool useMibMap = false;

        int drawBufferIndex = -1;
        unsigned int textureID = 0;
        unsigned int rbo = 0;

        GLbitfield clearMask = 0;

        bool externalDelete{ false };
        FrameBufferAttachment* shared{ nullptr };

        FrameBufferAttachment();
        ~FrameBufferAttachment();

        void create(
            std::string_view name,
            int width,
            int height);

        void clearBuffer(int fbo) const;
        void clearWithMask(int fbo, GLbitfield mask) const;

        void invalidate(
            int fbo) const;

        void bindTexture(
            const RenderContext& ctx,
            int unitIndex);

        void unbindTexture(
            const RenderContext& ctx,
            int unitIndex);

        bool valid() { return textureID > 0 || rbo > 0; }
        operator int() const { return textureID || rbo; }

        static FrameBufferAttachment getShared(FrameBufferAttachment* shared);
        static FrameBufferAttachment getDrawBuffer(GLenum attachment = GL_COLOR_ATTACHMENT0);

        static FrameBufferAttachment getTextureRGBA(GLenum attachment = GL_COLOR_ATTACHMENT0);
        static FrameBufferAttachment getTextureRGBAHdr(GLenum attachment = GL_COLOR_ATTACHMENT0);

        static FrameBufferAttachment getTextureRGB(GLenum attachment = GL_COLOR_ATTACHMENT0);
        static FrameBufferAttachment getTextureRGBHdr(GLenum attachment = GL_COLOR_ATTACHMENT0);

        static FrameBufferAttachment getObjectId();

        // G buffer: diffuse
        static FrameBufferAttachment getGBufferAlbedo(GLenum attachment);
        static FrameBufferAttachment getGBufferAlbedoHdr(GLenum attachment);

        // G buffer: specular
        static FrameBufferAttachment getGBufferSpecular(GLenum attachment);
        static FrameBufferAttachment getGBufferSpecularHdr(GLenum attachment);

        // G buffer: metal
        static FrameBufferAttachment getGBufferMetal(GLenum attachment);
        static FrameBufferAttachment getGBufferMetal16(GLenum attachment);

        // G buffer: emission
        static FrameBufferAttachment getGBufferEmission(GLenum attachment);
        static FrameBufferAttachment getGBufferEmissionHdr(GLenum attachment);

        // G buffer: position
        static FrameBufferAttachment getGBufferPosition(GLenum attachment);

        // G buffer: normal
        static FrameBufferAttachment getGBufferNormal(GLenum attachment);

        // Effect buffer: diffuse
        static FrameBufferAttachment getEffectTexture(GLenum attachment);
        static FrameBufferAttachment getEffectTextureHdr(GLenum attachment);

        static FrameBufferAttachment getDepthStencilRbo();
        static FrameBufferAttachment getDepthRbo();

        static FrameBufferAttachment getDepthTexture();
        static FrameBufferAttachment getDepthStencilTexture();
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
}
