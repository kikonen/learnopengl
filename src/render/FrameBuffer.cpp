#include "FrameBuffer.h"

#include <iostream>

#include <fmt/format.h>

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/RenderData.h"


namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };
}


FrameBuffer::FrameBuffer(
    const std::string& name,
    const FrameBufferSpecification& spec)
    : m_name(name),
    m_spec(spec)
{
    m_bufferInfo.u_bufferResolution = { m_spec.width, m_spec.height };
}

FrameBuffer::~FrameBuffer()
{
    if (!m_prepared) return;

    glDeleteFramebuffers(1, &m_fbo);
    m_spec.attachments.clear();
}

const std::string FrameBuffer::str() const noexcept
{
    return fmt::format(
        "<FBO: name={}, fbo={}, w={}, h={}, attachments={}>",
        m_name, m_fbo,
        m_spec.width, m_spec.height,
        m_spec.attachments.size());
}

void FrameBuffer::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    m_bufferInfo.u_bufferResolution = { m_spec.width, m_spec.height };

    {
        glCreateFramebuffers(1, &m_fbo);
        glObjectLabel(GL_FRAMEBUFFER, m_fbo, m_name.length(), m_name.c_str());

        KI_INFO(fmt::format("CREATE: FBO={}", str()));
    }

    {
        int idx = 0;
        for (auto& att : m_spec.attachments) {
            att.index = idx++;
        }
    }

    for (auto& att : m_spec.attachments) {
        std::string attName = fmt::format("{}-att-{}", m_name, att.index);

        if (att.type == FrameBufferAttachmentType::shared) {
            // NOTE KI drawBuffer index *can* be different between fbos
            if (att.useDrawBuffer) {
                att.drawBufferIndex = m_drawBuffers.size();
                m_drawBuffers.push_back(att.attachment);
            }

            if (att.shared->type == FrameBufferAttachmentType::texture) {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
            }
            else if (att.shared->type == FrameBufferAttachmentType::rbo) {
                glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, att.rbo);
            }
            else if (att.shared->type == FrameBufferAttachmentType::depth_texture) {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
            }
        } else if (att.type == FrameBufferAttachmentType::draw_buffer) {
            if (att.useDrawBuffer) {
                att.drawBufferIndex = m_drawBuffers.size();
                m_drawBuffers.push_back(att.attachment);
            }
        } else if (att.type == FrameBufferAttachmentType::texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
            glObjectLabel(GL_TEXTURE, att.textureID, attName.length(), attName.c_str());

            KI_INFO(fmt::format("CREATE_TEX: FBO={}, TEX={}", str(), att.textureID));

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

            glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
            glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

            glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));

            glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

            if (att.useDrawBuffer) {
                att.drawBufferIndex = m_drawBuffers.size();
                m_drawBuffers.push_back(att.attachment);
            }
        }
        else if (att.type == FrameBufferAttachmentType::rbo) {
            glCreateRenderbuffers(1, &att.rbo);
            glObjectLabel(GL_RENDERBUFFER, att.rbo, attName.length(), attName.c_str());

            KI_INFO(fmt::format("CREATE_RBO: FBO={}, RBO={}", str(), att.rbo));

            glNamedRenderbufferStorage(att.rbo, att.internalFormat, m_spec.width, m_spec.height);
            glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, att.rbo);

            m_hasDepth = true;
            m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else if (att.type == FrameBufferAttachmentType::depth_texture) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
            glObjectLabel(GL_TEXTURE, att.textureID, attName.length(), attName.c_str());

            KI_INFO(fmt::format("CREATE_DEPTH: FBO={}, DEPTH={}", str(), att.textureID));

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

            glTextureParameteri(att.textureID, GL_TEXTURE_MIN_FILTER, att.minFilter);
            glTextureParameteri(att.textureID, GL_TEXTURE_MAG_FILTER, att.magFilter);

            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_S, att.textureWrapS);
            glTextureParameteri(att.textureID, GL_TEXTURE_WRAP_T, att.textureWrapT);

            glTextureParameterfv(att.textureID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(att.borderColor));

            {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
            }

            m_hasDepth = true;
            m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else if (att.type == FrameBufferAttachmentType::shadow) {
            glCreateTextures(GL_TEXTURE_2D, 1, &att.textureID);
            glObjectLabel(GL_TEXTURE, att.textureID, attName.length(), attName.c_str());

            KI_INFO(fmt::format("CREATE_SHADOW: FBO={}, DEPTH={}", str(), att.textureID));

            glTextureStorage2D(att.textureID, 1, att.internalFormat, m_spec.width, m_spec.height);

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

            {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
            }

            m_hasDepth = true;
        }
    }

    {
        glNamedFramebufferDrawBuffer(m_fbo, GL_NONE);
        glNamedFramebufferReadBuffer(m_fbo, GL_NONE);

        resetDrawBuffers(FrameBuffer::RESET_DRAW_ALL);
    }

    if (m_drawBuffers.size() > 0) {
        glNamedFramebufferDrawBuffers(m_fbo, m_drawBuffers.size(), m_drawBuffers.data());
    }

    if (m_checkComplete) {
        GLenum status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::string msg = fmt::format(
                "FRAMEBUFFER:: Framebuffer is not complete! buffer={}, status=0x{:x} ({})",
                str(), status, status);
            KI_ERROR(msg);
            throw std::runtime_error{ msg };
        }
    }
}

