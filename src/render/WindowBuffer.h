#pragma once

#include "FrameBuffer.h"

struct UpdateViewContext;

namespace render {
    class WindowBuffer final : public FrameBuffer
    {
    public:
        WindowBuffer(
            bool forceBind,
            bool srgbEnabled) : WindowBuffer(0, forceBind, srgbEnabled) {}

        WindowBuffer(
            GLuint fbo,
            bool forceBind,
            bool srgbEnabled);

        virtual ~WindowBuffer() override {};

        void updateView(const UpdateViewContext& ctx);

        bool isSrgbEnabled() const noexcept override
        {
            return m_srgbEnabled;
        }

    private:
        bool m_srgbEnabled;
    };
}
