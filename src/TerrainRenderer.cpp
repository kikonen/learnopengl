#include "TerrainRenderer.h"

TerrainRenderer::TerrainRenderer(const Assets& assets)
	: assets(assets)
{
}

TerrainRenderer::~TerrainRenderer()
{
}

void TerrainRenderer::prepare()
{
}

void TerrainRenderer::update(RenderContext& ctx, std::map<int, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeTerrains) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void TerrainRenderer::render(RenderContext& ctx, std::map<int, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeTerrains) {
		for (auto& e : x.second) {
			Shader* shader = e->bind(ctx, nullptr);
			shader->shadowMap.set(assets.shadowMapUnitIndex);
			e->draw(ctx);
		}
	}
}
