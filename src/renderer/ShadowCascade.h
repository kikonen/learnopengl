#pragma once

#include <vector>
#include <memory>

#include "render/Camera.h"

#include "kigl/kigl.h"

namespace render {
    class FrameBuffer;
}

namespace render
{
    class RenderContext;
}

struct PrepareContext;

class Registry;
class Program;

struct ShadowUBO;

class ShadowCascade final {
public:
    ShadowCascade(
        int index,
        float shadowBegin,
        float shadowEnd,
        int mapSize)
    : m_index(index),
    m_shadowBegin(shadowBegin),
    m_shadowEnd(shadowEnd),
    m_mapSize(mapSize)
    {}

    ~ShadowCascade();

    void prepareRT(
        const PrepareContext& ctx);

    void bindTexture(kigl::GLState& state);

    GLuint getTextureID();

    void bind(
        const render::RenderContext& ctx,
        ShadowUBO& shadowUbo);

    void render(
        const render::RenderContext& ctx);

    float getNearPlane() const noexcept { return m_camera.getNearPlane(); }
    float getFarPlane() const noexcept { return m_camera.getFarPlane(); }

private:
    void drawNodes(
        const render::RenderContext& ctx);

public:
    const int m_index;
    const float m_shadowBegin;
    const float m_shadowEnd;
    const int m_mapSize;

private:
    // NOTE KI std::unique_ptr triggered exhaustive error loop
    render::FrameBuffer* m_buffer{ nullptr };

    size_t m_cascadeCount{ 0 };

    render::Camera m_camera;

    ki::program_id m_solidShadowProgramId{ 0 };
    ki::program_id m_alphaShadowProgramId{ 0 };
};
