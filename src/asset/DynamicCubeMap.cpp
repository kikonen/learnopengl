#include "DynamicCubeMap.h"

#include "scene/CubeMap.h"
#include "scene/RenderContext.h"


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
    ctx.state.bindTexture(unitIndex, m_textureID, false);
}

void DynamicCubeMap::bind(const RenderContext& ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_size, m_size);
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
    if (false) {
        const auto& res = ctx.m_resolution;

        // Reset viewport back
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, res.x, res.y);
    }

    ctx.bindMatricesUBO();
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

    if (glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        KI_ERROR("FRAMEBUFFER:: Framebuffer is not complete!");
        return;
    }

    // NOTE KI clear buffer to avoid showing garbage
    if (clear) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(clearMask);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    m_textureID = CubeMap::createEmpty(m_size);
    m_valid = true;
}
