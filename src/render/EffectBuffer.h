#pragma once

#include "asset/Assets.h"

struct UpdateViewContext;
class RenderContext;

namespace render {
    class FrameBuffer;
    class GBuffer;

    //
    // Effect buffer for NodeDraw to avoid buffer
    // format, and such issues with "targetBuffer"
    // i.e. uses same buffer size, etc. as GBuffer
    //
    class EffectBuffer {
    public:
        static const int ATT_ALBEDO_INDEX = 0;
        static const int ATT_BRIGHT_INDEX = 1;
        static const int ATT_DEPTH_INDEX = 2;
        static const int ATT_WORK_INDEX = 0;

    public:
        EffectBuffer() {}
        ~EffectBuffer() {}

        void prepare(
            const Assets& assets,
            GBuffer* gBuffer);

        void updateRT(const UpdateViewContext& ctx);

        void clearAll();
        void invalidateAll();

        void unbindTexture(const RenderContext& ctx);

    public:
        std::unique_ptr<FrameBuffer> m_primary{ nullptr };
        std::unique_ptr<FrameBuffer> m_secondary{ nullptr };
        std::vector<std::unique_ptr<FrameBuffer>> m_buffers;

    private:
        GBuffer* m_gBuffer{ nullptr };

        int m_width{ -1 };
        int m_height{ -1 };
    };
}
