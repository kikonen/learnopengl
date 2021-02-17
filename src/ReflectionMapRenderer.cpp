#include "ReflectionMapRenderer.h"

#include <vector>

#include "CubeMap.h"


const int CUBE_SIZE = 400;


ReflectionMapRenderer::ReflectionMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

ReflectionMapRenderer::~ReflectionMapRenderer()
{
}

void ReflectionMapRenderer::prepare()
{
}

void ReflectionMapRenderer::bind(const RenderContext& ctx)
{
}

void ReflectionMapRenderer::bindTexture(const RenderContext& ctx)
{
	glActiveTexture(assets.refactionMapUnitId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void ReflectionMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	textureID = CubeMap::createEmpty(CUBE_SIZE);

	if (++drawIndex < drawSkip) return;
	drawIndex = 0;

	unsigned int FBO;
	unsigned int RBO;

	{
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}

	{
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBE_SIZE, CUBE_SIZE);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
	}

	glViewport(0, 0, CUBE_SIZE, CUBE_SIZE);

//	RenderContext reflectionCtx(ctx.engine, ctx.dt, ctx.scene, ctx.camera);

	for (int i = 0; i < 6; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureID, 0);
		drawNodes(ctx, registry);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glDeleteRenderbuffers(1, &RBO);
	glDeleteFramebuffers(1, &FBO);

	glViewport(0, 0, ctx.width, ctx.height);
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
