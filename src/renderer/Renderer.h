#pragma once

#include "asset/Assets.h"
#include "asset/ShaderRegistry.h"
#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"

class Renderer
{
public:
    Renderer(const Assets& assets);
    virtual ~Renderer();

    virtual void prepare(ShaderRegistry& shaders);
    virtual void update(const RenderContext& ctx, NodeRegistry& registry);
    virtual void bind(const RenderContext& ctx);
    virtual void render(const RenderContext& ctx, NodeRegistry& registry);

protected:
    bool stepRender();

protected:
    const Assets& assets;

    int drawIndex = 0;
    int drawSkip = 0;

    bool rendered = false;
};
