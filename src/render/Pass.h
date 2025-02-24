#pragma once

#include <string>
#include <memory>

#include "kigl/GLBlendMode.h"
#include "kigl/GLStencilMode.h"

#include "render/size.h"

#include "TextureQuad.h"
#include "ScreenTri.h"

#include "DrawContext.h"
#include "PassContext.h"

struct UpdateViewContext;
struct PrepareContext;
class RenderContext;

class Program;

namespace render {
    class FrameBuffer;

    class Pass {
    public:
        Pass(const std::string& name);
        ~Pass();

        void prepare(const PrepareContext& ctx);

        void updateRT(const UpdateViewContext& ctx);

        void cleanup(const RenderContext& ctx);

        void initRender(const RenderContext& ctx);

        // @return next {src-buffer, src-attachment}
        PassContext render(
            const RenderContext& ctx,
            const DrawContext& drawContext,
            const PassContext& src);

    protected:
        bool updateSize(const UpdateViewContext& ctx, float bufferScale);

        void startScreenPass(
            const RenderContext& ctx,
            bool useStencil,
            const kigl::GLStencilMode& stencil,
            bool useBlend,
            const kigl::GLBlendMode& blend);

        void stopScreenPass(
            const RenderContext& ctx);

    public:
        const std::string m_name;

    protected:
        bool m_enabled{ true };
        bool m_enabledBlend{ true };

        int m_width{ -1 };
        int m_height{ -1 };

        render::TextureQuad& m_textureQuad;
        render::ScreenTri& m_screenTri;
    };
}
