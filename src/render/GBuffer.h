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
        static const int ATT_ALBEDO_ENUM = GL_COLOR_ATTACHMENT0;
        static const int ATT_EMISSION_ENUM = GL_COLOR_ATTACHMENT1;
        static const int ATT_NORMAL_ENUM = GL_COLOR_ATTACHMENT2;
        static const int ATT_MRAO_ENUM = GL_COLOR_ATTACHMENT3;
        //static const int ATT_VIEW_POSITION_ENUM = GL_COLOR_ATTACHMENT4;
        //static const int ATT_VIEW_Z_ENUM = GL_COLOR_ATTACHMENT5;

        static const int ATT_ALBEDO_INDEX = 0;
        //static const int ATT_SPECULAR_INDEX = 1;
        static const int ATT_EMISSION_INDEX = 1;
        static const int ATT_NORMAL_INDEX = 2;
        static const int ATT_MRAO_INDEX = 3;
        //static const int ATT_VIEW_POSITION_INDEX = 4;
        //static const int ATT_VIEW_Z_INDEX = 5;
        static const int ATT_DEPTH_INDEX = 4;

    public:
        GBuffer();
        ~GBuffer();

        void prepare();

        void updateRT(
            const UpdateViewContext& ctx,
            const std::string& namePrefix,
            float bufferScale);

        void bind(const RenderContext& ctx);

        void bindTexture(kigl::GLState& state);
        void unbindTexture(kigl::GLState& state);

        void bindDepthTexture(kigl::GLState& state);
        void unbindDepthTexture(kigl::GLState& state);

        void clearAll();
        void invalidateAll();

        void updateDepthCopy();

        FrameBufferAttachment* getAttachment(int attachmentIndex);

    public:
        std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

        bool m_depthCopyEnabled{ false };

    private:
        int m_width{ -1 };
        int m_height{ -1 };

        std::shared_ptr<FrameBufferAttachment> m_depthTexture;
    };
}
