#include "FrameBuffer.h"

#include <fmt/format.h>

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "asset/Assets.h"

#include "kigl/GLState.h"

#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/RenderData.h"


namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };
}

namespace render {
    FrameBuffer::FrameBuffer(
        std::string_view name,
        const FrameBufferSpecification& spec)
        : m_name{ name },
        m_spec{ spec }
    {
        m_bufferInfo.u_bufferResolution = { m_spec.width, m_spec.height };
    }

    FrameBuffer::~FrameBuffer()
    {
        if (!m_prepared) return;

        KI_INFO(fmt::format("DELETE: {}", str()));

        if (false) {
            if (m_attached.size() > 0) {
                glInvalidateNamedFramebufferData(m_fbo, static_cast<int>(m_attached.size()), m_attached.data());

                for (auto& att : m_spec.attachments) {
                    const auto type = att.type == FrameBufferAttachmentType::shared
                        ? att.shared->type
                        : att.type;

                    if (att.type == FrameBufferAttachmentType::texture) {
                        glNamedFramebufferTexture(m_fbo, att.attachment, 0, 0);
                    }
                    else if (att.type == FrameBufferAttachmentType::rbo) {
                        glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, 0);
                    }
                    else if (att.type == FrameBufferAttachmentType::depth_texture) {
                        glNamedFramebufferTexture(m_fbo, att.attachment, 0, 0);
                    }
                    else if (att.type == FrameBufferAttachmentType::depth_stencil_texture) {
                        glNamedFramebufferTexture(m_fbo, att.attachment, 0, 0);
                    }
                }
            }
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }

        glDeleteFramebuffers(1, &m_fbo);
        m_spec.attachments.clear();

