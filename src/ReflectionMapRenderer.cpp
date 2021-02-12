#include "ReflectionMapRenderer.h"

ReflectionMapRenderer::ReflectionMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

ReflectionMapRenderer::~ReflectionMapRenderer()
{
}

void ReflectionMapRenderer::bind(RenderContext& ctx)
{
}

void ReflectionMapRenderer::render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes, std::map<NodeType*, std::vector<Sprite*>>& typeSprites, std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
}
