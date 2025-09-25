#pragma once

#include <string>
#include <memory>

namespace kigl {
    class GLState;
}

struct UpdateViewContext;

namespace render {
    class RenderContext;
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
            GBuffer* gBuffer,
            const std::string& namePrefix,
            float bufferScale);

        void bind(const RenderContext& ctx);

        void bindTexture(kigl::GLState& state);
        void unbindTexture(kigl::GLState& state);

        void clearAll();
        void invalidateAll();

    public:
        std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

    private:
        int m_width{ -1 };
        int m_height{ -1 };
    };
}
