#pragma once

#include "Pass.h"

#include "OITBuffer.h"

namespace render {
    class PassDebug : Pass {
    public:
        PassDebug();
        ~PassDebug();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    private:
        void passDebug(
            const RenderContext& ctx,
            const DrawContext& drawContext);

    protected:
    };
}
