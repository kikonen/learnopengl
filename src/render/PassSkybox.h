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

        void updateRT(const UpdateViewContext& ctx);

        void initRender(const RenderContext& ctx);

        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        void drawSkybox(
            const RenderContext& ctx);

    protected:
        Program* m_program{ nullptr };
    };
}
