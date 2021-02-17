#include "ReflectionMapRenderer.h"

#include <vector>

#include "CubeMap.h"


ReflectionMapRenderer::ReflectionMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

ReflectionMapRenderer::~ReflectionMapRenderer()
{
}

void ReflectionMapRenderer::prepare()
{
	std::vector<TextureBuffer*> faces;

	for (auto& tb : textureBuffers) {
		tb.prepare();
		faces.push_back(&tb);
	}

	CubeMap cube;
//	cubeMapTextureID = cube.createFromFrameBuffers(faces);
}

void ReflectionMapRenderer::bind(const RenderContext& ctx)
{
}

void ReflectionMapRenderer::bindTexture(const RenderContext& ctx)
{
	if (cubeMapTextureID == -1) return;
	glActiveTexture(assets.refactionMapUnitId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
}

void ReflectionMapRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	if (++drawIndex < drawSkip) return;
	drawIndex = 0;

	RenderContext reflectionCtx(ctx.engine, ctx.dt, ctx.scene, ctx.camera);

	for (auto& tb : textureBuffers) {
		tb.bind();

		glClear(GL_DEPTH_BUFFER_BIT);
		drawNodes(reflectionCtx, registry);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, reflectionCtx.width, reflectionCtx.height);
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
