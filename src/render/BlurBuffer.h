#pragma once

#include<vector>
#include <memory>

#include "kigl/kigl.h"

struct UpdateViewContext;
class RenderContext;


namespace render
{
    class FrameBuffer;
    struct FrameBufferAttachment;

    class BlurBuffer
    {
    public:
        static const int ATT_COLOR_A = GL_COLOR_ATTACHMENT0;
        static const int ATT_COLOR_B = GL_COLOR_ATTACHMENT1;

        static const int ATT_COLOR_A_INDEX = 0;
        static const int ATT_COLOR_B_INDEX = 1;

    public:
        BlurBuffer();
        ~BlurBuffer();

        void prepare();

        void updateRT(const UpdateViewContext& ctx);

        void bind(
            const RenderContext& ctx,
            int bufferIndex);

        void bindTexture(
            const RenderContext& ctx,
            int bufferIndex,
            int attachmentIndex,
            int unitIndex);

        void unbindTexture(
            const RenderContext& ctx,
            int bufferIndex,
            int unitIndex);

    public:
        std::vector<std::unique_ptr<FrameBuffer>> m_buffers;

    private:
        int m_width{ -1 };
        int m_height{ -1 };
    };
}
