#include "DynamicCubeMap.h"

#include "scene/CubeMap.h"


DynamicCubeMap::DynamicCubeMap(int size)
    : size(size)
{
}

DynamicCubeMap::~DynamicCubeMap()
{
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &FBO);
}

void DynamicCubeMap::bindTexture(const RenderContext& ctx, int unitIndex)
{
    ctx.state.bindTexture(unitIndex, textureID);
}

void DynamicCubeMap::bind(const RenderContext& ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, size, size);
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
    KI_GL_UNBIND(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

    // Reset viewport back
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, ctx.width, ctx.height);
    ctx.bindMatricesUBO();
}

void DynamicCubeMap::prepare(
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    int clearMask = 0;

    {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        clearMask |= GL_COLOR_BUFFER_BIT;
    }

    {
        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        clearMask |= GL_DEPTH_BUFFER_BIT;
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        KI_ERROR("FRAMEBUFFER:: Framebuffer is not complete!");
        return;
    }

    // NOTE KI clear buffer to avoid showing garbage
    if (clear) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(clearMask);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    textureID = CubeMap::createEmpty(size);
    valid = true;
}
