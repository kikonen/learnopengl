#include "WaterMapRenderer.h"

#include "SkyboxRenderer.h"

WaterMapRenderer::WaterMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

WaterMapRenderer::~WaterMapRenderer()
{
	delete reflectionBuffer;
	delete refractionBuffer;
}

void WaterMapRenderer::prepare()
{
	FrameBufferSpecification spec = { assets.waterReflectionSize , assets.waterReflectionSize };
	reflectionBuffer = new TextureBuffer(spec);
	refractionBuffer = new TextureBuffer(spec);

	reflectionBuffer->prepare();
	refractionBuffer->prepare();

	debugViewport = new Viewport(
		glm::vec3(0.5, 0.5, 0),
		glm::vec3(0, 0, 0),
		glm::vec2(0.5f, 0.5f),
		reflectionBuffer->textureID,
		Shader::getShader(assets, TEX_VIEWPORT));

	debugViewport->prepare();
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;

	reflectionBuffer->bindTexture(ctx, assets.waterReflectionMapUnitId);
	refractionBuffer->bindTexture(ctx, assets.waterRefractionMapUnitId);
}

void WaterMapRenderer::bind(const RenderContext& ctx)
{
}

void WaterMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox)
{
//	if (drawIndex++ < drawSkip) return;
	drawIndex = 0;

	Water* closest = findClosest(ctx, registry);
	if (!closest) return;

	// https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
	// computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

	const glm::vec3& pos = closest->getPos();

	// reflection map
	{
		Camera camera(ctx.camera->getPos(), ctx.camera->getFront(), ctx.camera->getUp());
		camera.setZoom(ctx.camera->getZoom());
		camera.setRotation(ctx.camera->getRotation());

		RenderContext localCtx(ctx.assets, ctx.clock, ctx.scene, &camera, reflectionBuffer->spec.width, reflectionBuffer->spec.height);
		localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;
		localCtx.bindUBOs();

		reflectionBuffer->bind(localCtx);

		skybox->render(localCtx, registry);
		drawNodes(localCtx, registry);

		reflectionBuffer->unbind(ctx);
	}

	// refraction map
	{
		Camera camera(ctx.camera->getPos(), ctx.camera->getFront(), ctx.camera->getUp());
		camera.setZoom(ctx.camera->getZoom());
		camera.setRotation(ctx.camera->getRotation());

		RenderContext localCtx(ctx.assets, ctx.clock, ctx.scene, &camera, refractionBuffer->spec.width, refractionBuffer->spec.height);
		localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;
		localCtx.bindUBOs();

		refractionBuffer->bind(localCtx);

		skybox->render(localCtx, registry);
		drawNodes(localCtx, registry);

		refractionBuffer->unbind(ctx);
	}

	ctx.bindUBOs();
	rendered = true;
}

void WaterMapRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		if (t->water) continue;
 		Shader* shader = t->bind(ctx, nullptr);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}
}

Water* WaterMapRenderer::findClosest(const RenderContext& ctx, NodeRegistry& registry)
{
	const glm::vec3& cameraPos = ctx.camera->getPos();
	const glm::vec3& cameraDir = ctx.camera->getViewFront();

	std::map<float, Node*> sorted;
	for (auto& x : registry.nodes) {
		if (!x.first->water) continue;

		for (auto& e : x.second) {
			glm::vec3 ray = e->getPos() - cameraPos;
			float distance = glm::length(ray);
			//glm::vec3 fromCamera = glm::normalize(ray);
			//float dot = glm::dot(fromCamera, cameraDir);
			//if (dot < 0) continue;
			sorted[distance] = e;
		}
	}

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		return (Water*)it->second;
	}
	return nullptr;
}
