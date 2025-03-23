#pragma once

#include "Pass.h"

class EnvironmentProbeRenderer;

namespace render {
    class PassDebugEnvironmentProbe : Pass {
    public:
        PassDebugEnvironmentProbe();
        ~PassDebugEnvironmentProbe();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx, float bufferScale);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        std::unique_ptr<EnvironmentProbeRenderer> m_environmentProbeRenderer{ nullptr };
    };
}
