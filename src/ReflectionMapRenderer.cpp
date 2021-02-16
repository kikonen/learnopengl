#include "ReflectionMapRenderer.h"

ReflectionMapRenderer::ReflectionMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

ReflectionMapRenderer::~ReflectionMapRenderer()
{
}

void ReflectionMapRenderer::prepare()
{
	for (auto& frameBuffer : frameBuffers) {
		frameBuffer.prepare();

		frameBuffer.bindTexture(assets.shadowMapUnitId);
		glm::vec4 borderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));

		frameBuffer.bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, frameBuffer.textureID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		frameBuffer.unbind();
	}
}

void ReflectionMapRenderer::bind(const RenderContext& ctx)
{
}

void ReflectionMapRenderer::bindTexture(const RenderContext& ctx)
{
//	frameBuffer.bindTexture(assets.shadowMapUnitId);
}

void ReflectionMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	if (++drawIndex < drawSkip) return;
	drawIndex = 0;

	RenderContext reflectionCtx(ctx.engine, ctx.dt, ctx.scene, ctx.camera);

	for (auto& frameBuffer : frameBuffers) {
		frameBuffer.bind();

		glClear(GL_DEPTH_BUFFER_BIT);

//		drawNodes(reflectionCtx, registry);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, reflectionCtx.width, reflectionCtx.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}

void ReflectionMapRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.terrains) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}

	for (auto& x : registry.sprites) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);

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
		Shader* shader = t->bind(ctx, nullptr);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}
}
