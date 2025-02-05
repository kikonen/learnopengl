#pragma once

#include "Pass.h"

#include "GBuffer.h"

namespace render {
    class FrameBuffer;

    class PassDeferred : Pass {
    public:
        static const int ATT_ALBEDO_ENUM = GL_COLOR_ATTACHMENT0;
        static const int ATT_ALBEDO_INDEX = 0;
        static const int ATT_DEPTH_INDEX = 1;

        PassDeferred();
        ~PassDeferred();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void cleanup(const RenderContext& ctx);

        void initRender(
            const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext preDepth(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

        // @return next {src-buffer, src-attachment}
        PassContext combine(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

        GBuffer* getGBuffer() noexcept
        {
            return &m_gBuffer;
        }

    private:
        void passPreDepth(
            const RenderContext& ctx,
            const DrawContext& drawContext);

        void passDraw(
            const RenderContext& ctx,
            const DrawContext& drawContext);

    protected:
        bool m_preDepthEnabled{ true };

        GBuffer m_gBuffer;
        std::unique_ptr<FrameBuffer> m_buffer;

        Program* m_combineProgram{ nullptr };
    };
}
