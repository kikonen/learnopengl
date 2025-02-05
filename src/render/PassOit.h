#pragma once

#include "Pass.h"

#include "OITBuffer.h"

namespace render {
    class PassDeferred;

    class PassOit : Pass {
    public:
        PassOit();
        ~PassOit();

        void prepare(const PrepareContext& ctx);

        void updateRT(
            const UpdateViewContext& ctx,
            PassDeferred* passDeferrred);

        void cleanup(const RenderContext& ctx);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

        PassContext blend(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    private:
        void passOit(
            const RenderContext& ctx,
            const DrawContext& drawContext);

    protected:
        Program* m_oitProgram{ nullptr };
        Program* m_oitBlendProgram{ nullptr };

        OITBuffer m_oitBuffer;
    };
}
