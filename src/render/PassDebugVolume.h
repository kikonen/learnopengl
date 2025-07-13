#pragma once

#include "Pass.h"

class VolumeRenderer;

namespace render {
    class PassDebugVolume : Pass {
    public:
        PassDebugVolume();
        ~PassDebugVolume();

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

    protected:
        std::unique_ptr<VolumeRenderer> m_volumeRenderer{ nullptr };
    };
}
