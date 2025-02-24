#pragma once

#include <memory>

#include "kigl/kigl.h"

#include "render/size.h"

#include "Pass.h"

class DecalRenderer;

namespace render {
    class PassDecal final : Pass {
    public:
        PassDecal();
        ~PassDecal();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx, float bufferScale);

        void initRender(const RenderContext& ctx);

        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

        PassContext renderOit(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        Program* m_program{ nullptr };

        std::unique_ptr<DecalRenderer> m_decalRenderer;
    };
}
