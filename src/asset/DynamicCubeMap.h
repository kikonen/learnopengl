#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

#include "render/CubeMap.h"
#include "render/CubeMapBuffer.h"

class RenderContext;
struct FrameBufferAttachment;

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

    CubeMapBuffer asFrameBuffer(int side);

public:
    const int m_size;

    CubeMap m_cubeMap{ true };

    bool m_valid = false;

    bool m_rendered = false;
    int m_updateFace = -1;

    GLuint m_fbo = 0;

private:
    bool m_prepared = false;

};
