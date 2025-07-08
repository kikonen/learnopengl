#pragma once

#include <memory>

namespace kigl {
    class GLState;
}

struct UpdateViewContext;
class RenderContext;

namespace render {
    class FrameBuffer;

    class SsaoBuffer {
    public:
        static const int ATT_SSAO_INDEX = 0;
        static const int ATT_SSAO_BLUR_INDEX = 1;

    public:
        SsaoBuffer();
        ~SsaoBuffer();

        void prepare();

        void updateRT(
            const UpdateViewContext& ctx,
            float bufferScale);

        void bind(const RenderContext& ctx);

        void bindSsaoTexture(kigl::GLState& state);
        void bindSsaoBlurTexture(kigl::GLState& state);

        void unbindSsaoTexture(kigl::GLState& state);
        void unbindSsaoBlurTexture(kigl::GLState& state);

        void clearAll();
        void invalidateAll();

    public:
        std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

    private:
        int m_width{ -1 };
        int m_height{ -1 };
    };
}

