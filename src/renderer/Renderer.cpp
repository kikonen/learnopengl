#include "Renderer.h"


Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    drawSkip = assets.drawSkip;
}

void Renderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
}

void Renderer::bind(const RenderContext& ctx)
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
