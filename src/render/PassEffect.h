#pragma once

#include "Pass.h"

namespace render {
    class PassEffect : Pass {
    public:
        PassEffect();
        ~PassEffect();

        void prepare(const PrepareContext& ctx);

        void updateRT(
            const UpdateViewContext& ctx,
            const std::string& namePrefix,
            float bufferScale);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    private:
        void passEffect(
            const RenderContext& ctx,
            const DrawContext& drawContext);

    protected:
    };
}
