#include "DynamicCubeMap.h"

#include "scene/CubeMap.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"


DynamicCubeMap::DynamicCubeMap(int size)
    : m_size(size)
{
}

DynamicCubeMap::~DynamicCubeMap()
{
    glDeleteRenderbuffers(1, &m_depthBuffer);
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

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_size, m_size);
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
    //ctx.updateMatricesUBO();
}

void DynamicCubeMap::prepare(
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    int clearMask = 0;

    // TODO KI glNamedFramebufferTexture2DEXT missing
    {
        glCreateFramebuffers(1, &m_fbo);
        clearMask |= GL_COLOR_BUFFER_BIT;
    }

    {
        glCreateRenderbuffers(1, &m_depthBuffer);
        glNamedRenderbufferStorage(m_depthBuffer, GL_DEPTH_COMPONENT24, m_size, m_size);
        glNamedFramebufferRenderbuffer(m_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
        clearMask |= GL_DEPTH_BUFFER_BIT;
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
