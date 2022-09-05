#include "WaterMapRenderer.h"

#include "SkyboxRenderer.h"
#include "WaterNoiseGenerator.h"

WaterMapRenderer::WaterMapRenderer(const Assets& assets)
	: Renderer(assets)
{
	drawIndex = 1;
	drawSkip = 0;
}

WaterMapRenderer::~WaterMapRenderer()
{
	delete reflectionBuffer;
	delete refractionBuffer;
}

void WaterMapRenderer::prepare()
{
	FrameBufferSpecification spec = { 
		assets.waterReflectionSize , 
		assets.waterReflectionSize, 
		{ FrameBufferAttachment::getTexture(), FrameBufferAttachment::getRBODepth() } 
	};

	reflectionBuffer = new TextureBuffer(spec);
	refractionBuffer = new TextureBuffer(spec);

	reflectionBuffer->prepare();
	refractionBuffer->prepare();

	//WaterNoiseGenerator generator;
	//noiseTextureID = generator.generate();

	reflectionDebugViewport = std::make_shared<Viewport>(
		glm::vec3(0.5, 0.5, 0),
		glm::vec3(0, 0, 0),
		glm::vec2(0.5f, 0.5f),
		reflectionBuffer->spec.attachments[0].textureID,
		Shader::getShader(assets, TEX_VIEWPORT));

	refractionDebugViewport = std::make_shared<Viewport>(
		glm::vec3(0.5, 0.0, 0),
		glm::vec3(0, 0, 0),
		glm::vec2(0.5f, 0.5f),
		reflectionBuffer->spec.attachments[0].textureID,
		Shader::getShader(assets, TEX_VIEWPORT));

	reflectionDebugViewport->prepare();
	refractionDebugViewport->prepare();
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;

	reflectionBuffer->bindTexture(ctx, 0, assets.waterReflectionMapUnitIndex);
	refractionBuffer->bindTexture(ctx, 0, assets.waterRefractionMapUnitIndex);
	if (noiseTextureID != -1) {
		glBindTextures(assets.noiseUnitIndex, 1, &noiseTextureID);
	}
}

void WaterMapRenderer::bind(const RenderContext& ctx)
{
}

void WaterMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox)
{
	if (!stepRender()) return;

	Water* closest = findClosest(ctx, registry);
	if (!closest) return;

	// https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
	// computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

	glm::vec3 planePos = closest->getPos();

	// https://prideout.net/clip-planes
	// reflection map
	{
		glm::vec3 pos = ctx.camera.getPos();
		const float dist = pos.y - planePos.y;
		pos.y -= dist * 2;

		glm::vec3 rot = ctx.camera.getRotation();
		rot.x = -rot.x;

		Camera camera(pos, ctx.camera.getFront(), ctx.camera.getUp());
		camera.setZoom(ctx.camera.getZoom());
		camera.setRotation(rot);

		RenderContext localCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, camera, reflectionBuffer->spec.width, reflectionBuffer->spec.height);
		localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

		ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
		clip.enabled = true;
		clip.plane = glm::vec4(0, 1, 0, -planePos.y);

		localCtx.bindMatricesUBO();

		reflectionBuffer->bind(localCtx);

		drawNodes(localCtx, registry, skybox, closest);

		reflectionBuffer->unbind(ctx);
		ctx.bindClipPlanesUBO();
	}

	// refraction map
	{
		glm::vec3 rot = ctx.camera.getRotation();
		glm::vec3 pos = ctx.camera.getPos();

		Camera camera(pos, ctx.camera.getFront(), ctx.camera.getUp());
		camera.setZoom(ctx.camera.getZoom());
		camera.setRotation(rot);

		RenderContext localCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, camera, refractionBuffer->spec.width, refractionBuffer->spec.height);
		localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

		ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
		clip.enabled = true;
		clip.plane = glm::vec4(0, -1, 0, planePos.y);

		localCtx.bindMatricesUBO();

		refractionBuffer->bind(localCtx);

		drawNodes(localCtx, registry, skybox, closest);

		refractionBuffer->unbind(ctx);
		ctx.bindClipPlanesUBO();
	}

	ctx.bindMatricesUBO();

	rendered = true;
}

void WaterMapRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox, Node* current)
{
	glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox->render(ctx, registry);

	ctx.bindClipPlanesUBO();
	ctx.state.enable(GL_CLIP_DISTANCE0);

	for (auto& x : registry.nodes) {
		auto t = x.first;
		//if (t->water || t->light) continue;
		auto shader = t->bind(ctx, nullptr);

		auto& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			if (e == current) continue;
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
		t->unbind(ctx);
	}

	ctx.state.disable(GL_CLIP_DISTANCE0);
}

Water* WaterMapRenderer::findClosest(const RenderContext& ctx, NodeRegistry& registry)
{
	const glm::vec3& cameraPos = ctx.camera.getPos();
	const glm::vec3& cameraDir = ctx.camera.getViewFront();

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
