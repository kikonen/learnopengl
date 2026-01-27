#pragma once

#include <memory>

#include "Pass.h"

class SocketRenderer;

namespace render {
    class PassDebugSocket : Pass {
    public:
        PassDebugSocket();
        ~PassDebugSocket();

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
        std::unique_ptr<SocketRenderer> m_socketRenderer{ nullptr };
    };
}
