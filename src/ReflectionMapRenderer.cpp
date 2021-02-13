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

void ReflectionMapRenderer::bind(RenderContext& ctx)
{
}

void ReflectionMapRenderer::bindTexture(RenderContext& ctx)
{
//	frameBuffer.bindTexture(assets.shadowMapUnitId);
}

void ReflectionMapRenderer::render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes, std::map<NodeType*, std::vector<Sprite*>>& typeSprites, std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& frameBuffer : frameBuffers) {
		frameBuffer.bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		drawNodes(ctx, typeNodes, typeSprites, typeTerrains);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, ctx.engine.width, ctx.engine.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}

void ReflectionMapRenderer::drawNodes(
	RenderContext& ctx,
	std::map<NodeType*, std::vector<Node*>>& typeNodes,
	std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
	std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeTerrains) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}

	for (auto& x : typeSprites) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}

	for (auto& x : typeNodes) {
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
