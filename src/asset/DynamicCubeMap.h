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
    const int size;

    unsigned int textureID = -1;

    bool valid = false;

private:
    unsigned int FBO = -1;
    unsigned int depthBuffer = -1;
};

