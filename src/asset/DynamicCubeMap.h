#pragma once

#include "ki/GL.h"
#include "scene/RenderContext.h"

class DynamicCubeMap
{
public:
    DynamicCubeMap(int size);
    ~DynamicCubeMap();

    void prepare(
        const bool clear,
        const glm::vec4& clearColor);

    void bindTexture(const RenderContext& ctx, int unitIndex);

    void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);

public:
    const int m_size;

    unsigned int m_textureID = 0;

    bool m_valid = false;

private:
    bool m_prepared = false;

    unsigned int m_fbo = 0;
    unsigned int m_depthBuffer = 0;
};

