#pragma once

#include "asset/Assets.h"

class UpdateViewContext;
class RenderContext;

namespace render {
    class FrameBuffer;
    class GBuffer;

    class OITBuffer {
    public:
        static const int ATT_ACCUMULATOR_INDEX = 0;
        static const int ATT_REVEAL_INDEX = 1;

    public:
        OITBuffer() {}
        ~OITBuffer() {}

        void prepare(
            const Assets& assets,
            GBuffer* gBuffer);

        void updateRT(const UpdateViewContext& ctx);

        void bind(const RenderContext& ctx);

        void bindTexture(const RenderContext& ctx);
        void unbindTexture(const RenderContext& ctx);

        void clearAll();
        void invalidateAll();

    public:
        std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

    private:
        GBuffer* m_gBuffer{ nullptr };

        int m_width{ -1 };
        int m_height{ -1 };
    };
}
