#pragma once

#include "asset/Assets.h"
#include "asset/ShaderRegistry.h"
#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"


class Renderer
{
public:
    Renderer();
    virtual ~Renderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders);
    virtual void update(const RenderContext& ctx, const NodeRegistry& registry);
    virtual void bind(const RenderContext& ctx);

    //virtual void render(
    //    const RenderContext& ctx,
    //    const NodeRegistry& registry,
    //    SkyboxRenderer* skybox) = 0;

protected:
    bool stepRender();

protected:
    bool m_prepared = false;

    int drawIndex = 0;
    int drawSkip = 0;

    bool rendered = false;
};
