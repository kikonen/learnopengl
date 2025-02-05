#pragma once

#include "Pass.h"

namespace render {
    class PassCopy : Pass {
    public:
        PassCopy();
        ~PassCopy();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext copy(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src,
            FrameBuffer* targetBuffer);

    private:
        void passCopy(
            const RenderContext& ctx,
            const PassContext& src,
            FrameBuffer* dstBuffer,
            GLbitfield copyMask);

    protected:
    };
}
