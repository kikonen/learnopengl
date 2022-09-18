#include "Renderer.h"


Renderer::Renderer(const Assets& assets)
    : assets(assets)
{
}

Renderer::~Renderer()
{
}

void Renderer::prepare(ShaderRegistry& shaders)
{
    drawSkip = assets.drawSkip;
}

void Renderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
}

void Renderer::bind(const RenderContext& ctx)
{
}

void Renderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
}

bool Renderer::stepRender()
{
    drawIndex++;
    if (drawIndex >= drawSkip) {
        drawIndex = 0;
        return true;
    }
    return false;
}
