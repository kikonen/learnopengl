#include "WaterRenderer.h"

WaterRenderer::WaterRenderer(const Assets& assets)
	: Renderer(assets)
{
}

WaterRenderer::~WaterRenderer()
{
	delete reflectionMap;
	delete refractionMap;
}

void WaterRenderer::prepare()
{
	reflectionMap = new DynamicCubeMap(assets.waterReflectionCubeSize);
	refractionMap = new DynamicCubeMap(assets.waterRefractionCubeSize);
}

void WaterRenderer::bind(const RenderContext& ctx)
{
}

void WaterRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;

	reflectionMap->bindTexture(ctx, assets.waterReflectionMapUnitId);
	refractionMap->bindTexture(ctx, assets.waterReflectionMapUnitId);
}

void WaterRenderer::render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox)
{
}
