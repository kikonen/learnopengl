#include "FrameBufferAttachment.h"

#include "glm/ext.hpp"

#include "kigl/GLState.h"

#include "render/RenderContext.h"

namespace {
}

namespace render {
    FrameBufferAttachment::FrameBufferAttachment()
    {
    }

    FrameBufferAttachment::~FrameBufferAttachment()
    {
        // NOTE KI don't touch shared buffer
        if (shared) return;
        if (externalDelete) return;

        if (createdTexture) {
            glDeleteTextures(1, &textureID);
        }
        if (createdRbo) {
            glDeleteRenderbuffers(1, &rbo);
        }
    }

    void FrameBufferAttachment::create(
        std::string_view name,
        int width,
        int height)
    {
        auto& att = *this;

        {
            std::string attName = fmt::format("{}_att_{}_{}", name, att.index, att.name);

            if (att.type == FrameBufferAttachmentType::shared) {
                // NOTE KI nothing
            }
            else if (att.type == FrameBufferAttachmentType::draw_buffer) {
                // NOTE KI nothing
            }
            else if (att.type == FrameBufferAttachmentType::texture) {
                glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
                kigl::setLabel(GL_TEXTURE, att.textureID, attName);
                createdTexture = true;

                KI_INFO(fmt::format("CREATE_TEX: name={}, TEX={}", attName, att.textureID));

                glTextureStorage2D(att.textureID, 1, att.internalFormat, width, height);

                glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
                glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

                glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));
            }
            else if (att.type == FrameBufferAttachmentType::rbo) {
                glCreateRenderbuffers(1, &att.rbo);
                kigl::setLabel(GL_RENDERBUFFER, att.rbo, attName);
                createdRbo = true;

                KI_INFO(fmt::format("CREATE_RBO: name={}, RBO={}", attName, att.rbo));

                glNamedRenderbufferStorage(att.rbo, att.internalFormat, width, height);
            }
            else if (att.type == FrameBufferAttachmentType::depth_texture) {
                glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
                kigl::setLabel(GL_TEXTURE, att.textureID, attName);
                createdTexture = true;

                KI_INFO(fmt::format("CREATE_DEPTH: name={}, DEPTH={}", attName, att.textureID));

                glTextureStorage2D(att.textureID, 1, att.internalFormat, width, height);

                glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
                glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

                glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));
            }
            else if (att.type == FrameBufferAttachmentType::depth_stencil_texture) {
                glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
                kigl::setLabel(GL_TEXTURE, att.textureID, attName);
                createdTexture = true;

                KI_INFO(fmt::format("CREATE_DEPTH_STENCIL: name={}, DEPTH={}", attName, att.textureID));

                glTextureStorage2D(att.textureID, 1, att.internalFormat, width, height);

                glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
                glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

                glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));
            }
            else if (att.type == FrameBufferAttachmentType::shadow) {
                glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
                kigl::setLabel(GL_TEXTURE, att.textureID, attName);
                createdTexture = true;

                KI_INFO(fmt::format("CREATE_SHADOW: name={}, DEPTH={}", attName, att.textureID));

                glTextureStorage2D(att.textureID, 1, att.internalFormat, width, height);

                glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
                glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
                glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

                // NOTE KI *IMPORTANT* for shadow map min/mag interpolation
                // => sampler2DShadow init
                // https://stackoverflow.com/questions/22419682/glsl-sampler2dshadow-and-shadow2d-clarification
                glTextureParameteri(att.textureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glTextureParameteri(att.textureID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

                glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));
            }
        }
    }

    // NOTE KI *bindless* clear
    // i.e. "glClear" *requires* bind
    // => less changes of breaking state
    void FrameBufferAttachment::clearBuffer(int fbo) const
    {
        if (shared) return;

        switch (clearType) {
        case ClearType::NONE:
            break;
        case ClearType::FLOAT:
            if (activeDrawBufferIndex >= 0) {
                glClearNamedFramebufferfv(fbo, GL_COLOR, activeDrawBufferIndex, glm::value_ptr(clearColor));
            }
            break;
        case ClearType::INT:
            if (activeDrawBufferIndex >= 0) {
                glClearNamedFramebufferiv(fbo, GL_COLOR, activeDrawBufferIndex, glm::value_ptr(glm::ivec4(clearColor)));
            }
            break;
        case ClearType::UNSIGNED_INT:
            if (activeDrawBufferIndex >= 0) {
                glClearNamedFramebufferuiv(fbo, GL_COLOR, activeDrawBufferIndex, glm::value_ptr(glm::uvec4(clearColor)));
            }
            break;
        case ClearType::DEPTH:
            glClearNamedFramebufferfv(fbo, GL_DEPTH, 0, &clearColor[0]);
            break;
        case ClearType::STENCIL:
            glClearNamedFramebufferiv(fbo, GL_STENCIL, 0, &stencilClear);
            break;
        case ClearType::DEPTH_STENCIL:
            glClearNamedFramebufferfi(fbo, GL_DEPTH_STENCIL, 0, clearColor[0], stencilClear);
            break;
        }
    }

    void FrameBufferAttachment::invalidate(
        int fbo) const
    {
        //https://www.khronos.org/opengl/wiki/Framebuffer
        // https://community.arm.com/arm-community-blogs/b/graphics-gaming-and-vr-blog/posts/mali-performance-2-how-to-correctly-handle-framebuffers

        if (shared) return;

        GLenum attachments[] = { attachment };

        // TODO KI based into NSight this consumes some time; perhaps irrelevant to do
        glInvalidateNamedFramebufferData(fbo, 1, attachments);
    }

    void FrameBufferAttachment::clearWithMask(int fbo, GLbitfield mask) const
    {
        if (clearMask & mask) {
            clearBuffer(fbo);
        }
    }

    void FrameBufferAttachment::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, textureID, false);
    }

    void FrameBufferAttachment::unbindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.unbindTexture(unitIndex, false);
    }

    FrameBufferAttachment FrameBufferAttachment::getShared(
        FrameBufferAttachment* shared,
        GLenum attachment)
    {
        FrameBufferAttachment spec{ *shared };
        spec.type = FrameBufferAttachmentType::shared;
        spec.shared = shared;
        spec.name = fmt::format("{}_shared", spec.name);
        // NOTE KI no clearMask on shared; cannot clear as part as shared

        if (attachment) {
            spec.attachment = attachment;
        }

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getDrawBuffer(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::draw_buffer;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearType = ClearType::NONE;
        spec.name = "draw";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getTextureRGBA(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "texture_RGBA8";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getTextureRGBAHdr(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI float, not normalized, to go over 1.0
        spec.internalFormat = GL_RGBA16F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "texture_RGBA16F";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getTextureRGB(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "texture_RGB8";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getTextureRGBHdr(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI float, not normalized, to go over 1.0
        spec.internalFormat = GL_R11F_G11F_B10F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "texture_GL_R11F_G11F_B10F";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getObjectId()
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA8;
        spec.attachment = GL_COLOR_ATTACHMENT0;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "object_id";

        return spec;
    }

    // G buffer: diffuse.rgb
    FrameBufferAttachment FrameBufferAttachment::getGBufferAlbedo(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "albedo_RGB8";

        return spec;
    }

    // G buffer: diffuse.rgb
    FrameBufferAttachment FrameBufferAttachment::getGBufferAlbedoHdr(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI float, not normalized, to go over 1.0
        spec.internalFormat = GL_R11F_G11F_B10F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "albedo_R11F_G11F_B10F";

        return spec;
    }

    // G buffer: specular.rgb + shininess.a
    FrameBufferAttachment FrameBufferAttachment::getGBufferSpecular(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "specular_RGBA8";

        return spec;
    }

    // G buffer: specular.rgb + shininess.a
    FrameBufferAttachment FrameBufferAttachment::getGBufferSpecularHdr(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI float, not normalized, to go over 1.0
        spec.internalFormat = GL_R11F_G11F_B10F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "specular_GL_R11F_G11F_B10F";

        return spec;
    }

    // MRAO: [metalness, roughness, ambient-occlusion]
    FrameBufferAttachment FrameBufferAttachment::getGBufferMRA(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "mra_RGB8";

        return spec;
    }

    // MRAO: [metalness, roughness, ambient-occlusion]
    FrameBufferAttachment FrameBufferAttachment::getGBufferMRA16(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI metal values normalized to range [0, 1]
        spec.internalFormat = GL_RGB16;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "mra_RGB16";

        return spec;
    }

    // G buffer: emission
    FrameBufferAttachment FrameBufferAttachment::getGBufferEmission(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGB8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "emission_RGB8";

        return spec;
    }

    // G buffer: emission
    FrameBufferAttachment FrameBufferAttachment::getGBufferEmissionHdr(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI float, not normalized, to go over 1.0
        spec.internalFormat = GL_R11F_G11F_B10F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "emission_R11F_G11F_B10F";

        return spec;
    }

    // G buffer: position
    FrameBufferAttachment FrameBufferAttachment::getGBufferPosition(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI GL_RGB16F is *NOT* enough precision; causes very block light/shadows
        // - also vertical black stripes appering in mirror
        spec.internalFormat = GL_RGB32F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "position_RGB32F";

        return spec;
    }

    // G buffer: normal ([0, 1] scale, normalized)
    FrameBufferAttachment FrameBufferAttachment::getGBufferNormal(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI it seems GL_R11F_G11F_B10F is not enough precision for normal
        // => causes odd artifacts in light render
        // => *likely* reason is that GL_R11F_G11F_B10F does NOT have sign bit
        //    - https://www.khronos.org/opengl/wiki/Image_Format
        // => MUST scale [-1, 1] range to [0, 1] range in gbuffer (& remember to normalize)
        // NOTE KI GL_R11F_G11F_B10F does not seem to be enough resolution for curved surfaces
        // => cause "ring" arfifacts
        // => shadow artifacts on ball surface seem to be different issue
        //spec.internalFormat = GL_R11F_G11F_B10F;
        spec.internalFormat = GL_RGB16F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "normal_RGB16F";

        return spec;
    }

    // G buffer: viewZ
    FrameBufferAttachment FrameBufferAttachment::getGBufferViewZ(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_R16F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "view_z_R16F";

        return spec;
    }

    // Effect buffer: diffuse.rgba
    FrameBufferAttachment FrameBufferAttachment::getEffectTexture(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_RGBA8;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "ettect_RGBA8";

        return spec;
    }

    // Effect buffer: diffuse.rgba
    FrameBufferAttachment FrameBufferAttachment::getEffectTextureHdr(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI float, not normalized, to go over 1.0
        spec.internalFormat = GL_RGBA16F;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;
        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "ettect_RGBA16F";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::FrameBufferAttachment::getDepthTexture()
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::depth_texture;
        // NOTE KI need to have 24bit, 16bit is FAR TOO SMALL
        spec.internalFormat = GL_DEPTH_COMPONENT24;
        spec.attachment = GL_DEPTH_ATTACHMENT;
        spec.minFilter = GL_NEAREST;
        spec.magFilter = GL_NEAREST;
        //spec.minFilter = GL_LINEAR;
        //spec.magFilter = GL_LINEAR;
        spec.textureWrapS = GL_CLAMP_TO_BORDER;
        spec.textureWrapT = GL_CLAMP_TO_BORDER;

        spec.borderColor = { 1.f, 1.f, 1.f, 1.f };
        spec.clearColor = { 1.f, 0.f, 0.f, 0.f };
        spec.clearType = ClearType::DEPTH;

        spec.clearMask = GL_DEPTH_BUFFER_BIT;
        spec.name = "depth_24";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::FrameBufferAttachment::getDepthStencilTexture()
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::depth_stencil_texture;
        spec.internalFormat = GL_DEPTH24_STENCIL8;
        spec.attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        spec.clearType = ClearType::DEPTH_STENCIL;

        spec.minFilter = GL_NEAREST;
        spec.magFilter = GL_NEAREST;

        spec.textureWrapS = GL_CLAMP_TO_BORDER;
        spec.textureWrapT = GL_CLAMP_TO_BORDER;

        spec.borderColor = { 1.f, 1.f, 1.f, 1.f };

        // NOTE KI (depth, clear, _, _)
        spec.clearColor = { 1.f, 0.f, 0.f, 0.f };
        spec.clearMask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        spec.name = "depth_stencil_24_8";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getShadow()
    {
        // https://stackoverflow.com/questions/22419682/glsl-sampler2dshadow-and-shadow2d-clarification
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::shadow;
        // NOTE KI need to have 24bit, 16bit is FAR TOO SMALL
        spec.internalFormat = GL_DEPTH_COMPONENT24;
        spec.attachment = GL_DEPTH_ATTACHMENT;
        // NOTE KI linear slower, but *BETTER* results
        // CHECK KI does it actually matter for shadowmap?!?
        // "LINEAR" is *supposed* get free 4 texel PCF in shadow mapping
        // - https://fabiensanglard.net/shadowmappingPCF/index.php
        // => based into experimentation that is true
        //spec.minFilter = GL_NEAREST;
        //spec.magFilter = GL_NEAREST;
        spec.minFilter = GL_LINEAR;
        spec.magFilter = GL_LINEAR;
        spec.textureWrapS = GL_CLAMP_TO_BORDER;
        spec.textureWrapT = GL_CLAMP_TO_BORDER;

        spec.borderColor = { 1.f, 1.f, 1.f, 1.f };
        spec.clearColor = { 1.f, 0.f, 0.f, 0.f };
        spec.clearType = ClearType::DEPTH;

        spec.clearMask = GL_DEPTH_BUFFER_BIT;
        spec.name = "shadow_24";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getDepthStencilRbo()
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::rbo;
        spec.internalFormat = GL_DEPTH24_STENCIL8;
        spec.attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        spec.clearType = ClearType::DEPTH_STENCIL;

        // NOTE KI (depth, clear, _, _)
        spec.clearColor = { 1.f, 0.f, 0.f, 0.f };

        spec.clearMask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        spec.name = "depth_stencil_24_8";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getDepthRbo()
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::rbo;
        spec.internalFormat = GL_DEPTH_COMPONENT24;
        spec.attachment = GL_DEPTH_ATTACHMENT;
        spec.clearType = ClearType::DEPTH;

        // NOTE KI (depth, clear, _, _)
        spec.clearColor = { 1.f, 0.f, 0.f, 0.f };

        spec.clearMask = GL_DEPTH_BUFFER_BIT;
        spec.name = "depth_rbo_24";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getOITAccumulatorTexture(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        // NOTE KI *can* accumulate over 1.0
        spec.internalFormat = GL_RGBA16F;
        spec.minFilter = GL_NEAREST;
        spec.magFilter = GL_NEAREST;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;

        spec.clearColor = { 0.f, 0.f, 0.f, 0.f };
        spec.borderColor = { 0.f, 0.f, 0.f, 0.f };

        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "oit_acc_RGBA16F";

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

        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "oit_reveal_R8";

        return spec;
    }

    FrameBufferAttachment FrameBufferAttachment::getSsaoTexture(GLenum attachment)
    {
        FrameBufferAttachment spec;
        spec.type = FrameBufferAttachmentType::texture;
        spec.internalFormat = GL_R16F;
        spec.minFilter = GL_NEAREST;
        spec.magFilter = GL_NEAREST;
        spec.attachment = attachment;
        spec.useDrawBuffer = true;

        spec.clearColor = { 1.f, 1.f, 1.f, 1.f };
        spec.borderColor = { 1.f, 1.f, 1.f, 1.f };

        spec.clearMask = GL_COLOR_BUFFER_BIT;
        spec.name = "ssao_R16F";

        return spec;
    }
}
