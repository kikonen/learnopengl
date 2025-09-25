#pragma once

#include<vector>
#include <memory>

#include "kigl/kigl.h"

struct UpdateViewContext;

namespace render
{
    class RenderContext;
    class FrameBuffer;
    struct FrameBufferAttachment;

    class BlurBuffer
    {
    public:
        static const int BUFFER_COUNT = 3;

        static const int ATT_COLOR_A_ENUM = GL_COLOR_ATTACHMENT0;
        static const int ATT_COLOR_B_ENUM = GL_COLOR_ATTACHMENT1;

        static const int ATT_COLOR_A_INDEX = 0;
        static const int ATT_COLOR_B_INDEX = 1;

    public:
        BlurBuffer();
        ~BlurBuffer();

        void prepare();

        void updateRT(
            const UpdateViewContext& ctx,
            const std::string& namePrefix,
            float bufferScale);

        void bind(
            const RenderContext& ctx,
            int bufferIndex);

        void bindTexture(
            kigl::GLState& state,
            int bufferIndex,
            int attachmentIndex,
            int unitIndex);

        void unbindTexture(
            kigl::GLState& state,
            int bufferIndex,
            int unitIndex);

    public:
        std::vector<std::unique_ptr<FrameBuffer>> m_buffers;

    private:
        int m_width{ -1 };
        int m_height{ -1 };
    };
}
