#pragma once

#include <memory>

#include "kigl/kigl.h"

#include "render/size.h"

#include "Pass.h"

class DecalRenderer;

namespace render {
    class PassSkybox final : Pass {
    public:
        PassSkybox();
        ~PassSkybox();

        void prepare(const PrepareContext& ctx);

        void updateRT(
            const UpdateViewContext& ctx,
            const std::string& namePrefix,
            float bufferScale);

        void initRender(const RenderContext& ctx);

        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        void drawSkybox(
            const RenderContext& ctx);

    protected:
        // day-only skybox program (no UNIT_SKYBOX_NIGHT sampler)
        Program* m_program{ nullptr };
        // day+night variant (USE_SKYBOX_NIGHT); selected only when the scene has a night skybox
        Program* m_programNight{ nullptr };
    };
}
