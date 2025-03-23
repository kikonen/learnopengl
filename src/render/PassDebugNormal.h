#pragma once

#include "Pass.h"

class NormalRenderer;

namespace render {
    class PassDebugNormal : Pass {
    public:
        PassDebugNormal();
        ~PassDebugNormal();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx, float bufferScale);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        std::unique_ptr<NormalRenderer> m_normalRenderer{ nullptr };
    };
}
