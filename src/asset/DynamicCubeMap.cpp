#include "DynamicCubeMap.h"

#include "glm/ext.hpp"

#include "render/CubeMap.h"

#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/FrameBufferAttachment.h"


DynamicCubeMap::DynamicCubeMap(int size)
    : m_size(size)
{
}

DynamicCubeMap::~DynamicCubeMap()
{
    glDeleteFramebuffers(1, &m_fbo);
}

void DynamicCubeMap::bindTexture(const RenderContext& ctx, int unitIndex)
{
    m_cubeMap.bindTexture(ctx, unitIndex);
}

void DynamicCubeMap::bind(const RenderContext& ctx)
{
    // NOTE KI must flush before changing render target
    ctx.m_batch->flush(ctx);

    ctx.m_state.bindFrameBuffer(m_fbo, false);
    glViewport(0, 0, m_size, m_size);
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
}

void DynamicCubeMap::prepare(
    const FrameBufferAttachment* depthAttachment,
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    // TODO KI glNamedFramebufferTexture2DEXT missing (resolved!)
    {
        glCreateFramebuffers(1, &m_fbo);
    }

    {
        glNamedFramebufferRenderbuffer(
            m_fbo,
            depthAttachment->attachment,
            GL_RENDERBUFFER,
            depthAttachment->rbo);
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
        glClearNamedFramebufferfv(m_fbo, GL_COLOR, 0, glm::value_ptr(clearColor));
        glClearNamedFramebufferfi(m_fbo, GL_DEPTH_STENCIL, 0, 1.f, 0);
    }

    m_cubeMap.m_size = m_size;
    m_cubeMap.create();

    m_valid = true;
}

CubeMapBuffer DynamicCubeMap::asFrameBuffer(int face)
{
    return {
        m_fbo,
        m_size,
        (GLenum)GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
        m_cubeMap.m_textureID,
    };
}
