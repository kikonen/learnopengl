#pragma once

#include "asset/Assets.h"


class FrameBuffer;
class RenderContext;

class GBuffer {
public:
    GBuffer() {}
    ~GBuffer() {}

    void prepare(const Assets& assets);

    void updateView(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx);
    void unbindTexture(const RenderContext& ctx);

    void bindDepthTexture(const RenderContext& ctx);
    void unbindDepthTexture(const RenderContext& ctx);

public:
    std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

private:
};
