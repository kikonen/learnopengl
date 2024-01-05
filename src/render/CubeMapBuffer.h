#pragma once

#include "FrameBuffer.h"

namespace render {
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

        virtual void bind(const RenderContext& ctx) override;

    private:
        const int m_face;
        const GLuint m_textureID;
    };
}
