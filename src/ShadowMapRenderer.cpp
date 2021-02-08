#include "ShadowMapRenderer.h"

#include "Scene.h"

ShadowMapRenderer::ShadowMapRenderer(const Assets& assets)
	: assets(assets)
{
	shadowShader = Shader::getShader(assets, TEX_SIMPLE_DEPTH);
	shadowDebugShader = Shader::getShader(assets, TEX_DEBUG_DEPTH);
}


void ShadowMapRenderer::prepare()
{
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

	shadowShader->prepare();
	shadowDebugShader->prepare();

	debugViewport = new Viewport(glm::vec3(-1 + 0.01, 1 - 0.01, 0), glm::vec3(0, 0, 0), glm::vec2(0.5f, 0.5f), frameBuffer, shadowDebugShader);
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

void ShadowMapRenderer::bindTexture(RenderContext& ctx)
{
	frameBuffer.bindTexture(assets.shadowMapUnitId);
}

void ShadowMapRenderer::render(
	RenderContext& ctx, 
	std::map<NodeType*, std::vector<Node*>>& typeNodes,
	std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
	std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	frameBuffer.bind();

	glClear(GL_DEPTH_BUFFER_BIT);

	drawNodes(ctx, typeNodes, typeSprites, typeTerrains);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, ctx.engine.width, ctx.engine.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ShadowMapRenderer::drawNodes(
	RenderContext& ctx, 
	std::map<NodeType*, std::vector<Node*>>& typeNodes,
	std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
	std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeNodes) {
		x.first->bind(ctx, shadowShader);

		for (auto& e : x.second) {
			if (e->light || e->skipShadow) {
				continue;
			}
			e->bind(ctx, shadowShader);
			e->draw(ctx);
		}
	}

	//for (auto& x : typeTerrains) {
	//	for (auto& e : x.second) {
	//		e->bind(ctx, shadowShader);
	//		e->draw(ctx);
	//	}
	//}

	for (auto& x : typeSprites) {
		x.first->bind(ctx, shadowShader);

		for (auto& e : x.second) {
			e->bind(ctx, shadowShader);
			e->draw(ctx);
		}
	}
}
