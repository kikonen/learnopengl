#pragma once

#include "asset/Assets.h"

#include "registry/ShaderRegistry.h"
#include "registry/NodeRegistry.h"

class RenderContext;

class Renderer
{
public:
    Renderer();
    virtual ~Renderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders);
    virtual void update(const RenderContext& ctx);

    //virtual void render(
    //    const RenderContext& ctx,
    //    SkyboxRenderer* skybox) = 0;

protected:
    bool needRender(const RenderContext& ctx);

protected:
    bool m_prepared = false;

    float m_renderFrequency = 0.f;
    float m_elapsedTime = 0.f;

    unsigned long m_lastHitFrame = 0;

    bool m_rendered = false;
};
