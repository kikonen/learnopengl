#pragma once

#include "asset/Assets.h"


class FrameBuffer;
class RenderContext;

class OITBuffer {
public:
    OITBuffer() {}
    ~OITBuffer() {}

    void prepare(const Assets& assets);

    void updateView(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx);
    void unbindTexture(const RenderContext& ctx);

public:
    std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

private:
};
