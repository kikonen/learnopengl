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
    ShadowCascade(int level)
    : m_level(level)
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
    const int m_level;

private:
    // NOTE KI std::unique_ptr triggered exhaustive error loop
    FrameBuffer* m_buffer{ nullptr };

    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.f;
    float m_frustumSize = 100.f;

    //Program* m_shadowProgram{ nullptr };
    Program* m_solidShadowProgram{ nullptr };
    Program* m_blendedShadowProgram{ nullptr };
};

