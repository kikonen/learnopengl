#pragma once

#include "FrameBuffer.h"

class CubeMapBuffer  final : public FrameBuffer
{
public:
    CubeMapBuffer(
        GLuint fbo,
        int size,
        int face,
        GLuint textureID);

    virtual ~CubeMapBuffer() override = default;

    void bindFace();

    virtual void bind(const RenderContext& ctx);

private:
    const int m_face;
    const GLuint m_textureID;
};
