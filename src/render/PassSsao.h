#pragma once

#include "Pass.h"

#include "SsaoBuffer.h"

namespace render {
    class PassDeferred;

    class PassSsao : Pass {
    public:
        PassSsao();
        ~PassSsao();

        void prepare(const PrepareContext& ctx);

        void updateRT(
            const UpdateViewContext& ctx,
            float bufferScale);

        void cleanup(const RenderContext& ctx);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        Program* m_ssaoProgram{ nullptr };
        Program* m_ssaoBlurProgram{ nullptr };

        SsaoBuffer m_ssaoBuffer;
    };
}
