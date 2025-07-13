#pragma once

#include <memory>

#include "kigl/kigl.h"

#include "render/size.h"

#include "Pass.h"

namespace render {
    class PassFog final : Pass {
    public:
        PassFog();
        ~PassFog();

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
        Program* m_program{ nullptr };
    };
}
