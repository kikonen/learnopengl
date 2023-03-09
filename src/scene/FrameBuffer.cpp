#include "FrameBuffer.h"

#include <iostream>

#include "glm/glm.hpp"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

FrameBuffer::FrameBuffer(const FrameBufferSpecification& spec)
    : m_spec(spec)
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

void FrameBuffer::prepare(
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    {
        glCreateFramebuffers(1, &m_fbo);
    }

    int clearMask = 0;

    for (auto& att : m_spec.attachments) {
        if (att.type == FrameBufferAttachmentType::texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);

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
            glNamedRenderbufferStorage(att.rbo, att.internalFormat, m_spec.width, m_spec.height);
            glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, att.rbo);

            clearMask |= GL_COLOR_BUFFER_BIT;
        }
        else if (att.type == FrameBufferAttachmentType::depth_texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);

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

    if (glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        KI_ERROR("FRAMEBUFFER:: Framebuffer is not complete!");
        throw std::runtime_error{ "FRAMEBUFFER:: Framebuffer is not complete!" };
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

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
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
    ctx.state.bindTexture(unitIndex, m_spec.attachments[attachmentIndex].textureID, false);
}
