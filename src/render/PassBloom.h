#pragma once

#include "Pass.h"

#include "BlurBuffer.h"

namespace render {
    class PassBloom : Pass {
    public:
        PassBloom();
        ~PassBloom();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    private:
        void passBloom(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        Program* m_bloomInitProgram{ nullptr };
        Program* m_bloomBlurProgram{ nullptr };
        Program* m_bloomFinalProgram{ nullptr };
        Program* m_blurVerticalProgram{ nullptr };
        Program* m_blurHorizontalProgram{ nullptr };
        Program* m_blurFinalProgramCS{ nullptr };

        int m_effectBloomIterations{ 0 };

        BlurBuffer m_blurBuffer;
    };
}
