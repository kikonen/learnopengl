#include "TerrainRenderer.h"

TerrainRenderer::TerrainRenderer(const Assets& assets)
	: Renderer(assets)
{
}

TerrainRenderer::~TerrainRenderer()
{
}

void TerrainRenderer::prepare()
{
}

void TerrainRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.terrains) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void TerrainRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.terrains) {
		Shader* shader = x.first->bind(ctx, nullptr);
		if (!shader) continue;
		shader->reflectionMap.set(assets.reflectionMapUnitIndex);
		shader->shadowMap.set(assets.shadowMapUnitIndex);

		for (auto& e : x.second) {
			e->bind(ctx, shader);
			e->draw(ctx);
		}
	}
}
