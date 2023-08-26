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
    //ctx.m_batch->flush(ctx);
    if (!ctx.m_batch->isFlushed()) {
        throw std::runtime_error{ fmt::format("BIND_ERROR: Batch was NOT flushed: FBO={}", m_fbo) };
    }

    ctx.m_state.bindFrameBuffer(m_fbo, false);
    ctx.m_state.setViewport({ 0, 0, m_size, m_size });
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
    //ctx.m_state.bindFrameBuffer(0, false);
}

void DynamicCubeMap::prepare(
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    {
        glCreateFramebuffers(1, &m_fbo);
        KI_INFO(fmt::format("CREATE: FBO={}, dynamic_cube_map", m_fbo));
    }

    //GLenum status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
    //if (status != GL_FRAMEBUFFER_COMPLETE) {
    //    std::string msg = fmt::format(
    //        "FRAMEBUFFER:: Framebuffer is not complete! status=0x{:x} ({})",
    //        status, status);
    //    KI_ERROR(msg);
    //    throw std::runtime_error{ msg };
    //}

    //// NOTE KI clear buffer to avoid showing garbage
    //if (clear) {
    //    glClearNamedFramebufferfv(m_fbo, GL_COLOR, 0, glm::value_ptr(clearColor));
    //}

    m_cubeMap.m_size = m_size;
    m_cubeMap.m_internalFormat = GL_RGB16F;
    m_cubeMap.create();

    //glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    //for (unsigned int face = 0; face < 6; face++)
    //{
    //    // https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glFramebufferTexture2D.xml
    //    glFramebufferTexture2D(
    //        GL_FRAMEBUFFER,
    //        GL_COLOR_ATTACHMENT0,
    //        (GLenum)GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
    //        m_cubeMap.m_textureID,
    //        0);
    //}
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
