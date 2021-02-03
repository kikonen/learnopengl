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

void TerrainRenderer::render(RenderContext& ctx, std::vector<Terrain*>& terrains)
{
	for (auto terrain : terrains) {
		Shader* shader = terrain->bind(ctx, nullptr);
		shader->shadowMap.set(assets.shadowMapUnitIndex);
		terrain->draw(ctx);
	}
}
