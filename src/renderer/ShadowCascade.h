#pragma once

#include <vector>
#include <memory>

#include "asset/Assets.h"

#include "component/Camera.h"

#include "kigl/kigl.h"


class FrameBuffer;
class Registry;
class RenderContext;
class Program;

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
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx);

    GLuint getTextureID();

    void bind(const RenderContext& ctx);

    void render(
        const RenderContext& ctx);

    float getNearPlane() const noexcept { return m_camera.getNearPlane(); }
    float getFarPlane() const noexcept { return m_camera.getFarPlane(); }

private:
    void drawNodes(
        const RenderContext& ctx);

public:
    const int m_index;
    const float m_shadowBegin;
    const float m_shadowEnd;
    const int m_mapSize;

private:
    // NOTE KI std::unique_ptr triggered exhaustive error loop
    FrameBuffer* m_buffer{ nullptr };

    size_t m_cascadeCount{ 0 };

    Camera m_camera;

    Program* m_solidShadowProgram{ nullptr };
    Program* m_alphaShadowProgram{ nullptr };
};
