#pragma once

#include "asset/Assets.h"


class FrameBuffer;
class RenderContext;
class GBuffer;

//
// Effect buffer for NodeDraw to avoid buffer
// format, and such issues with "targetBuffer"
// i.e. uses same buffer size, etc. as GBuffer
//
class EffectBuffer {
public:
    EffectBuffer() {}
    ~EffectBuffer() {}

    void prepare(
        const Assets& assets,
        GBuffer* gBuffer);

    void updateView(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx);
    void unbindTexture(const RenderContext& ctx);

    void clear();

public:
    std::unique_ptr<FrameBuffer> m_primary{ nullptr };
    std::vector<std::unique_ptr<FrameBuffer>> m_buffers;

private:
    GBuffer* m_gBuffer{ nullptr };
};
