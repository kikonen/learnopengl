#include "ShadowMapRenderer.h"

#include "Scene.h"

ShadowMapRenderer::ShadowMapRenderer(const Assets& assets)
	: Renderer(assets)
{
	shadowShader = Shader::getShader(assets, TEX_SIMPLE_DEPTH);
	shadowDebugShader = Shader::getShader(assets, TEX_DEBUG_DEPTH);
}

void ShadowMapRenderer::prepare()
{
	shadowShader->prepare();
	shadowDebugShader->prepare();
	
	shadowBuffer.prepare();

	shadowShader->prepare();
	shadowDebugShader->prepare();

	debugViewport = new Viewport(
		glm::vec3(-1 + 0.01, 1 - 0.01, 0), 
		glm::vec3(0, 0, 0), 
		glm::vec2(0.5f, 0.5f), 
		shadowBuffer, 
		shadowDebugShader, 
		[this](Viewport& vp) {
			shadowDebugShader->nearPlane.set(0.1f);
			shadowDebugShader->farPlane.set(1000.f);
		});

	debugViewport->prepare();
}

void ShadowMapRenderer::bind(RenderContext& ctx)
{
	Light* light = ctx.scene->getDirLight();
	if (!light) return;

	glm::mat4 b = {
		{0.5f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.5f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.5f, 0.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
	};

	glm::mat4 lightView = glm::lookAt(light->pos, light->target, glm::vec3(0.0, 1.0, 0.0));

	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);

	//lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	ctx.lightSpaceMatrix = lightSpaceMatrix;
}

void ShadowMapRenderer::bindTexture(const RenderContext& ctx)
{
	shadowBuffer.bindTexture(assets.shadowMapUnitId);
}

void ShadowMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	if (++drawIndex < drawSkip) return;
	drawIndex = 0;

	shadowBuffer.bind();

	glClear(GL_DEPTH_BUFFER_BIT);

	drawNodes(ctx, registry);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, ctx.width, ctx.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ShadowMapRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
#if 0
	for (auto& x : registry.terrains) {
		NodeType* t = x.first;
		if (t->light || t->skipShadow) continue;
		Shader* shader = t->bind(ctx, shadowShader);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}
#endif

	for (auto& x : registry.sprites) {
		NodeType* t = x.first;
		if (t->light || t->skipShadow) continue;
		Shader* shader = t->bind(ctx, shadowShader);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}

	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		if (t->light || t->skipShadow) continue;
		Shader* shader = t->bind(ctx, shadowShader);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}
}
