#include "FrameBuffer.h"

#include <iostream>

#include "glm/glm.hpp"

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
        if (att.RBO) {
            glDeleteRenderbuffers(1, &att.RBO);
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
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    }

    int clearMask = 0;

    for (auto& att : m_spec.attachments) {
        if (att.type == FrameBufferAttachmentType::texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);

            glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
            glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrap);
            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrap);

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

            glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));

            glFramebufferTexture2D(GL_FRAMEBUFFER, att.attachment, GL_TEXTURE_2D, att.textureID, 0);

            clearMask |= GL_COLOR_BUFFER_BIT;
        }
        else if (att.type == FrameBufferAttachmentType::rbo) {
            glGenRenderbuffers(1, &att.RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, att.RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, att.internalFormat, m_spec.width, m_spec.height);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, att.attachment, GL_RENDERBUFFER, att.RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            clearMask |= GL_COLOR_BUFFER_BIT;
        }
        else if (att.type == FrameBufferAttachmentType::depth_texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

            glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
            glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

            glTextureParameteri(att.textureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(att.textureID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrap);
            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrap);

            glm::vec4 borderColor{ 1.0f, 1.0f, 1.0f, 1.0f };
            glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));

            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, att.attachment, GL_TEXTURE_2D, att.textureID, 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
            }

            clearMask |= GL_DEPTH_BUFFER_BIT;
        }
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        KI_ERROR_SB("FRAMEBUFFER:: Framebuffer is not complete!");
        KI_BREAK();
    }

    // NOTE KI clear buffer to avoid showing garbage
    if (clear) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(clearMask);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::bind(const RenderContext& ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_spec.width, m_spec.height);
}

void FrameBuffer::unbind(const RenderContext& ctx)
{
    const auto& res = ctx.resolution;

    // NOTE KI 0 stands for "default"
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, res.x, res.y);
}

void FrameBuffer::bindTexture(const RenderContext& ctx, int attachmentIndex, int unitIndex)
{
    ctx.state.bindTexture(unitIndex, m_spec.attachments[attachmentIndex].textureID);
}
