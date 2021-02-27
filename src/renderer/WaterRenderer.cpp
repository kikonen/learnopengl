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
	FrameBufferSpecification spec = { assets.waterReflectionSize , assets.waterReflectionSize };
	reflectionMap = new TextureBuffer(spec);
	refractionMap = new TextureBuffer(spec);
}

void WaterRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.waters) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void WaterRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;

	reflectionMap->bindTexture(ctx, assets.waterReflectionMapUnitId);
	refractionMap->bindTexture(ctx, assets.waterRefractionMapUnitId);
}

void WaterRenderer::bind(const RenderContext& ctx)
{
}

void WaterRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.waters) {
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
