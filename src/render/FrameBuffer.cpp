#include "FrameBuffer.h"

#include <iostream>

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "render/RenderContext.h"
#include "render/Batch.h"

FrameBuffer::FrameBuffer(
    const std::string& name,
    const FrameBufferSpecification& spec)
    : m_name(name),
    m_spec(spec)
{
}

FrameBuffer::~FrameBuffer()
{
    if (!m_prepared) return;

    glDeleteFramebuffers(1, &m_fbo);

    for (auto& att : m_spec.attachments) {
        if (att.textureID) {
            glDeleteTextures(1, &att.textureID);
        }
        if (att.rbo) {
            glDeleteRenderbuffers(1, &att.rbo);
        }
    }

}

const std::string FrameBuffer::str() const noexcept
{
    return fmt::format(
        "<FBO: name={}, fbo={}, w={}, h={}, attachments={}>",
        m_name, m_fbo,
        m_spec.width, m_spec.height,
        m_spec.attachments.size());
}

void FrameBuffer::prepare(
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    {
        glCreateFramebuffers(1, &m_fbo);
        KI_INFO(fmt::format("CREATE: FBO={}", str()));
    }

    int clearMask = 0;

    for (auto& att : m_spec.attachments) {
        if (att.type == FrameBufferAttachmentType::texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
            KI_INFO(fmt::format("CREATE_TEX: FBO={}, TEX={}", str(), att.textureID));

            glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
            glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

            glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));

            glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

            if (att.useDrawBuffer) {
                m_drawBuffers.push_back(att.attachment);
            }

            clearMask |= GL_COLOR_BUFFER_BIT;
        }
        else if (att.type == FrameBufferAttachmentType::rbo) {
            glCreateRenderbuffers(1, &att.rbo);
            KI_INFO(fmt::format("CREATE_RBO: FBO={}, RBO={}", str(), att.rbo));

            glNamedRenderbufferStorage(att.rbo, att.internalFormat, m_spec.width, m_spec.height);
            glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, att.rbo);

            clearMask |= GL_COLOR_BUFFER_BIT;
        }
        else if (att.type == FrameBufferAttachmentType::depth_texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
            KI_INFO(fmt::format("CREATE_DEPTH: FBO={}, DEPTH={}", str(), att.textureID));

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

            glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
            glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

            // NOTE KI *IMPORTANT* for shadow map min/mag interpolation
            glTextureParameteri(att.textureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(att.textureID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

            glm::vec4 borderColor{ 1.0f, 1.0f, 1.0f, 1.0f };
            glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));

            {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
                glNamedFramebufferDrawBuffer(m_fbo, GL_NONE);
                glNamedFramebufferReadBuffer(m_fbo, GL_NONE);
            }

            clearMask |= GL_DEPTH_BUFFER_BIT;
        }
    }

    if (m_drawBuffers.size() > 0) {
        glNamedFramebufferDrawBuffers(m_fbo, m_drawBuffers.size(), m_drawBuffers.data());
    }

    GLenum status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::string msg = fmt::format(
            "FRAMEBUFFER:: Framebuffer is not complete! status=0x{:x} ({})",
            status, status);
        KI_ERROR(msg);
        throw std::runtime_error{ msg };
    }

    // NOTE KI clear buffer to avoid showing garbage
    if (clear) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(clearMask);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void FrameBuffer::bind(const RenderContext& ctx)
{
    // NOTE KI MUST flush before changing render target
    ctx.m_batch->flush(ctx);

    ctx.m_state.bindFrameBuffer(m_fbo, m_forceBind);
    glViewport(0, 0, m_spec.width, m_spec.height);
}

void FrameBuffer::unbind(const RenderContext& ctx)
{
    // NOTE KI WindowBuffer.bind() is responsible for this now!

    //const auto& res = ctx.m_resolution;

    //// NOTE KI 0 stands for "default"
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0, 0, res.x, res.y);
}

void FrameBuffer::bindTexture(const RenderContext& ctx, int attachmentIndex, int unitIndex)
{
    ctx.m_state.bindTexture(unitIndex, m_spec.attachments[attachmentIndex].textureID, false);
}

void FrameBuffer::unbindTexture(const RenderContext& ctx, int unitIndex)
{
    ctx.m_state.bindTexture(unitIndex, 0, true);
}

// mask = GL_COLOR_BUFFER_BIT,
void FrameBuffer::blit(
    FrameBuffer* target,
    GLbitfield mask,
    const glm::vec2& pos,
    const glm::vec2& size)
{
    const float srcW = m_spec.width;
    const float srcH = m_spec.height;

    const float dstW = target->m_spec.width;
    const float dstH = target->m_spec.height;

    const glm::vec2 s0{ 0 };
    const glm::vec2 s1{ srcW, srcH };

    const float mx = dstW * 0.5f;
    const float my = dstH * 0.5f;

    const float dx = mx - mx * -pos.x;
    const float dy = my - my * pos.y;

    const float sx = mx * size.x;
    const float sy = my * size.y;

    const glm::vec2 d0{ dx, dy };
    const glm::vec2 d1{ dx + sx, dy + sy };

    glBlitNamedFramebuffer(
        m_fbo,
        target->m_fbo,
        s0.x,
        s0.y,
        s1.x,
        s1.y,
        d0.x,
        d0.y,
        d1.x,
        d1.y,
        mask,
        GL_NEAREST);
}

void FrameBuffer::clear(
    const RenderContext& ctx,
    GLbitfield clearMask,
    const glm::vec4& clearColor)
{
    // NOTE KI clear for current draw buffer buffer (main/mirror/etc.)
    if (ctx.m_assets.clearColor) {
        if (ctx.m_assets.debugClearColor) {
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        }
        clearMask |= GL_COLOR_BUFFER_BIT;
    }
    if (clearMask != 0) {
        glClear(clearMask);
    }
}
