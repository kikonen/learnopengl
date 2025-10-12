#pragma once

#include <string>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLFrameBufferHandle.h"

#include "render/CubeMap.h"
#include "render/CubeMapBuffer.h"

namespace render
{
    class RenderContext;
}

struct PrepareContext;
struct FrameBufferAttachment;

class DynamicCubeMap
{
public:
    DynamicCubeMap(
        std::string_view name,
        int size);

    ~DynamicCubeMap();

    void prepareRT(
        const PrepareContext& ctx,
        const bool clear,
        const glm::vec4& clearColor);

    const kigl::GLTextureHandle& getTextureHandle() const;

    void bindTexture(
        kigl::GLState& state,
        int unitIndex);

    void unbindTexture(
        kigl::GLState& state,
        int unitIndex);

    void bind(const render::RenderContext& ctx);
    void unbind(const render::RenderContext& ctx);

    render::CubeMapBuffer asFrameBuffer(int side);

public:
    const int m_size;
    std::string m_name;

    render::CubeMap m_cubeMap;

    bool m_valid{ false };

    bool m_rendered{ false };
    int m_updateFace{ -1 };

    kigl::GLFrameBufferHandle m_fbo;

private:
    bool m_prepared{ false };

};
