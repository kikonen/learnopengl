#include "CubeMapBuffer.h"

namespace render {
    CubeMapBuffer::CubeMapBuffer(
        GLuint fbo,
        int size,
        int face,
        GLuint textureID)
        : FrameBuffer(
            "cube_map",
            {
                size, size,
                {}
            }),
        m_face(face),
        m_textureID(textureID)
    {
        m_fbo = fbo;

        {
            auto albedo = FrameBufferAttachment::getDrawBuffer(GL_COLOR_ATTACHMENT0);
            albedo.minFilter = GL_LINEAR_MIPMAP_NEAREST;
            albedo.magFilter = GL_NEAREST;
            albedo.textureWrapS = GL_REPEAT;
            albedo.textureWrapT = GL_REPEAT;
            albedo.clearType = ClearType::FLOAT;
            albedo.clearMask = GL_COLOR_BUFFER_BIT;
            albedo.externalDelete = true;

            m_spec.attachments.emplace_back(albedo);
        }
    }

    void CubeMapBuffer::bindFace()
    {
        // NOTE KI side vs. face difference
        // https://stackoverflow.com/questions/55169053/opengl-render-to-cubemap-using-dsa-direct-state-access
        glNamedFramebufferTextureLayer(
            m_fbo,
            GL_COLOR_ATTACHMENT0,
            m_textureID,
            0,
            m_face);
    }

    void CubeMapBuffer::bind(const RenderContext& ctx)
    {
        FrameBuffer::bind(ctx);
    }

}
