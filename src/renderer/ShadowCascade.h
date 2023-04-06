#pragma once

#include <vector>
#include <memory>

#include "asset/Assets.h"
#include "asset/Uniform.h"


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
        float frustumSize,
        int mapSize)
    : m_index(index),
    m_shadowBegin(shadowBegin),
    m_shadowEnd(shadowEnd),
    m_frustumSize(frustumSize),
    m_mapSize(mapSize)
    {}

    ~ShadowCascade();

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx);

    GLuint getTextureID();

    void bind(const RenderContext& ctx);

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(
        const RenderContext& ctx);

public:
    const int m_index;
    const float m_shadowBegin;
    const float m_shadowEnd;
    const float m_frustumSize;
    const int m_mapSize;

    float m_nearPlane{ 0.f };
    float m_farPlane{ 0.f };

private:
    // NOTE KI std::unique_ptr triggered exhaustive error loop
    FrameBuffer* m_buffer{ nullptr };

    //Program* m_shadowProgram{ nullptr };
    Program* m_solidShadowProgram{ nullptr };
    Program* m_blendedShadowProgram{ nullptr };

    uniform::UInt u_shadowIndex{ "u_shadowIndex", UNIFORM_SHADOW_MAP_INDEX };
};

