#pragma once

#include "FrameBuffer.h"

class UpdateViewContext;

namespace render {
    class WindowBuffer final : public FrameBuffer
    {
    public:
        WindowBuffer(bool forceBind) : WindowBuffer(0, forceBind) {}

        WindowBuffer(GLuint fbo, bool forceBind);

        virtual ~WindowBuffer() override {};

        void updateRT(const UpdateViewContext& ctx);

    private:
    };
}
