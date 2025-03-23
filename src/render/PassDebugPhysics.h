#pragma once

#include "Pass.h"

class PhysicsRenderer;

namespace render {
    class PassDebugPhysics : Pass {
    public:
        PassDebugPhysics();
        ~PassDebugPhysics();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx, float bufferScale);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        std::unique_ptr<PhysicsRenderer> m_physicsRenderer{ nullptr };
    };
}
