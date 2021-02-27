#include "WaterMapRenderer.h"

WaterMapRenderer::WaterMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

WaterMapRenderer::~WaterMapRenderer()
{
	delete reflectionMap;
	delete refractionMap;
}

void WaterMapRenderer::prepare()
{
	FrameBufferSpecification spec = { assets.waterReflectionSize , assets.waterReflectionSize };
	reflectionMap = new TextureBuffer(spec);
	refractionMap = new TextureBuffer(spec);
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;

	reflectionMap->bindTexture(ctx, assets.waterReflectionMapUnitId);
	refractionMap->bindTexture(ctx, assets.waterRefractionMapUnitId);
}

void WaterMapRenderer::bind(const RenderContext& ctx)
{
}

void WaterMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
}
