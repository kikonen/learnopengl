#pragma once

#include <memory>

#include "kigl/kigl.h"

struct UpdateViewContext;
class RenderContext;

namespace render {
    class FrameBuffer;
    struct FrameBufferAttachment;

    class GBuffer {
    public:
        static const int ATT_ALBEDO = GL_COLOR_ATTACHMENT0;
        static const int ATT_EMISSION = GL_COLOR_ATTACHMENT1;
        static const int ATT_NORMAL = GL_COLOR_ATTACHMENT2;
        static const int ATT_METAL = GL_COLOR_ATTACHMENT3;
        static const int ATT_VIEW_Z = GL_COLOR_ATTACHMENT4;

        static const int ATT_ALBEDO_INDEX = 0;
        //static const int ATT_SPECULAR_INDEX = 1;
        static const int ATT_EMISSION_INDEX = 1;
        static const int ATT_NORMAL_INDEX = 2;
        static const int ATT_METAL_INDEX = 3;
        static const int ATT_VIEW_Z_INDEX = 4;
        static const int ATT_DEPTH_INDEX = 5;

    public:
        GBuffer() {}
        ~GBuffer() {}

        void prepare();

        void updateRT(const UpdateViewContext& ctx);

        void bind(const RenderContext& ctx);

        void bindTexture(const RenderContext& ctx);
        void unbindTexture(const RenderContext& ctx);

        void bindDepthTexture(const RenderContext& ctx);
        void unbindDepthTexture(const RenderContext& ctx);

        void clearAll();
        void invalidateAll();

        FrameBufferAttachment* getAttachment(int attachmentIndex);

    public:
        std::unique_ptr<FrameBuffer> m_buffer{ nullptr };
        std::unique_ptr<FrameBufferAttachment> m_depthTexture;

    private:

        int m_width{ -1 };
        int m_height{ -1 };
    };
}
