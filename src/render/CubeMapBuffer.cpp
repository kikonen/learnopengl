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
    auto albedo = FrameBufferAttachment::getTextureRGBHdr();
    albedo.minFilter = GL_LINEAR;
    albedo.magFilter = GL_LINEAR;
    albedo.textureWrapS = GL_REPEAT;
    albedo.textureWrapT = GL_REPEAT;
    m_spec.attachments.emplace_back(FrameBufferAttachment::getShared(&albedo));

    m_fbo = fbo;
}

void CubeMapBuffer::bind(const RenderContext& ctx)
{
    FrameBuffer::bind(ctx);

    // TODO KI *WHY* this cannot be done in prepare
    // => is it truly required to be done on *every* render cycle again
    // NOTE KI *AFTER* binding framebuffer
    // https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glFramebufferTexture2D.xml
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        m_side,
        m_textureID,
        0);
}
