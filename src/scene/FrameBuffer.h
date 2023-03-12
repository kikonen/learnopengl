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
    FrameBuffer(const FrameBufferSpecification& spec);
    virtual ~FrameBuffer();

    virtual void prepare(
        const bool clear,
        const glm::vec4& clearColor);

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

    void clear(
        const RenderContext& ctx,
        const glm::vec4& clearColor);

public:
    FrameBufferSpecification m_spec;

    GLuint m_fbo = 0;

    int m_clearMask = 0;

protected:
    bool m_prepared = false;
    std::vector<GLenum> m_drawBuffers;
};

