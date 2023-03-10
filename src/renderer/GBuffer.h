#pragma once

#include "asset/Assets.h"


#include "scene/FrameBuffer.h"


class RenderContext;

class GBuffer {
public:
    GBuffer() {}

    void prepare(const Assets& assets);

    void update(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx);
    void unbindTexture(const RenderContext& ctx);

    void blit(
        FrameBuffer* target,
        const glm::vec2& pos,
        const glm::vec2& size);

public:
    std::unique_ptr<FrameBuffer> m_buffer{ nullptr };

private:
};
