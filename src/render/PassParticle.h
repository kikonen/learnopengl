#pragma once

#include <memory>

#include "kigl/kigl.h"

#include "render/size.h"

#include "Pass.h"

class ParticleRenderer;

namespace render {
    class PassParticle final : Pass {
    public:
        PassParticle();
        ~PassParticle();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void initRender(const RenderContext& ctx);

        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        Program* m_program{ nullptr };

        std::unique_ptr<ParticleRenderer> m_particleRenderer;
    };
}
