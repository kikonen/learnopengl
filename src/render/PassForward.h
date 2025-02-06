#pragma once

#include "Pass.h"

namespace render {
    class PassForward : Pass {
    public:
        PassForward();
        ~PassForward();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx, float bufferScale);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    private:
        void passForward(
            const RenderContext& ctx,
            const DrawContext& drawContext);

    protected:
    };
}
