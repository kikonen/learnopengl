#pragma once

#include "asset/Assets.h"


class FrameBuffer;
struct FrameBufferAttachment;
class UpdateViewContext;
class RenderContext;

class GBuffer {
public:
    static const int ATT_ALBEDO_INDEX = 0;
    //static const int ATT_SPECULAR_INDEX = 1;
    static const int ATT_EMISSION_INDEX = 1;
    static const int ATT_NORMAL_INDEX = 2;
    static const int ATT_METAL_INDEX = 3;
    static const int ATT_DEPTH_INDEX = 4;

public:
    GBuffer() {}
    ~GBuffer() {}

    void prepare(const Assets& assets);

    void updateRT(const UpdateViewContext& ctx);

    void bind(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx);
    void unbindTexture(const RenderContext& ctx);

    void bindDepthTexture(const RenderContext& ctx);
    void unbindDepthTexture(const RenderContext& ctx);

    void clearAll();
    void invalidateAll();

public:
    std::unique_ptr<FrameBuffer> m_buffer{ nullptr };
    std::unique_ptr<FrameBufferAttachment> m_depthTexture;

private:

    int m_width{ -1 };
    int m_height{ -1 };
};
