#pragma once

#include <vector>
#include <memory>

#include <util/Ref.h>

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
        int mapSize);

    ~ShadowCascade();

    void prepareRT(
        const PrepareContext& ctx);

    void clear();
    void bindTexture(kigl::GLState& state);

    glm::ivec2 getTextureSize();
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
    util::Ref<render::FrameBuffer> m_frameBuffer{ nullptr };

    size_t m_cascadeCount{ 0 };

    render::Camera m_camera;

    ki::program_id m_solidShadowProgramId{ 0 };
    ki::program_id m_alphaShadowProgramId{ 0 };
};
