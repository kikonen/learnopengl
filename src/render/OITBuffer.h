#pragma once

#include <memory>

struct UpdateViewContext;
class RenderContext;

namespace render {
    class FrameBuffer;
    class GBuffer;

    class OITBuffer {
    public:
        static const int ATT_ACCUMULATOR_INDEX = 0;
        static const int ATT_REVEAL_INDEX = 1;
        static const int ATT_EMISSION_INDEX = 2;

    public:
        OITBuffer() {}
        ~OITBuffer() {}

        void prepare();

        void updateRT(
            const UpdateViewContext& ctx,
            GBuffer* gBuffer);

        void bind(const RenderContext& ctx);

        void bindTexture(const RenderContext& ctx);
        void unbindTexture(const RenderContext& ctx);

        void clearAll();
        void invalidateAll();

    public:
        std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

    private:
        int m_width{ -1 };
        int m_height{ -1 };
    };
}
