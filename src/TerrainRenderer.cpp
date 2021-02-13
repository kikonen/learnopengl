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

void TerrainRenderer::update(RenderContext& ctx, std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeTerrains) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void TerrainRenderer::render(RenderContext& ctx, std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeTerrains) {
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
