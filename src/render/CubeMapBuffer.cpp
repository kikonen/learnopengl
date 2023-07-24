#include "CubeMapBuffer.h"

CubeMapBuffer::CubeMapBuffer(
    GLuint fbo,
    int size,
    GLenum side,
    GLuint textureID)
    : FrameBuffer( "cube_map", { size, size, {} }),
    m_side(side),
    m_textureID(textureID)
{
    m_fbo = fbo;
}

void CubeMapBuffer::bind(const RenderContext& ctx)
{
    FrameBuffer::bind(ctx);

    // NOTE KI *AFTER* binding framebuffer
    // https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glFramebufferTexture2D.xml
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        m_side,
        m_textureID,
        0);
}
