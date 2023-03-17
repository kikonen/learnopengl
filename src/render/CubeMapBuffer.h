#pragma once

#include "FrameBuffer.h"

class CubeMapBuffer  final : public FrameBuffer
{
public:
    CubeMapBuffer(
        GLuint fbo,
        int size,
        GLenum side,
        GLuint textureID);

    virtual ~CubeMapBuffer() override = default;

    virtual void bind(const RenderContext& ctx);

private:
    GLenum m_side;
    GLuint m_textureID;
};
