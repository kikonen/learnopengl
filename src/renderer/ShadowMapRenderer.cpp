#include "ShadowMapRenderer.h"

#include "scene/Scene.h"

ShadowMapRenderer::ShadowMapRenderer(const Assets& assets)
	: Renderer(assets)
{
	shadowShader = Shader::getShader(assets, TEX_SIMPLE_DEPTH);
	shadowDebugShader = Shader::getShader(assets, TEX_DEBUG_DEPTH);
}

ShadowMapRenderer::~ShadowMapRenderer()
{
	delete shadowBuffer;
	delete debugViewport;
}

void ShadowMapRenderer::prepare()
{
	shadowShader->prepare();
	shadowDebugShader->prepare();	

	shadowShader->prepare();
	shadowDebugShader->prepare();

	shadowBuffer = new ShadowBuffer({ assets.shadowMapSize, assets.shadowMapSize, { FrameBufferAttachment::getDepthTexture() } });
	shadowBuffer->prepare();

	debugViewport = new Viewport(
		glm::vec3(-1 + 0.01, 1 - 0.01, 0), 
		glm::vec3(0, 0, 0), 
		glm::vec2(0.5f, 0.5f), 
		shadowBuffer->spec.attachments[0].textureID, 
		shadowDebugShader, 
		[this](Viewport& vp) {
			shadowDebugShader->nearPlane.set(assets.shadowNearPlane);
			shadowDebugShader->farPlane.set(assets.shadowFarPlane);
		});

	debugViewport->prepare();
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
	Light* light = ctx.scene->getDirLight();
	if (!light) return;

	//glm::mat4 b = {
	//	{0.5f, 0.0f, 0.0f, 0.0f},
	//	{0.0f, 0.5f, 0.0f, 0.0f},
	//	{0.0f, 0.0f, 0.5f, 0.0f},
	//	{0.5f, 0.5f, 0.5f, 1.0f},
	//};

	glm::mat4 lightView = glm::lookAt(light->getPos(), light->getTarget(), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, assets.shadowNearPlane, assets.shadowFarPlane);

	//lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

	ctx.lightSpaceMatrix = lightProjection * lightView;
}

void ShadowMapRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;
	shadowBuffer->bindTexture(ctx, 0, assets.shadowMapUnitIndex);
}

void ShadowMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	if (drawIndex++ < drawSkip) return;
	drawIndex = 0;

	{
		shadowBuffer->bind(ctx);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawNodes(ctx, registry);
		shadowBuffer->unbind(ctx);
	}

	rendered = true;
}

void ShadowMapRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		if (t->noShadow) continue;
		Shader* shader = t->bind(ctx, shadowShader);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
		t->unbind(ctx);
	}
}