void FrameBuffer::resetDrawBuffers(int activeCount)
{
    if (activeCount == FrameBuffer::RESET_DRAW_ACTIVE) activeCount = m_activeDrawBuffers;
    if (activeCount < 0) activeCount = m_drawBuffers.size();
    if (activeCount > m_drawBuffers.size()) activeCount = m_drawBuffers.size();

    if (m_activeDrawBuffers != activeCount) {
        m_activeDrawBuffers = activeCount;
        glNamedFramebufferDrawBuffers(m_fbo, activeCount, m_drawBuffers.data());
    }
}

void FrameBuffer::bind(const RenderContext& ctx)
{
    if (!ctx.m_batch->isFlushed()) {
        throw std::runtime_error{ fmt::format("BIND_ERROR: Batch was NOT flushed: FBO={}", str()) };
    }

    bool changed = ctx.m_state.bindFrameBuffer(m_fbo, m_forceBind);
    if (changed) {
        changed = ctx.m_state.setViewport({ 0, 0, m_spec.width, m_spec.height });
    }

    if (changed) {
        if (ctx.m_state.setBufferResolution(m_bufferInfo.u_bufferResolution)) {
            ctx.m_renderData->updateBufferInfo(m_bufferInfo);
        }
    }
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
    const glm::vec2& size,
    GLenum filter)
{
    // NOTE KI GL_COLOR_ATTACHMENT0 values irrelevant if just GL_DEPTH_BUFFER_BIT
    blit(target, mask, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, pos, size, filter);
}

// mask = GL_COLOR_BUFFER_BIT,
void FrameBuffer::blit(
    FrameBuffer* target,
    GLbitfield mask,
    GLenum sourceColorAttachment,
    GLenum targetColorAttachment,
    const glm::vec2& pos,
    const glm::vec2& size,
    GLenum filter)
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

    if (mask & GL_COLOR_BUFFER_BIT) {
        glNamedFramebufferReadBuffer(m_fbo, sourceColorAttachment);
        glNamedFramebufferDrawBuffer(target->m_fbo, targetColorAttachment);
    }

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
        filter);

    if (mask & GL_COLOR_BUFFER_BIT) {
        // NOTE KI MUST reset draw buffer state (keep current active count)
        target->resetDrawBuffers(FrameBuffer::RESET_DRAW_ACTIVE);
    }
}

void FrameBuffer::copy(
    FrameBuffer* target,
    int sourceAttachmentIndex,
    int targetAttachmentIndex)
{
    auto& srcAtt = m_spec.attachments[sourceAttachmentIndex];
    auto& dstAtt = target->m_spec.attachments[targetAttachmentIndex];

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCopyImageSubData.xhtml
    // https://gamedev.stackexchange.com/questions/194959/glcopyimagesubdata-slower-than-binding-drawing
    // https://stackoverflow.com/questions/23981016/best-method-to-copy-texture-to-texture
    glCopyImageSubData(
        srcAtt.textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
        dstAtt.textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
        m_spec.width,
        m_spec.height,
        1);
}

void FrameBuffer::clear(
    const RenderContext& ctx,
    GLbitfield clearMask,
    const glm::vec4& debugColor)
{
    const bool useDebugColor = ctx.m_assets.useDebugColor;
    const bool hasAttachments = !m_spec.attachments.empty();

    //if (clearMask & GL_COLOR_BUFFER_BIT) {
    //    if (useDebugColor) {
    //        ctx.m_state.clearColor(debugColor);
    //    }
    //    else {
    //        ctx.m_state.clearColor(BLACK_COLOR);
    //    }
    //}

    // NOTE KI if no attachments cannot know
    if (hasAttachments) {
        if (!m_hasDepth) {
            clearMask &= ~GL_DEPTH_BUFFER_BIT;
        }
        if (!m_hasStencil) {
            clearMask &= ~GL_STENCIL_BUFFER_BIT;
        }
    }

    if (clearMask == 0) return;

    if (hasAttachments) {
        for (auto& att : m_spec.attachments) {
            if (useDebugColor && att.clearMask & GL_COLOR_BUFFER_BIT) {
                glm::vec4 oldColor = att.clearColor;
                att.clearColor = debugColor;
                att.clearWithMask(m_fbo, clearMask);
                att.clearColor = oldColor;
            }
            else {
                att.clearWithMask(m_fbo, clearMask);
            }
        }
    }
    else {
        // NOTE KI *FAILS* if buffer is not bound
        glClear(clearMask);
    }
}

void FrameBuffer::clearAll() const
{
    for (const auto& att : m_spec.attachments) {
       att.clearBuffer(m_fbo);
    }
}

void FrameBuffer::clearAttachment(int attachmentIndex) const
{
    m_spec.attachments[attachmentIndex].clearBuffer(m_fbo);
}

void FrameBuffer::invalidateAttachment(
    int attachmentIndex)
{
    m_spec.attachments[attachmentIndex].invalidate(m_fbo);
}

void FrameBuffer::invalidateAll()
{
    for (const auto& att : m_spec.attachments) {
        att.invalidate(m_fbo);
    }
}

FrameBufferAttachment* FrameBuffer::getDepthAttachment()
{
    for (auto& att : m_spec.attachments) {
        if (att.attachment == GL_DEPTH_ATTACHMENT) {
            return &att;
        }
    }
    return nullptr;
}
