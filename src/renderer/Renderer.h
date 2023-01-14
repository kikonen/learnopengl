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

    virtual void prepare(
        const Assets& assets,
        ShaderRegistry& shaders,
        MaterialRegistry& materialRegistry);

    virtual void update(const RenderContext& ctx);

    //virtual void render(
    //    const RenderContext& ctx,
    //    SkyboxRenderer* skybox) = 0;

protected:
    bool needRender(const RenderContext& ctx);
    void setClosest(Node* closest, int tagIndex);

protected:
    bool m_prepared = false;

    int m_renderFrameStart = 0;
    int m_renderFrameStep = 0;

    float m_elapsedSecs = -1;

    float m_lastHitTime = 0.f;
    unsigned long m_lastHitFrame = 0;

    bool m_rendered = false;

    Node* m_lastClosest{ nullptr };
};