        kigl::GLState::get().invalidateFrameBuffer();
    }

    std::string FrameBuffer::str() const noexcept
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
            kigl::setLabel(GL_FRAMEBUFFER, m_fbo, m_name);

            KI_INFO(fmt::format("CREATE: FBO={}", str()));
        }

        {
            int idx = 0;
            for (auto& att : m_spec.attachments) {
                att.index = idx++;
            }
        }

        for (auto& att : m_spec.attachments) {
            att.create(m_name, m_spec.width, m_spec.height);
            m_attached.push_back(att.attachment);
        }

        for (auto& att : m_spec.attachments) {
            if (att.type == FrameBufferAttachmentType::shared) {
                if (att.shared->type == FrameBufferAttachmentType::texture) {
                    glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
                }
                else if (att.shared->type == FrameBufferAttachmentType::rbo) {
                    glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, att.rbo);

                    m_depthIndex = att.index;
                    m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
                }
                else if (att.shared->type == FrameBufferAttachmentType::depth_texture) {
                    glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

                    m_depthIndex = att.index;
                    m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
                }
                else if (att.shared->type == FrameBufferAttachmentType::depth_stencil_texture) {
                    glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

                    m_depthIndex = att.index;
                    m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
                }
            }
            else if (att.type == FrameBufferAttachmentType::draw_buffer) {
                // NOTE KI draw_buffer, "non attached" texture handled externally
            }
            else if (att.type == FrameBufferAttachmentType::texture) {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);
            }
            else if (att.type == FrameBufferAttachmentType::rbo) {
                glNamedFramebufferRenderbuffer(m_fbo, att.attachment, GL_RENDERBUFFER, att.rbo);

                m_depthIndex = att.index;
                m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
            }
            else if (att.type == FrameBufferAttachmentType::depth_texture) {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

                m_depthIndex = att.index;
                m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
            }
            else if (att.type == FrameBufferAttachmentType::depth_stencil_texture) {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

                m_depthIndex = att.index;
                m_hasStencil = att.attachment == GL_DEPTH_STENCIL_ATTACHMENT;
            }
            else if (att.type == FrameBufferAttachmentType::shadow) {
                glNamedFramebufferTexture(m_fbo, att.attachment, att.textureID, 0);

                m_depthIndex = att.index;
            }
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

        {
            glNamedFramebufferReadBuffer(m_fbo, GL_NONE);
            resetDrawBuffers();
            clearAll();
        }
    }

    void FrameBuffer::resetDrawBuffers()
    {
        m_activeDrawBuffers.reserve(m_spec.attachments.size());

        m_activeDrawBuffers.clear();
        for (auto& att : m_spec.attachments) {
            if (att.useDrawBuffer) {
                m_activeDrawBuffers.push_back(att.attachment);
                att.activeDrawBufferIndex = static_cast<int>(m_activeDrawBuffers.size() - 1);
            }
            else {
                att.activeDrawBufferIndex = -1;
            }
        }

        activateDrawBuffers();
    }

    void FrameBuffer::activateDrawBuffers()
    {
        if (!m_activeDrawBuffers.empty()) {
            glNamedFramebufferDrawBuffers(m_fbo, static_cast<int>(m_activeDrawBuffers.size()), m_activeDrawBuffers.data());
        }
        else {
            glNamedFramebufferDrawBuffer(m_fbo, GL_NONE);
        }
    }

    void FrameBuffer::removeDrawBuffers()
    {
        m_activeDrawBuffers.clear();
        activateDrawBuffers();
    }

    void FrameBuffer::setDrawBuffer(int attachmentIndex)
    {
        m_activeDrawBuffers.clear();
        for (auto& att : m_spec.attachments) {
            att.activeDrawBufferIndex = -1;
        }

        {
            auto& att = m_spec.attachments[attachmentIndex];
            m_activeDrawBuffers.push_back(att.attachment);
            att.activeDrawBufferIndex = 0;
        }

        activateDrawBuffers();
    }

    void FrameBuffer::bind(const RenderContext& ctx)
    {
        auto& state = ctx.m_state;

        ctx.validateRender("FBO");

        bool changed = state.bindFrameBuffer(m_fbo, m_forceBind);
        if (changed) {
            changed = state.setViewport({ 0, 0, m_spec.width, m_spec.height });
        }

        if (changed) {
            if (state.setBufferResolution(m_bufferInfo.u_bufferResolution)) {
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

    void FrameBuffer::bindTexture(
        kigl::GLState& state,
        int attachmentIndex,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_spec.attachments[attachmentIndex].textureID, false);
    }

    void FrameBuffer::unbindTexture(
        kigl::GLState & state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, 0, true);
    }

    void FrameBuffer::bindImageTexture(
        kigl::GLState& state,
        int attachmentIndex,
        int binding)
    {
        const auto& att = m_spec.attachments[attachmentIndex];

        glBindImageTexture(
            0,
            att.textureID,
            0,
            GL_FALSE,
            0,
            GL_READ_WRITE,
            att.internalFormat);
    }

    // mask = GL_COLOR_BUFFER_BIT,
    void FrameBuffer::blit(
        FrameBuffer* target,
        GLbitfield mask,
        const glm::vec2& pos,
        const glm::vec2& size,
        GLenum filter) const noexcept
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
        GLenum filter) const noexcept
    {
        target->m_saveDrawBuffers = target->m_activeDrawBuffers;

        const float srcW = static_cast<float>(m_spec.width);
        const float srcH = static_cast<float>(m_spec.height);

        const float dstW = static_cast<float>(target->m_spec.width);
        const float dstH = static_cast<float>(target->m_spec.height);

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
            static_cast<GLuint>(s0.x),
            static_cast<GLuint>(s0.y),
            static_cast<GLuint>(s1.x),
            static_cast<GLuint>(s1.y),
            static_cast<GLuint>(d0.x),
            static_cast<GLuint>(d0.y),
            static_cast<GLuint>(d1.x),
            static_cast<GLuint>(d1.y),
            mask,
            filter);

        if (mask & GL_COLOR_BUFFER_BIT) {
            // NOTE KI MUST reset draw buffer state (keep current active count)
            target->m_activeDrawBuffers = target->m_saveDrawBuffers;
            target->resetDrawBuffers();
        }
    }

    void FrameBuffer::copy(
        FrameBuffer* target,
        int sourceAttachmentIndex,
        int targetAttachmentIndex) const noexcept
    {
        auto& dstAtt = target->m_spec.attachments[targetAttachmentIndex];
        copy(&dstAtt, sourceAttachmentIndex);
    }

    void FrameBuffer::copy(
        FrameBufferAttachment* dstAtt,
        int sourceAttachmentIndex) const noexcept
    {
        auto& srcAtt = m_spec.attachments[sourceAttachmentIndex];

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCopyImageSubData.xhtml
        // https://gamedev.stackexchange.com/questions/194959/glcopyimagesubdata-slower-than-binding-drawing
        // https://stackoverflow.com/questions/23981016/best-method-to-copy-texture-to-texture
        glCopyImageSubData(
            srcAtt.textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            dstAtt->textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            m_spec.width,
            m_spec.height,
            1);
    }

    void FrameBuffer::clear(
        const RenderContext& ctx,
        GLbitfield clearMask,
        const glm::vec4& debugColor)
    {
        const auto& assets = ctx.m_assets;

        const bool useDebugColor = assets.useDebugColor;
        const bool hasAttachments = !m_spec.attachments.empty();

        //if (clearMask & GL_COLOR_BUFFER_BIT) {
        //    if (useDebugColor) {
        //        ki::GLState.get().clearColor(debugColor);
        //    }
        //    else {
        //        ki::GLState.get().clearColor(BLACK_COLOR);
        //    }
        //}

        // NOTE KI if no attachments cannot know
        if (hasAttachments) {
            if (m_depthIndex >= 0) {
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
            if (m_fbo != 0) {
                KI_WARN_OUT(fmt::format("FBO clear: {}", str()));
            }
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
        return m_depthIndex >= 0 ? &m_spec.attachments[m_depthIndex] : nullptr;
    }
}
