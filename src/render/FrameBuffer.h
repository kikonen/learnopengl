#pragma once

#include <vector>
#include <initializer_list>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "shader/BufferInfoUBO.h"

#include "FrameBufferAttachment.h"

class RenderContext;

namespace render {
    class FrameBuffer
    {
    public:
        static const int RESET_DRAW_ALL = -2;
        static const int RESET_DRAW_ACTIVE = -1;

    public:
        FrameBuffer(
            std::string_view name,
            const FrameBufferSpecification& spec);

        virtual ~FrameBuffer();

        std::string str() const noexcept;

        virtual void prepare();

        // @param activeCount
        // FrameBuffer::RESET_DRAW_ACTIVE keep current active count
        // FrameBuffer::RESET_DRAW_ALL reset to default all buffers
        void resetDrawBuffers(int activeCount);

        void setDrawBuffer(int attachmentIndex);

        virtual void bind(const RenderContext& ctx);
        void unbind(const RenderContext& ctx);

        // NOTE KI bind if needed
        void bindTexture(
            kigl::GLState& state,
            int attachmentIndex,
            int unitIndex);

        // NOTE KI *force* unbind
        void unbindTexture(
            kigl::GLState& state,
            int unitIndex);

        void bindImageTexture(
            kigl::GLState& state,
            int attachmentIndex,
            int binding);

        // TODO KI does NOT support blit for depth only color
        void blit(
            FrameBuffer* target,
            GLbitfield mask,
            const glm::vec2& pos,
            const glm::vec2& size,
            GLenum filter) const noexcept;

        // TODO KI does NOT support blit for depth only color
        void blit(
            FrameBuffer* target,
            GLbitfield mask,
            GLenum sourceColorAttachment,
            GLenum targetColorAttachment,
            const glm::vec2& pos,
            const glm::vec2& size,
            GLenum filter) const noexcept;

        void copy(
            FrameBuffer* target,
            int sourceAttachmentIndex,
            int targetAttachmentIndex) const noexcept;

        void copy(
            FrameBufferAttachment* dstAtt,
            int sourceAttachmentIndex) const noexcept;

        void clear(
            const RenderContext& ctx,
            GLbitfield clearMask,
            const glm::vec4& clearColor);

        void clearAll() const;
        void clearAttachment(int attachmentIndex) const;

        void invalidateAttachment(
            int attachmentIndex);

        void invalidateAll();

        size_t getDrawBufferCount() const noexcept {
            return m_drawBuffers.size();
        }

        FrameBufferAttachment* getDepthAttachment();

        glm::ivec2 getSize() const noexcept
        {
            return { m_spec.width, m_spec.height };
        }

        operator int() const { return m_fbo; }

    public:
        const std::string m_name;

        FrameBufferSpecification m_spec;

        bool m_checkComplete{ true };

        GLuint m_fbo = 0;
        bool m_forceBind{ false };
        bool m_hasStencil{ false };
        bool m_hasDepth{ false };

        int m_clearMask = 0;

    protected:
        bool m_prepared = false;

        int m_activeDrawBuffers{ -1 };
        std::vector<GLenum> m_drawBuffers;
        std::vector<GLenum> m_attached;

        BufferInfoUBO m_bufferInfo;
    };

}
