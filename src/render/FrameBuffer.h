#pragma once

#include <vector>
#include <initializer_list>

#include <glm/glm.hpp>

#include "ki/GL.h"

#include "FrameBufferAttachment.h"


class RenderContext;

class FrameBuffer
{
public:
    FrameBuffer(
        const std::string& name,
        const FrameBufferSpecification& spec);

    virtual ~FrameBuffer();

    const std::string str() const noexcept;

    virtual void prepare(
        const bool clear);

    virtual void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);

    // NOTE KI bind if needed
    void bindTexture(const RenderContext& ctx, int attachmentIndex, int unitIndex);

    // NOTE KI *force* unbind
    void unbindTexture(const RenderContext& ctx, int unitIndex);

    void blit(
        FrameBuffer* target,
        GLbitfield mask,
        const glm::vec2& pos,
        const glm::vec2& size);

    void blit(
        FrameBuffer* target,
        GLbitfield mask,
        GLenum sourceColorAttachment,
        GLenum targetColorAttachment,
        const glm::vec2& pos,
        const glm::vec2& size);

    void clear(
        const RenderContext& ctx,
        GLbitfield clearMask,
        const glm::vec4& clearColor);

    void clearAll() const;
    void clearAttachment(int attachmentIndex) const;

    int getDrawBufferCount() const noexcept {
        return m_drawBuffers.size();
    }

public:
    const std::string m_name;

    FrameBufferSpecification m_spec;

    GLuint m_fbo = 0;
    bool m_forceBind{ false };

    int m_clearMask = 0;

protected:
    bool m_prepared = false;
    std::vector<GLenum> m_drawBuffers;
};

